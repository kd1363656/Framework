#pragma once

namespace FWK::Converter
{
	class StaticModelBinaryConverter final : public BinaryFileConverterBase
	{
	private:

		struct StaticModelBinaryHeader final
		{
			std::uint64_t m_fileSize    = Constant::k_emptyAssetFileSize;
			std::uint16_t m_versioin    = k_staticModelAssetTypeID;
			std::uint16_t m_assetTypeID = k_staticModelAssetVersion;

			std::uint64_t m_modelMeshCount = Constant::k_emptyModelMeshCount;
		};

		struct StaticMOdelMeshBinaryHeader final
		{
			std::uint64_t m_vertexCount = Constant::k_emptyModelVertexCount;
			std::uint64_t m_indexCount  = Constant::k_emptyModelIndexCount;

			std::uint64_t m_baseColorTextureFilePathSize = Constant::k_emptyTextureFilePathSize;
			std::uint64_t m_normalTextureFilePathSize    = Constant::k_emptyTextureFilePathSize;
			std::uint64_t m_roughnessTextureFilePathSize = Constant::k_emptyTextureFilePathSize;
			std::uint64_t m_metallicTextureFilePathSize  = Constant::k_emptyTextureFilePathSize;

			std::uint64_t m_meshCount			   = Constant::k_emptyModelMeshletCount;
			std::uint64_t m_uniqueVertexIndexCount = Constant::k_emptyModelUniqueVertexIndexCount;
			std::uint64_t m_primitiveIndexCount    = Constant::k_emptyModelPrimitiveIndexCount;
			std::uint64_t m_meshletBoundsCount     = Constant::k_emptyModelMeshletBoundsCount;
		};

	public:

		 StaticModelBinaryConverter()		   = default;
		~StaticModelBinaryConverter() override = default;

		StaticModelBinaryConverter(const StaticModelBinaryConverter&)		    = delete;
		StaticModelBinaryConverter(	     StaticModelBinaryConverter&&) noexcept = default;

		StaticModelBinaryConverter& operator=(const StaticModelBinaryConverter&)		   = delete;
		StaticModelBinaryConverter& operator=(	    StaticModelBinaryConverter&&) noexcept = default;

	private:

		// 'S' = 0x53, 'T' = 0x54のため、0x5354で"ST"を表す
		static constexpr std::uint16_t k_staticModelAssetTypeID = 0x5354U;

		// ※ 注意 : Assetとして保存する構造体が変化したらバージョンを上げる
		static constexpr std::uint16_t k_staticModelAssetVersion = 6U;
	};
}