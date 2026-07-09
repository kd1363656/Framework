#pragma once

namespace FWK::Graphics
{
	class StaticModelFBXLoader final : public FBXModelLoaderBase
	{
	public:

		 StaticModelFBXLoader()			 = default;
		~StaticModelFBXLoader() override = default;

		bool LoadStaticModelFile(const std::filesystem::path& a_filePath, Graphics::StaticModelRecord& a_staticModelRecord) const;

	private:

		bool ExtractModelData          (const ufbx_scene*    a_fbxScene,		    StaticModelRecord::StaticModelData&		         a_staticModelData)									               const;
		bool ExtractModelMeshList      (const ufbx_node*     a_fbxNode,			    std::vector<StaticModelRecord::StaticModelMesh>& a_staticModelMeshList)								               const;
		bool ExtractModelMeshByMaterial(const std::size_t&   a_materialIndex, const ufbx_node*							             a_fbxNode, StaticModelRecord::StaticModelMesh& a_staticModelMesh) const;
		void ExtractModelMaterial      (const ufbx_material* a_fbxMaterial, StaticModelRecord::ModelMaterialAssetData&               a_modelMaterialAssetData)						                   const;

		TypeAlias::Math::Color FetchBaseColorFactor(const ufbx_material_map& a_materialMap) const;

		float FetchMaterialFactor(const ufbx_material_map& a_materialMap, const float a_defaultValue) const;

		static constexpr std::string_view k_assetLoadSourceDebugText      = "Asset";
		static constexpr std::string_view k_ufbxLoadSourceDebugText       = "UFBX";
		static constexpr std::string_view k_emptyTextureFilePathDebugText = "None";

		static constexpr std::size_t k_invalidMaterialIndex = std::numeric_limits<std::size_t>::max();
	};
}