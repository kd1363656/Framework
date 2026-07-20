#include "SkeletalAnimationModel.hlsli"

cbuffer CBSkeletalAnimationVertexSkinning : register(b0)
{
    uint g_sourceVertexBufferSRVDescriptorIndex;
    uint g_bonePaletteBufferSRVDescriptorIndex;
    uint g_boneMatrixBufferSRVDescriptorIndex;
    uint g_skinnedVertexBufferUAVDescriptorIndex;
    
    uint g_vertexCount;
}

// 不正な負のBone Weightを補正するときの下限値
// 通常、Bone Weightはゼロ以上でなければならない
// 読み込んだModelデータに負数が含まれていた場合は、
// この値まで補正する
static const float k_minimumBoneWeight = 0.0F;

// Bone Weightを有効と判断するための許容誤差
// 浮動小数点数は計算誤差を含むため、
// 完全なゼロとの比較ではなく、この値以下かどうかで判定する
static const float k_boneWeightEpsilon = 0.000001F;

// 頂点座標を行列変換するときの同次座標W成分
// 位置には平行移動を適用する必要があるため、W成分を1にする
static const float k_positionHomogeneousElement = 1.0F;

// 法線行列変換するときの同次座標W成分
// 法線は方向を表すため、平行移動の影響を受けないよう
// W成分をゼロにする
static const float k_directionHomogeneousElement = 0.0F;

// スキニング結果を加算するときの頂点座標の初期値
static const float4 k_initialAccumulatedPosition = float4(0.0F, 0.0F, 0.0F, 0.0F);

// スキニング結果を加算するときの方向ベクトルの初期値
static const float3 k_initialAccumulatedDirection = float3(0.0F, 0.0F, 0.0F);

// 実際に使用できたBone Weight合計の初期値
static const float k_initialValidBoneWeightTotal = 0.0F;

// Bone Influenceの走査を開始するIndex
static const uint k_firstBoneInfluenceIndex = 0U;

// 一つの頂点が保持できる最大Bone Influence数
// C++側のSkeletalAnimationModelVertexも、
// ZeroからThreeまでの4個のBone Palette Indexと
// 4個のBone Weightを保持している
static const uint k_maxBoneInfluenceCount = 4U;

// 一つのThread GroupがX方向に処理する頂点数
// 一つのThreadが一つの頂点を担当するため、
// 一つのThread Groupで最大64頂点を処理する
static const uint k_vertexSkinningThreadCountX = 64U;

static const uint k_singleThreadCountY = 1U;
static const uint k_singleThreadCountZ = 1U;

static const uint k_initialBonePaletteElementCount      = 0U;
static const uint k_initialBonePaletteElementByteStride = 0U;
static const uint k_initialBoneMatrixElementCount       = 0U;
static const uint k_initialBoneMatrixElementByteStride  = 0U;

// スキニング後の法線または接線を安全に正規化する
// 通常はnormalizeでもよいが、長さがほぼゼロのベクトルを
// normalizeすると、NaNなどの不正な値が発生する可能性がある
// 長さがほぼゼロだった場合は、Modelが元から保持していた
// 法線または接線を代わりに返す
float3 NormalizeSkinningDirection(const float3 a_direction, const float3 a_fallbackDirection)
{
    // sqrtを実行さずに長さを判定できるように、長さの二乗を求める
    const float l_lengthSquared = dot(a_direction, a_direction);

    // スキニング後の方向がほぼゼロベクトルだった場合は、
    // 不正な正規化を避けるため、元の方向を返す
    if (l_lengthSquared <= k_boneWeightEpsilon) { return a_fallbackDirection; }
    
    // rsqrtは平方根の逆数を求める
    // direction / lengthと同じ結果になるが、
    // GPU上ではこちらの形が効率的に処理される
    return a_direction * rsqrt(l_lengthSquared);
}

