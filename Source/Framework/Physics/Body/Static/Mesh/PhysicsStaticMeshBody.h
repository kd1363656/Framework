#pragma once

namespace FWK::Physics
{
	class PhysicsStaticMeshBody : public PhysicsStaticBodyBase
	{
	private:

		friend class Scene;

	public:

		 PhysicsStaticMeshBody()          = default;
		~PhysicsStaticMeshBody() override = default;

		bool CreateBody(const Graphics::StaticModelRecord::StaticModelData& a_staticModelData, const bool a_isPushBackEnabled, TypeAlias::Math::Matrix& a_worldMatrix) override;

		bool ApplyWorldTransform(TypeAlias::Math::Matrix& a_worldMatrix) override;

	private:

		JPH::RefConst<JPH::Shape> CreateShape(const Graphics::StaticModelRecord::StaticModelData& a_staticModelData) const;

		static constexpr float k_oppositeNormalDotThreshold = 0.0F;

		static constexpr std::size_t k_invalidTotalVertexCount = 0ULL;
		static constexpr std::size_t k_invalidTotalIndexCount  = 0ULL;

		static constexpr std::size_t k_triangleIndexZeroOffset = 0ULL;
		static constexpr std::size_t k_triangleIndexOneOffset  = 1ULL;
		static constexpr std::size_t k_triangleIndexTwoOffset  = 2ULL;

		static constexpr JPH::uint32 k_defaultMaterialIndex = 0U;

		FWK_DEFINE_TYPE_INFO(PhysicsStaticMeshBody, PhysicsStaticBodyBase)
	};
}