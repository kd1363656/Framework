#pragma once

namespace FWK::Struct
{
    // ModelMesh1つが使うMaterial
    struct ModelMaterial final
    {
         ModelMaterial() = default;
        ~ModelMaterial() = default;

        ModelMaterial(const ModelMaterial&)           = delete;
        ModelMaterial(      ModelMaterial&&) noexcept = default;

        ModelMaterial& operator=(const ModelMaterial&)           = delete;
        ModelMaterial& operator=(      ModelMaterial&&) noexcept = default;

        // 実行時に読み込むMaterial(テクスチャやシェーダーへ送るデータはMaterial側が持つ)
        Graphics::Material m_material = {};

        // モデルファイルの場所を基準にした.materialファイルの相対パス(空の場合はデフォルトマテリアルを使う)
        // ※ モデルのバイナリーファイルにはこのパスだけを保存する
        std::wstring m_materialFilePath = {};
    };

    // Meshlet1個分の参照範囲情報
    struct ModelMeshlet final
    {
        static constexpr std::uint32_t k_initialMeshletVertexOffset   = 0U;
        static constexpr std::uint32_t k_initialMeshletTriangleOffset = 0U;
        static constexpr std::uint32_t k_initialMeshletVertexCount    = 0U;
        static constexpr std::uint32_t k_initialMeshletTriangleCount  = 0U;

        // m_uniqueVertexIndexListの開始位置
        // MeshShaderでは、このOffsetからm_vertexCount個分の元頂点Indexを読む
        std::uint32_t m_vertexOffset = k_initialMeshletVertexOffset;

        // このMeshletで使用するユニーク頂点数
        // MeshShaderのSetMeshOutputCounts()で出力頂点数として使用する
        std::uint32_t m_vertexCount = k_initialMeshletVertexCount;

        // m_primitiveIndexListの開始位置
        // MeshShader側では、このOffsetからm_triangleCount個分の三角形情報を読む
        std::uint32_t m_triangleOffset = k_initialMeshletTriangleOffset;

        // このMeshletに含まれる三角形数
        std::uint32_t m_triangleCount = k_initialMeshletTriangleCount;
    };

    // Meshlet 1個分のカリング用境界情報
    // Meshlet単位のFrustumCullingやBackface Cone Cullingで使用する
    struct ModelMeshletBounds final
    {
        static constexpr float k_initialMeshletBoundsRadius     = 0.0F;
        static constexpr float k_initialMeshletBoundsConeCutoff = 0.0F;
        static constexpr float k_initialMeshletBoundsPadding    = 0.0F;

        // Meshletを囲むBounding Sphereの中心座標
        // Frustum Cullingで視錐台の中にあるか判定するために使用する
        TypeAlias::Math::Vector3 m_center = {};

        // Meshletを囲むBounding Sphereの半径
        // m_centerと合わせて、Meshletがカメラに映る可能性があるか判定する
        float m_radius = k_initialMeshletBoundsRadius;

        // Backface Cone Culling用の円錐の頂点座標
        // Meshlet内の三角形群がカメラから見て裏向きか判定するために使用する
        TypeAlias::Math::Vector3 m_coneApex = {};

        // Backface Cone Culling用の判定しきい値
        // m_coneAxisと視線方向の内積判定などで使用する
        float m_coneCutoff = k_initialMeshletBoundsConeCutoff;

        // Backface Cone Culling用の円錐の向き
        // Meshlet内の三角形群が大体どちらを向いているかを表す
        TypeAlias::Math::Vector3 m_coneAxis = {};

        // GPU側の構造体サイズを16bytes単位に揃えるためのPadding
        // Vector3 + float の形にして、HLSL側のStructuredBufferで扱いやすくする
        float m_padding = k_initialMeshletBoundsPadding;
    };

