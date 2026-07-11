#pragma once

namespace FWK::Converter
{
	class StaticModelBinaryConverter final : public ModelBinaryConverterBase
	{
	private:

		struct ModelBinaryHeader final
		{
			std::uint64_t m_fileSize    = k_emptyAssetFileSize;
			std::uint16_t m_version     = k_modelAssetVersion;
			std::uint16_t m_assetTypeID = k_modelAssetTypeID;

			std::uint64_t m_modelMeshCount = Constant::k_emptyModelMeshCount;
		};

	public:

		 StaticModelBinaryConverter()		   = default;
		~StaticModelBinaryConverter() override = default;

		bool LoadAsset(const std::filesystem::path& a_filePath, Graphics::StaticModelRecord& a_staticModelRecord);

		bool SaveAsset(const Graphics::StaticModelRecord& a_staticModelRecord, const std::filesystem::path& a_filePath);

	private:

		void FailLoadAsset(Graphics::StaticModelRecord::ModelData& a_modelData);

		ModelBinaryHeader CreateModelBinaryHeader(const Graphics::StaticModelRecord::ModelData& a_modelData, const std::uint64_t& a_fileSize) const;

		std::uint64_t CalculateAssetFileSize(const Graphics::StaticModelRecord::ModelData& a_modelData) const;

		// 'S' = 0x53, 'T' = 0x54のため、0x5354で"ST"を表す
		static constexpr std::uint16_t k_modelAssetTypeID = 0x5354U;

		// ※ 注意 : Assetとして保存する構造体が変化したらバージョンを上げる
		static constexpr std::uint16_t k_modelAssetVersion = 1U;
	};
}