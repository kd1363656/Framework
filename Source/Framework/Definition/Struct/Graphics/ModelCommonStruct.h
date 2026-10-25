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

        Graphics::Material m_material = {};

        // モデルファイルの場所を基準にした.materialファイルの相対パス(空の場合はデフォルトマテリアルを使う)
        // ※ モデルのバイナリーファイルにはこのパスだけを保存する
        std::wstring m_materialFilePath = {};
    };

    // Meshlet1個分の参照範囲情報
    struct ModelMeshlet final
    {
        std::uint32_t m_vertexOffset = Constant::k_initialMeshletVertexOffset;

        std::uint32_t m_vertexCount = Constant::k_initialMeshletVertexCount;

        std::uint32_t m_triangleOffset = Constant::k_initialMeshletTriangleOffset;

        std::uint32_t m_triangleCount = Constant::k_initialMeshletTriangleCount;
    };

    // Meshlet 1個分のカリング用境界情報
    // Meshlet単位のFrustumCullingやBackface Cone Cullingで使用する
    struct ModelMeshletBounds final
    {
        TypeAlias::Math::Vector3 m_center = {};

        float m_radius = Constant::k_initialMeshletBoundsRadius;

        TypeAlias::Math::Vector3 m_coneApex = {};

        float m_coneCutoff = Constant::k_initialMeshletBoundsConeCutoff;

        TypeAlias::Math::Vector3 m_coneAxis = {};

        // GPU側の構造体サイズを16bytes単位に揃えるためのPadding
        // Vector3 + float の形にして、HLSL側のStructuredBufferで扱いやすくする
        float m_padding = Constant::k_initialMeshletBoundsPadding;
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
        std::uint32_t m_meshletOffset = Constant::k_initialMeshLODMeshletOffset;
        std::uint32_t m_meshletCount  = Constant::k_initialMeshLODMeshletCount;

        std::uint32_t m_triangleCount = Constant::k_initialMeshLODTriangleCount;

        // 元の形からどれだけ形が崩れたか(モデルのローカル座標での距離)
        // 描画時にこの値を画面上のピクセル数へ換算し、1ピクセル程度以下ならこのLODを使う
        float m_simplifyError = Constant::k_initialMeshLODSimplifyError;

        // モーフ(ブレンドシェイプ)のターゲット1つをウェイト1で効かせたときに、このLODで起きる崩れ(モデルのローカル座標での距離)
        // モーフの崩れはウェイトに比例するため、描画時は「m_simplifyError + m_morphError × モーフのウェイト」でLODを選ぶ
        // (モーフを持たないMeshは0)
        // ※ GPUへそのまま送るため、HLSL側(ModelGPUDriven.hlsliのModelGPUDrivenMeshLOD)と同じ並びにする
        float m_morphError = Constant::k_initialMeshLODMorphError;
    };

    // 1つのModelMeshが持つMeshlet関連データ一式
    // ※ MeshShader描画では、このデータをGPU Buffer化してShader側から参照する
    struct ModelMeshletData final
    {
        std::vector<ModelMeshlet> m_meshletList = {};

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

        // 頂点ごとの「外側に重なっている別のMeshの数」(ModelLODLayerDetectorが自動で求める。要素数は頂点数、上限はConstant::k_maxLODCoveredLayerCount)
        // LODを描くときに、覆われている頂点だけを「この数 × LODの許容誤差」だけカメラから見て奥へ押し出し、
        // 重なっている服と肌などが簡略化で前後に入れ替わって透けるのを防ぐ(視線の上で動かすので、形や輪郭は変わらない)
        // ※ バイナリーファイルに保存するため、変えた場合はモデルのAssetVersionを上げる
        std::vector<std::uint32_t> m_lodCoveredLayerCountList = {};

        TypeAlias::Math::Vector3 m_boundsCenter = {};

        float m_boundsRadius = Constant::k_initialMeshletDataBoundsRadius;

        // LODで表面が実際にずれた量 ÷ meshoptimizerが求めた誤差(LODの選び方で使う誤差)の、モデル全体での最大値(1～4)
        // meshoptimizerの誤差は実際のずれより小さく出ることがあるため、覆われている頂点を奥へ押し出す距離に掛けて足りなくならないようにする
        // ※ バイナリーファイルに保存するため、変えた場合はモデルのAssetVersionを上げる
        float m_lodSurfaceDeviationScale = Constant::k_initialLODSurfaceDeviationScale;
    };

    struct ModelMeshRuntimeDataBase
    {
         ModelMeshRuntimeDataBase() = default;
        ~ModelMeshRuntimeDataBase() = default;

        ModelMeshRuntimeDataBase(const ModelMeshRuntimeDataBase&)           = delete;
        ModelMeshRuntimeDataBase(      ModelMeshRuntimeDataBase&&) noexcept = default;

        ModelMeshRuntimeDataBase& operator=(const ModelMeshRuntimeDataBase&)           = delete;
        ModelMeshRuntimeDataBase& operator=(      ModelMeshRuntimeDataBase&&) noexcept = default;

        Graphics::StaticStructuredBuffer m_vertexBuffer = {};

        Graphics::StaticStructuredBuffer m_meshletBuffer = {};

        Graphics::StaticStructuredBuffer m_uniqueVertexIndexBuffer = {};

        Graphics::StaticStructuredBuffer m_primitiveIndexBuffer = {};

        Graphics::StaticStructuredBuffer m_meshletBoundsBuffer = {};

        Graphics::StaticStructuredBuffer m_lodCoveredLayerCountBuffer = {};
    };
}