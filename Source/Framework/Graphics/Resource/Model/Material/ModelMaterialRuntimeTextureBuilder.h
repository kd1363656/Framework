#pragma once

namespace FWK::Graphics
{
	template <Concept::IsDerivedAssetRecordBaseConcept ModelRecordType>
	class ModelMaterialRuntimeTextureBuilder final
	{
	public:

		 ModelMaterialRuntimeTextureBuilder() = default;
		~ModelMaterialRuntimeTextureBuilder() = default;

		void BuildMaterialRuntimeTextures(const std::filesystem::path& a_filePath, ModelRecordType& a_modelRecord) const
		{
			// ランタイムパラメータを作成していく
			for (auto& l_modelMesh : a_modelRecord.GetMutableREFModelData().m_modelMeshList)
			{
				const auto& l_modelMaterialAssetData   = l_modelMesh.m_modelMaterial.m_modelMaterialAssetData;
					  auto& l_modelMaterialRuntimeData = l_modelMesh.m_modelMaterial.m_modelMaterialRuntimeData;

				// ベースカラーテクスチャの読み込み
				l_modelMaterialRuntimeData.m_baseColorTexture = CreateSingleMaterialTexture(a_filePath, 
																					        l_modelMaterialAssetData.m_baseColorTextureFilePath, 
																					        Enum::TextureLoadColorSpace::SRGB, 
																					        Enum::DefaultTextureType::BaseColor);

				// ノーマルテクスチャの読み込み
				l_modelMaterialRuntimeData.m_normalTexture = CreateSingleMaterialTexture(a_filePath, 
																				         l_modelMaterialAssetData.m_normalTextureFilePath, 
																				         Enum::TextureLoadColorSpace::Linear,
																				         Enum::DefaultTextureType::Normal);

				// メタリックテクスチャの読み込み
				l_modelMaterialRuntimeData.m_metallicTexture = CreateSingleMaterialTexture(a_filePath, 
																				           l_modelMaterialAssetData.m_metallicTextureFilePath, 
																				           Enum::TextureLoadColorSpace::Linear,
																				           Enum::DefaultTextureType::Metallic);

				// ラフネステクスチャの読み込み
				l_modelMaterialRuntimeData.m_roughnessTexture = CreateSingleMaterialTexture(a_filePath, 
																				            l_modelMaterialAssetData.m_roughnessTextureFilePath, 
																				            Enum::TextureLoadColorSpace::Linear,
																				            Enum::DefaultTextureType::Roughness);
			}	
		}

	private:

		std::shared_ptr<Texture> CreateSingleMaterialTexture(const std::filesystem::path&      a_modelFilePath, 
													         const std::wstring&			   a_textureFilePath,
													         const Enum::TextureLoadColorSpace a_textureLoadColorSpace,
													         const Enum::DefaultTextureType    a_defaultTextureType) const
		{
			auto l_texture = std::make_shared<Texture>();

			std::filesystem::path l_textureFilePath = a_textureFilePath;

			// FBXから取得したTextureFilePathが相対パスの場合
			// ModelFilePathの親フォルダからの相対パスとして解決する
			if (l_textureFilePath.is_relative())
			{
				l_textureFilePath = a_modelFilePath.parent_path() / l_textureFilePath;
			}

			l_texture->Load(l_textureFilePath, a_textureLoadColorSpace, a_defaultTextureType);

			return l_texture;
		}
	};
}