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

	private:

		Utility::AssetFilePath m_assetFilePath = {};

		FWK_DEFINE_TYPE_INFO(ModelComponentBase, ComponentBase)
	};
}