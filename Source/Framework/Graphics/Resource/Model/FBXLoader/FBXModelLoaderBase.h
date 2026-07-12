#pragma once

namespace FWK::Graphics
{
	class FBXModelLoaderBase
	{
	public:

				 FBXModelLoaderBase() = default;
		virtual ~FBXModelLoaderBase() = default;

		inline static const std::filesystem::path k_lowerFBXExtension = ".fbx";

	protected:

		ufbx_scene* LoadFBXScene(const std::filesystem::path& a_filePath) const;

		void ExtractModelMaterial(const ufbx_material* a_fbxMaterial, Struct::ModelMaterialAssetData& a_modelMaterialAssetData) const;

		void DestroyFBXScene(ufbx_scene* a_fbxScene) const;

		TypeAlias::Math::Vector3 FetchWorldVertexPosition(const ufbx_node* a_fbxNode, const ufbx_mesh*    a_fbxMesh, const std::uint32_t a_vertexIndex) const;
		TypeAlias::Math::Vector2 FetchVertexUV           (const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex)                                const;
		TypeAlias::Math::Vector3 FetchWorldVertexNormal  (const ufbx_node* a_fbxNode, const ufbx_mesh*    a_fbxMesh, const std::uint32_t a_vertexIndex) const;
		TypeAlias::Math::Vector4 FetchWorldVertexTangent (const ufbx_node* a_fbxNode, const ufbx_mesh*    a_fbxMesh, const std::uint32_t a_vertexIndex) const;

		std::wstring FetchMaterialTextureFilePath(const ufbx_material_map& a_materialMap) const;

		TypeAlias::Math::Color FetchBaseColorFactor(const ufbx_material_map& a_materialMap) const;

		float FetchMaterialFactor(const ufbx_material_map& a_materialMap, const float a_defaultValue) const;

	private:

		ufbx_load_opts CreateFBXLoadOptions() const;

#if defined(_DEBUG)
		static constexpr std::size_t k_errorTextBufferSize = 1024ULL;
#endif

		static constexpr ufbx_real k_modelFBXTargetUnitMeters = 1.0;

		static constexpr float k_uvCoordinateMax = 1.0F;

		static constexpr float k_defaultTangentX = 1.0F;
		static constexpr float k_defaultTangentY = 0.0F;
		static constexpr float k_defaultTangentZ = 0.0F;
		static constexpr float k_defaultTangentW = 1.0F;
	};
}