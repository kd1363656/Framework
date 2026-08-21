#pragma once

namespace FWK
{
	class StaticModelRegisterDrawRequestStrategyBase;
}

namespace FWK
{
	class StaticModelComponent final : public ModelComponentBase
	{
	public:

	     StaticModelComponent()          = default;
		~StaticModelComponent() override = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;

		void PostDeserialize() override;

		void PostLateUpdate() override;

		void EditInspector() override;

		nlohmann::json SerializePrefab() override;

		void AddRegisterDrawRequestStrategy(std::unique_ptr<StaticModelRegisterDrawRequestStrategyBase>&& a_registerDrawRequestStrategy);

		const auto& GetREFRegisterDrawRequestStrategyMap() const { return m_registerDrawRequestStrategyMap; }

		const auto& GetREFDrawRequestData() const { return m_drawRequestData; }

		auto& GetMutableRegisterDrawRequestStrategyMap() { return m_registerDrawRequestStrategyMap; }

	private:

		void UpdateDrawRequestData();

		std::unordered_map<TypeAlias::StaticTypeID, std::unique_ptr<StaticModelRegisterDrawRequestStrategyBase>> m_registerDrawRequestStrategyMap = {};

		std::shared_ptr<Graphics::StaticModel>                       m_model           = std::make_shared<Graphics::StaticModel>                      ();
		std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData> m_drawRequestData = std::make_shared<Struct::StaticModelPerObjectDrawRequestData>();

		StaticModelComponentInspector m_inspector = {};

		FWK_DEFINE_TYPE_INFO(StaticModelComponent, ModelComponentBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::StaticModelComponent)