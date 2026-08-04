#pragma once

namespace FWK::Physics
{
	class PhysicsManager;
}

namespace FWK::Converter
{
	class PhyisicsManagerJsonConverter final
	{
	public:

		 PhyisicsManagerJsonConverter() = default;
		~PhyisicsManagerJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsManager& a_physicsManager) const;

		nlohmann::json Serialize(const Physics::PhysicsManager& a_physicsManager) const;

	private:

		static constexpr std::string_view k_isDisableDebugDrawJsonKey = "IsDisableDebugDraw";
	};
}