#pragma once

namespace FWK::Converter
{
	class StaticModelBinaryConverter final : public BinaryFileConverterBase
	{
	private:

		struct ModelBinaryHeader final
		{
			std::uint64_t m_fileSize    = k_emptyAssetFileSize;
			std::uint16_t m_version     = k_staticModelAssetVersion;
			std::uint16_t m_assetTypeID = k_staticModelAssetTypeID;

			std::uint64_t m_modelMeshCount = k_emptyModelMeshCount;
		};

		struct ModelMeshBinaryHeader final
		{
			std::uint64_t m_vertexCount = k_emptyModelVertexCount;
			std::uint64_t m_indexCount  = k_emptyModelIndexCount;

			std::uint64_t m_baseColorTextureFilePathSize = k_emptyTextureFilePathSize;
			std::uint64_t m_normalTextureFilePathSize    = k_emptyTextureFilePathSize;
			std::uint64_t m_roughnessTextureFilePathSize = k_emptyTextureFilePathSize;
			std::uint64_t m_metallicTextureFilePathSize  = k_emptyTextureFilePathSize;

			std::uint64_t m_meshletCount		   = k_emptyModelMeshletCount;
			std::uint64_t m_uniqueVertexIndexCount = k_emptyModelUniqueVertexIndexCount;
			std::uint64_t m_primitiveIndexCount    = k_emptyModelPrimitiveIndexCount;
			std::uint64_t m_meshletBoundsCount     = k_emptyModelMeshletBoundsCount;
		};

	public:

		 StaticModelBinaryConverter()		   = default;
		~StaticModelBinaryConverter() override = default;

		bool LoadAsset(const std::filesystem::path& a_filePath, Graphics::StaticModelRecord& a_staticModelRecord);

		bool SaveAsset(const Graphics::StaticModelRecord& a_staticModelRecord, const std::filesystem::path& a_filePath);

		static constexpr std::size_t k_emptyModelMeshCount = 0ULL;

		static constexpr std::uint64_t k_emptyModelVertexCount            = 0ULL;
		static constexpr std::uint64_t k_emptyModelIndexCount             = 0ULL;
		static constexpr std::uint64_t k_emptyTextureFilePathSize         = 0ULL;
		static constexpr std::uint64_t k_emptyModelMeshletCount           = 0ULL;
		static constexpr std::uint64_t k_emptyModelUniqueVertexIndexCount = 0ULL;
		static constexpr std::uint64_t k_emptyModelPrimitiveIndexCount    = 0ULL;
		static constexpr std::uint64_t k_emptyModelMeshletBoundsCount     = 0ULL;

	private:

		bool CanLoadAsset(const std::filesystem::path& a_filePath) const;

		void FailLoadAsset(Graphics::StaticModelRecord::StaticModelData& a_staticModelData);

		ModelBinaryHeader CreateModelBinaryHeader(const Graphics::StaticModelRecord::StaticModelData& a_staticModelData, const std::uint64_t& a_fileSize) const;

		ModelMeshBinaryHeader CreateModelMeshBinaryHeader(const Graphics::StaticModelRecord::StaticModelMesh& a_staticModelMesh) const;

		std::uint64_t CalculateAssetFileSize(const Graphics::StaticModelRecord::StaticModelData& a_staticModelData) const;

		// 'S' = 0x53, 'T' = 0x54のため、0x5354で"ST"を表す
		static constexpr std::uint16_t k_staticModelAssetTypeID = 0x5354U;

		// ※ 注意 : Assetとして保存する構造体が変化したらバージョンを上げる
		static constexpr std::uint16_t k_staticModelAssetVersion = 1U;
	};
}