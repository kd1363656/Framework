#pragma once

namespace FWK::Physics
{
	class PhysicsStaticBoxBody final : public PhysicsStaticBodyBase
	{
	private:

		// TODO
		friend class Scene;

	public:

		 PhysicsStaticBoxBody()          = default;
		~PhysicsStaticBoxBody() override = default;

		bool CreateBody(const TypeAlias::Math::Vector3& a_worldPosition, const bool a_isPushBackEnabled) override;

	private:

		JPH::RefConst<JPH::Shape> CreateShape() const;

		bool ApplyShapeChange();

		static constexpr float k_minHalfExtentLength     = 0.0;
		static constexpr float k_defaultHalfExtentLength = 0.5F;

		TypeAlias::Math::Vector3 m_halfExtent = { k_defaultHalfExtentLength, k_defaultHalfExtentLength, k_defaultHalfExtentLength };

		FWK_DEFINE_TYPE_INFO(PhysicsStaticBoxBody, PhysicsStaticBodyBase)
	};
}