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

	protected:

		const auto& GetREFAssetFilePathHelper() const { return m_assetFilePathHelper; }

	private:

		Utility::AssetFilePathHelper m_assetFilePathHelper = {};

		FWK_DEFINE_TYPE_INFO(ModelComponentBase, ComponentBase)
	};
}