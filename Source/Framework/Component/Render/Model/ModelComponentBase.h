#pragma once

namespace FWK
{
	class ModelComponentBase : public FWK::ComponentBase
	{
	public:

		 ModelComponentBase()          = default;
		~ModelComponentBase() override = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;

		nlohmann::json SerializePrefab() override;

		const auto& GetREFDrawRequestPassList() const { return m_assetFilePathHelper; }
		const auto& GetREFAssetFilePathHelper() const { return m_assetFilePathHelper; }

		auto& GetMutableREFAssetFilePathHelper() { return m_assetFilePathHelper; }

	private:

		std::vector<std::uint32_t> m_drawRequestPassList = {};

		Utility::AssetFilePathHelper m_assetFilePathHelper = {};

		Converter::ModelComponentBaseJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(ModelComponentBase, ComponentBase)
	};
}