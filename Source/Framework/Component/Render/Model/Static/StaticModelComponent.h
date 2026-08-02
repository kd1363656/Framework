#pragma once

namespace FWK
{
	class StaticModelComponent final : public ModelComponentBase
	{
	public:

	     StaticModelComponent()          = default;
		~StaticModelComponent() override = default;

		void DeserializePrefab(const nlohmann::json& a_rootJson) override;

		nlohmann::json SerializePrefab() override;

	private:

		FWK_DEFINE_TYPE_INFO(StaticModelComponent, ModelComponentBase)
	};
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::ComponentSharedFactory, FWK::StaticModelComponent)