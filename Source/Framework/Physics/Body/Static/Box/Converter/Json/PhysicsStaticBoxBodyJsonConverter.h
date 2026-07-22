#pragma once

namespace FWK::Physics
{
	class PhysicsStaticBoxBody;
}

namespace FWK::Converter
{
	class PhysicsStaticBoxBodyJsonConverter
	{
	public:

		 PhysicsStaticBoxBodyJsonConverter() = default;
		~PhysicsStaticBoxBodyJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, Physics::PhysicsStaticBoxBody& a_physicsStaticBoxBody) const;

		nlohmann::json Serialize(const Physics::PhysicsStaticBoxBody& a_physicsStaticBoxBody) const;

	private:

		static constexpr std::string_view k_halfExtentJsonKey = "HalfExtent";
	};
}