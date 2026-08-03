#pragma once

namespace FWK
{
	class StaticModelComponent final : public ModelComponentBase
	{
	public:

	     StaticModelComponent()          = default;
		~StaticModelComponent() override = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;

		void PostDeserialize() override;

		nlohmann::json SerializePrefab() override;

	private:

		std::shared_ptr<Graphics::StaticModel>                       m_model           = std::make_shared<Graphics::StaticModel>                      ();
		std::shared_ptr<Struct::StaticModelPerObjectDrawRequestData> m_drawRequestData = std::make_shared<Struct::StaticModelPerObjectDrawRequestData>();

		FWK_DEFINE_TYPE_INFO(StaticModelComponent, ModelComponentBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::StaticModelComponent)