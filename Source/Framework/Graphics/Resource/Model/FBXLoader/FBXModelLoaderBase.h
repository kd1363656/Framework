#pragma once

namespace FWK::Graphics
{
	class FBXModelLoaderBase
	{
	public:

				 FBXModelLoaderBase() = default;
		virtual ~FBXModelLoaderBase() = default;

	protected:

		ufbx_scene* LoadFBXScene(const std::filesystem::path& a_filePath) const;

		void DestroyFBXScene(ufbx_scene* a_fbxScene) const;

		TypeAlias::Math::Vector3 FetchLocalVertexPosition(const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const;
		TypeAlias::Math::Vector2 FetchVertexUV           (const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const;
		TypeAlias::Math::Vector3 FetchLocalVertexNormal  (const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const;
		TypeAlias::Math::Vector4 FetchLocalVertexTangent (const ufbx_mesh* a_fbxMesh, const std::uint32_t a_vertexIndex) const;

		TypeAlias::Math::Vector3 TransformImportPosition(const TypeAlias::Math::Vector3& a_position) const;
		TypeAlias::Math::Vector3 TransformImportNormal  (const TypeAlias::Math::Vector3& a_normal)   const;
		TypeAlias::Math::Vector4 TransformImportTangent (const TypeAlias::Math::Vector4& a_tangent)  const;

		TypeAlias::Math::Vector3 ConvertUFBXVector3ToVector3(const ufbx_vec3& a_fbxVector) const;
		TypeAlias::Math::Vector2 ConvertUFBXVector2ToVector2(const ufbx_vec2& a_fbxVector) const;

	private:

		ufbx_load_opts CreateFBXLoadOptions() const;

#if defined(_DEBUG)
		static constexpr std::size_t k_errorTextBufferSize = 1024ULL;
#endif

		static constexpr float k_modelImportScale = 0.01F;

		static constexpr float k_importAxisNegative = -1.0F;

		static constexpr float k_uvCoordinateMax = 1.0F;

		static constexpr float k_defaultTangentX = 1.0F;
		static constexpr float k_defaultTangentY = 0.0F;
		static constexpr float k_defaultTangentZ = 0.0F;
		static constexpr float k_defaultTangentW = 1.0F;
	};
}