    // LOD(Level Of Detail : 詳細度)1段階分の情報
    // 遠くのモデルは画面上で小さく映るため、三角形を減らした形でも見た目がほとんど変わらない。
    // そこで、三角形数を段階的に減らした形をあらかじめ作っておき、距離に応じて使い分ける。
    //
    // 全LODのMeshletは、ModelMeshletDataの各リストへ「LOD0、LOD1、LOD2…」の順に連結して格納する。
    // LOD1段階は「何番目のMeshletから何個使うか」という範囲だけを持つため、
    // GPU Bufferや頂点Bufferを増やさずにLODを切り替えられる
    struct ModelMeshLOD final
    {
        static constexpr std::uint32_t k_initialMeshletOffset = 0U;
        static constexpr std::uint32_t k_initialMeshletCount  = 0U;
        static constexpr std::uint32_t k_initialTriangleCount = 0U;

        static constexpr float k_initialSimplifyError = 0.0F;

        // このLODが使うMeshletの先頭番号と個数
        std::uint32_t m_meshletOffset = k_initialMeshletOffset;
        std::uint32_t m_meshletCount  = k_initialMeshletCount;

        // このLODの三角形数(計測、確認用)
        std::uint32_t m_triangleCount = k_initialTriangleCount;

        // 元の形からどれだけ形が崩れたか(モデルのローカル座標での距離)
        // 描画時にこの値を画面上のピクセル数へ換算し、1ピクセル程度以下ならこのLODを使う
        float m_simplifyError = k_initialSimplifyError;
    };

    // 1つのModelMeshが持つMeshlet関連データ一式
    // ※ MeshShader描画では、このデータをGPU Buffer化してShader側から参照する
    struct ModelMeshletData final
    {
        static constexpr float k_initialBoundsRadius = 0.0F;

        // Meshlet一覧
        // 1要素がMeshlet1個分の参照範囲情報を表す
        std::vector<ModelMeshlet> m_meshletList = {};

        // 各Meshletが使用する元頂点Index一覧
        // Meshlet内のLocalVertexIndexから、ModelVertexList上の頂点Indexへ変換するために使用
        std::vector<std::uint32_t> m_uniqueVertexIndexList = {};

        // Meshlet内の三角形情報
        // meshoptimizerが出力するPrimitiveIndexはuint8_t
        // そのままuint32_tに1個ずつ拡張するとGPUBufferが大きくなるため、
        // 3個のuint8_tのPrimitiveIndexListを1個のuint32_tへPackして保存する
        std::vector<std::uint32_t> m_primitiveIndexList = {};

        // Meshletごとのカリング用境界情報
        // m_meshletList[i]に対するBoundsはm_meshletBoundsList[i]に入る
        std::vector<ModelMeshletBounds> m_meshletBoundsList = {};

        // LOD一覧(m_lodList[0]が元の形、番号が大きいほど三角形が少ない)
        std::vector<ModelMeshLOD> m_lodList = {};

        // Mesh全体を囲むBounding Sphere(ローカル座標)
        // LOD選択でカメラからの距離を求めるために使用する
        TypeAlias::Math::Vector3 m_boundsCenter = {};

        float m_boundsRadius = k_initialBoundsRadius;
    };

    struct ModelMeshRuntimeDataBase
    {
         ModelMeshRuntimeDataBase() = default;
        ~ModelMeshRuntimeDataBase() = default;

        ModelMeshRuntimeDataBase(const ModelMeshRuntimeDataBase&)           = delete;
        ModelMeshRuntimeDataBase(      ModelMeshRuntimeDataBase&&) noexcept = default;

        ModelMeshRuntimeDataBase& operator=(const ModelMeshRuntimeDataBase&)           = delete;
        ModelMeshRuntimeDataBase& operator=(      ModelMeshRuntimeDataBase&&) noexcept = default;

        // MeshShaderで頂点情報を参照するためのStructuredBuffer
        Graphics::StaticStructuredBuffer m_vertexBuffer = {};

        // MeshShaderで参照するMeshlet情報用StructuredBuffer
        Graphics::StaticStructuredBuffer m_meshletBuffer = {};

        // Meshlet内LocalVertexIndexからModelVertexIndexへ変換するStructuredBuffer
        Graphics::StaticStructuredBuffer m_uniqueVertexIndexBuffer = {};

        // Meshlet内の三角形を組み立てるためのStructuredBuffer
        Graphics::StaticStructuredBuffer m_primitiveIndexBuffer = {};

        // Meshlet単位のカリング情報を参照するStructuredBuffer
        Graphics::StaticStructuredBuffer m_meshletBoundsBuffer = {};
    };
}