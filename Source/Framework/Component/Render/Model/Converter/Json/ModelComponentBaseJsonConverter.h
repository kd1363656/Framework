#pragma once

namespace FWK
{
	class ModelComponentBase;
}

namespace FWK::Converter
{
	class ModelComponentBaseJsonConverter final
	{
	public:

		 ModelComponentBaseJsonConverter() = default;
		~ModelComponentBaseJsonConverter() = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson, ModelComponentBase& a_modelComponentBase) const;
		
		nlohmann::json SerializePrefab(const ModelComponentBase& a_modelComponentBase) const;

	private:

		static constexpr std::string_view k_assetFilePathHelperJsonKey = "AssetFilePathHelper";
	};
}