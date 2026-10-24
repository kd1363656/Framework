#pragma once

namespace FWK::Graphics
{
    // 各ModelMeshが使うMaterialを読み込むクラス
    // テクスチャの読み込みやシェーダーへ送るデータの作成は、Material自身が行う
    template <Concept::IsDerivedAssetRecordBaseConcept ModelRecordType>
    class ModelMaterialRuntimeBuilder final
    {
    public:

         ModelMaterialRuntimeBuilder() = default;
        ~ModelMaterialRuntimeBuilder() = default;

        void BuildMaterialRuntimeData(const std::filesystem::path& a_filePath, ModelRecordType& a_modelRecord) const
        {
            auto& l_modelData = a_modelRecord.GetMutableREFModelData();

            for (auto& l_modelMesh : l_modelData.m_modelMeshList)
            {
                auto& l_modelMaterial = l_modelMesh.m_modelMaterial;

                // .materialファイルが無い場合は、デフォルトマテリアルになる
                l_modelMaterial.m_material.Load(FetchVALMaterialFilePath(a_filePath, l_modelMaterial.m_materialFilePath));
            }
        }

    private:

        std::filesystem::path FetchVALMaterialFilePath(const std::filesystem::path& a_modelFilePath, const std::wstring& a_materialFilePath) const
        {
            // 空のパスはデフォルトマテリアルを表すため、そのまま返す
            if (a_materialFilePath.empty()) { return {}; }

            const std::filesystem::path l_materialFilePath = a_materialFilePath;

            // モデルファイルの親フォルダからの相対パスとして解決する
            if (l_materialFilePath.is_relative())
            {
                return a_modelFilePath.parent_path() / l_materialFilePath;
            }

            return l_materialFilePath;
        }
    };
}