#pragma once

namespace FWK::Converter
{
	class StaticModelBinaryConverter final : public BinaryFileConverterBase
	{
	private:

		struct StaticModelBinaryHeader final
		{
			std::uint64_t m_fileSize    = Constant::k_emptyAssetFileSize;
			std::uint16_t m_version     = k_staticModelAssetVersion;
			std::uint16_t m_assetTypeID = k_staticModelAssetTypeID;

			std::uint64_t m_modelMeshCount = Constant::k_emptyModelMeshCount;
		};

		struct StaticModelMeshBinaryHeader final
		{
			std::uint64_t m_vertexCount = Constant::k_emptyModelVertexCount;
			std::uint64_t m_indexCount  = Constant::k_emptyModelIndexCount;

			std::uint64_t m_baseColorTextureFilePathSize = Constant::k_emptyTextureFilePathSize;
			std::uint64_t m_normalTextureFilePathSize    = Constant::k_emptyTextureFilePathSize;
			std::uint64_t m_roughnessTextureFilePathSize = Constant::k_emptyTextureFilePathSize;
			std::uint64_t m_metallicTextureFilePathSize  = Constant::k_emptyTextureFilePathSize;

			std::uint64_t m_meshletCount		   = Constant::k_emptyModelMeshletCount;
			std::uint64_t m_uniqueVertexIndexCount = Constant::k_emptyModelUniqueVertexIndexCount;
			std::uint64_t m_primitiveIndexCount    = Constant::k_emptyModelPrimitiveIndexCount;
			std::uint64_t m_meshletBoundsCount     = Constant::k_emptyModelMeshletBoundsCount;
		};

	public:

		 StaticModelBinaryConverter()		   = default;
		~StaticModelBinaryConverter() override = default;

		StaticModelBinaryConverter(const StaticModelBinaryConverter&)		    = delete;
		StaticModelBinaryConverter(	     StaticModelBinaryConverter&&) noexcept = delete;

		StaticModelBinaryConverter& operator=(const StaticModelBinaryConverter&)		   = delete;
		StaticModelBinaryConverter& operator=(	    StaticModelBinaryConverter&&) noexcept = delete;

		bool LoadStaticModelAsset(const std::filesystem::path& a_filePath, Graphics::StaticModelRecord& a_staticModelRecord);

		bool SaveStaticModelAsset(const std::filesystem::path& a_filePath, const Graphics::StaticModelRecord& a_staticModelRecord);

	private:

		bool CanLoadStaticModelAsset(const std::filesystem::path& a_filePath) const;

		void FailLoadStaticModelAsset(Struct::StaticModelData& a_staticModelData);

		StaticModelBinaryHeader CreateStaticModelBinaryHeader(const Struct::StaticModelData& a_staticModelData, const std::uint64_t& a_fileSize) const;

		StaticModelMeshBinaryHeader CreateStaticModelMeshBinaryHeader(const Struct::StaticModelMesh& a_staticModelMesh) const;

		std::uint64_t CalculateStaticModelAssetFileSize(const Struct::StaticModelData& a_staticModelData) const;

		// 'S' = 0x53, 'T' = 0x54のため、0x5354で"ST"を表す
		static constexpr std::uint16_t k_staticModelAssetTypeID = 0x5354U;

		// ※ 注意 : Assetとして保存する構造体が変化したらバージョンを上げる
		static constexpr std::uint16_t k_staticModelAssetVersion = 1U;
	};
}