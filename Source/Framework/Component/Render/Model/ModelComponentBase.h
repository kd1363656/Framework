#pragma once

namespace FWK
{
	class ModelComponentBase : public FWK::ComponentBase
	{
	public:

		 ModelComponentBase()          = default;
		~ModelComponentBase() override = default;

		void INIT() override;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;
		void PostDeserialize  ()                                 override;

		void EditInspector() override;

		nlohmann::json SerializePrefab() override;

		const auto& GetREFDrawRequestPassList() const { return m_assetFilePathHelper; }
	
		std::weak_ptr<Utility::AssetFilePathHelper> GetVALAssetFilePathHelper() const { return m_assetFilePathHelper; }

		const auto& GetREFFetchTransformComponentFromSelfGameObjectHelper() const { return m_fetchTransformComponentFromSelfGameObjectHelper; }

	private:

		std::shared_ptr<Utility::AssetFilePathHelper> m_assetFilePathHelper = {};

		Utility::FetchTransformComponentFromSelfGameObjectHelper m_fetchTransformComponentFromSelfGameObjectHelper = {};

		ModelComponentBaseInspector m_inspector = {};

		Converter::ModelComponentBaseJsonConverter m_jsonConverter = {};

		FWK_DEFINE_TYPE_INFO(ModelComponentBase, ComponentBase)
	};
}