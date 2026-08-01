#pragma once

namespace FWK::Utility
{
	class AssetFilePath;
}

namespace FWK::Converter
{
	class AssetFilePathJsonConverter final
	{
	public:

		 AssetFilePathJsonConverter() = default;
		~AssetFilePathJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Utility::AssetFilePath& a_assetFilePath) const;

		nlohmann::json Serialize(const Utility::AssetFilePath& a_assetFilePath) const;

	private:

		static constexpr std::string_view k_assetFilePathJsonKey = "AssetFilePath";
	};
}