// 一つのThreadが一つの頂点をスキニングする
// X方向には64個のThreadを生成し、
// Y方向とZ方向には一つだけThreadを生成する
[numthread(k_vertexSkinningThreadCountX, k_singleThreadCountY, k_singleThreadCountZ)]
void main(const uint3 a_dispatchThreadID : SV_DispatchThreadID)
{
    // X方向のThreadIDを、そのまま頂点Indexとして使用する
    const uint l_vertexIndex = a_dispatchThreadID.x;
    
    // C++側では頂点数をThreadGroupの大きさで切り上げてDispatchする
    // 例えば頂点数が65の場合でも、128個のThreadが実行される
    // そのため、実在しない頂点を処理するThreadはここで終了させる
    if (l_vertexIndex >= g_vertexCount) { return; }
    
    StructuredBuffer  <SkeletalAnimationModelVertex>             l_sourceVertexBuffer  = ResourceDescriptorHeap[g_sourceVertexBufferSRVDescriptorIndex];
    StructuredBuffer  <SkeletalAnimationModelBonePaletteElement> l_bonePaletteBuffer   = ResourceDescriptorHeap[g_bonePaletteBufferSRVDescriptorIndex];
    StructuredBuffer  <SkeletalAnimationBoneMatrixElement>       l_boneMatrixBuffer    = ResourceDescriptorHeap[g_boneMatrixBufferSRVDescriptorIndex]; 
    RWStructuredBuffer<SkeletalAnimationSkinnedVertex>           l_skinnedVertexBuffer = ResourceDescriptorHeap[g_skinnedVertexBufferUAVDescriptorIndex];
    
    uint l_bonePaletteElementCount      = k_initialBonePaletteElementCount;
    uint l_bonePaletteElementByteStride = k_initialBonePaletteElementByteStride;
    uint l_boneMatrixElementCount       = k_initialBoneMatrixElementCount;
    uint l_boneMatrixElementByteStride  = k_initialBoneMatrixElementByteStride;
 
    l_bonePaletteBuffer.GetDimensions(l_bonePaletteElementCount, l_bonePaletteElementByteStride);
    l_boneMatrixBuffer.GetDimensions (l_boneMatrixElementCount,  l_boneMatrixElementByteStride);

    const SkeletalAnimationModelVertex l_sourceVertex = l_sourceVertexBuffer[l_vertexIndex];
    
    // 一つの頂点が保持している4個のBone Palette Indexを
    // 一つのuint4へまとめる
    // これにより、Bone Influenceをfor文で共通処理できる
    const uint4 l_bonePaletteIndexList = uint4(l_sourceVertex.bonePaletteIndexZero,
                                               l_sourceVertex.bonePaletteIndexOne,
                                               l_sourceVertex.bonePaletteIndexTwo,
                                               l_sourceVertex.bonePaletteIndexThree);
    
    // 読み込んだModelに不正な負のBoneWeightが含まれていた場合は、
    // BoneWeightとして許可する最小値へ補正する
    const float4 l_boneWeightList = max(l_sourceVertex.boneWeight, k_minimumBoneWeight);
    
    float3 l_accumulatedPosition = k_initialAccumulatedPosition;
    float3 l_accumulatedNormal   = k_initialAccumulatedDirection;
    float3 l_accumulatedTangent = k_initialAccumulatedDirection;
    
    // 実際に使用できたBone Weightの合計
    // 無効なPalette IndexやBone Indexを持つInfluenceは除外するため、
    // 元データのWeight合計と異なる場合がある
    float l_validBoneWeightTotal = k_initialValidBoneWeightTotal;
    
    // 一つの頂点が保持しているすべてのBone Influenceを処理する
    // Bone Influence数は固定なので、unrollを指定して
    // ループを展開しやすくする
    [unroll]
    for (uint l_boneInfluenceIndex = k_firstBoneInfluenceIndex; l_boneInfluenceIndex < k_maxBoneInfluenceCount; ++l_boneInfluenceIndex)
    {
        // 現在処理しているBoneInfluenceのWeightを取得する
        const float l_boneWeight = l_boneWeightList[l_boneInfluenceIndex];
        
        // WeightがほぼゼロのInfluenceは頂点へ影響しないため、
        // BonePaletteやBoneMatrixを参照せずに次へ進む
        if (l_boneWeight <= k_boneWeightEpsilon) { continue; }

        // 頂点が参照しているBonePaletteIndexを取得する
        const uint l_bonePaletteIndex = l_bonePaletteIndexList[l_boneInfluenceIndex];
        
        // 壊されたModelデータによって範囲外のPaletteIndexが
        // 指定された場合は、そのInfluenceを使用しない
        if (l_bonePaletteIndex >= l_bonePaletteElementCount) { continue; }

        // PaletteIndexに対応するBonePalette要素を取得する
        const SkeletalAnimationModelBonePaletteElement l_bonePaletteElement = l_bonePaletteBuffer[l_bonePaletteIndex];
        
        // BonePaletteが参照しているBoneIndexが、
        // BoneMatrixBufferの範囲内にあるか確認する
        if (l_bonePaletteElement.boneIndex >= l_boneMatrixElementCount) { continue; }

        // 現在Poseにおける対象BoneのGloablMatrixを取得する
        const SkeletalAnimationBoneMatrixElement l_boneMatrixElement = l_boneMatrixBuffer[l_bonePaletteElement.boneIndex];
        
        // LinearBlendSkinningで使用する行列を作成する
        // inverseBindPoseMatrix : Model頂点をBindPose時のBoneLocal空間へ移動する行列
        // globalBoneMatrix      : 現在AnimationPoseに置けるBoneのGloabl変換行列
        // Simplemathと現在のHLSLは行ベクトル方式であるため
        // Vertex * inverseBindPoseMatrix * globalBoneMatrixの順番になる
        const float4x4 l_skinningMatrix = mul(l_bonePaletteElement.inverseBindPoseMatrix, l_boneMatrixElement.globalBoneMatrix);
        
        // 頂点座標を現在のBoneで変換する
        const float4 l_skinnedPosition = mul(float4(l_sourceVertex.position, k_positionHomogeneousElement), l_skinningMatrix);

        // 法線を現在のBoneで変換する
        // 法線は方向なので平行移動を適用しない
        const float4 l_skinnedNormal = mul(float4(l_sourceVertex.normal, k_directionHomogeneousElement), l_skinningMatrix);
        
        // 接線を現在のBoneで変換する
        // tangent.wには接線空間の向きを示す符号が格納されているため、
        // xyzだけを行列変換し、wは後で元の値を引き継ぐ
        const float4 l_skinnedTangent = mul(float4(l_sourceVertex.tangent.xyz, k_directionHomogeneousElement), l_skinningMatrix);
        
        // Boneで変換した頂点座標にWeightを掛けて加算する
        l_accumulatedPosition += l_skinnedPosition * l_boneWeight;
        
        // Boneで変換した法線にWegithを掛けて加算する
        l_accumulatedNormal += l_skinnedNormal.xyz * l_boneWeight;
        
        // Boneで変換した接線にWeightを掛けて加算する
        l_accumulatedTangent += l_skinnedNormal.xyz * l_boneWeight;
        
        // 正常に使用できたBoneWeightを合計する
        l_validBoneWeightTotal += l_boneWeight;
    }
    
    // ComputeShaderから出力するスキニング済み頂点
    SkeletalAnimationSkinnedVertex l_skinnedVertex = (SkeletalAnimationSkinnedVertex) 0;
    
    // 有効なBoneInfluenceを一つも使用できなかった場合は、
    // 頂点が原点へ移動したり、法線がゼロになったりしないよう、
    // 元頂点の情報をそのまま出力する
    if (l_validBoneWeightTotal <= k_boneWeightEpsilon)
    {
        l_skinnedVertex.position = l_sourceVertex.position;
        l_skinnedVertex.normal   = l_sourceVertex.normal;
        l_skinnedVertex.tangent  = l_sourceVertex.tangent;
        l_skinnedVertex.uv       = l_sourceVertex.uv;
        
        l_skinnedVertexBuffer[l_vertexIndex] = l_skinnedVertex;
        
        return;
    }

    // 正常なModelではBoneWeightの合計は1になる
    // ただし、範囲外のPaletteIndexやBoneIndexを除外した場合は
    // 使用で来たWeightの合計が1未満になることがある
    // そのまま出力すると頂点が縮むため、
    // 使用できたWeight合計の逆数を求めて正規化する
    const float l_inverseValidBoneWeightTotal = rcp(l_validBoneWeightTotal);
    
    // 加算した頂点座標を、使用で来たWeight合計で正規化する
    l_skinnedVertex.position = l_accumulatedPosition.xyz * l_inverseValidBoneWeightTotal;

    // 加算した法線をWeight合計で補正した後、
    // 単位ベクトルになるよう安全に正規化する
    l_skinnedVertex.normal = NormalizeSkinningDirection(l_accumulatedNormal * l_inverseValidBoneWeightTotal, l_sourceVertex.normal);
    
    // 接線のxyzを安全に正規化する
    // tangent.wは接線空間の左右方向を表す値なので、
    // スキニングせず元頂点の値を維持する
    l_skinnedVertex.tangent = float4(NormalizeSkinningDirection(l_accumulatedTangent * l_inverseValidBoneWeightTotal, l_sourceVertex.tangent.xyz), l_sourceVertex.tangent.w);
    
    // UV座標はBoneTransformの影響を受けないため、
    // 元頂点からそのままコピーする
    l_skinnedVertex.uv = l_sourceVertex.uv;
    
    // 完成したスキニング済み頂点をUAVBufferへ書き込む
    // 後続の描画Passでは、このBufferをSRVとして読み取る
    l_skinnedVertexBuffer[l_vertexIndex] = l_skinnedVertex;
}