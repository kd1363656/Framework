#pragma once

namespace FWK::Utility
{
	class AssetFilePathHelper;
}

namespace FWK::Converter
{
	class AssetFilePathHelperJsonConverter final
	{
	public:

		 AssetFilePathHelperJsonConverter() = default;
		~AssetFilePathHelperJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Utility::AssetFilePathHelper& a_assetFilePathHelper) const;

		nlohmann::json Serialize(const Utility::AssetFilePathHelper& a_assetFilePathHelper) const;

	private:

		static constexpr std::string_view k_assetFilePathJsonKey = "AssetFilePath";
	};
}