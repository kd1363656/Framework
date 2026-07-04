#pragma once

namespace FWK::Graphics
{
	class PhysicsBodyBase
	{
	protected:

		// 派生クラスだけがBodyを設定できるようにprotectedにする
		explicit PhysicsBodyBase(const std::shared_ptr<JPH::PhysicsSystem>& a_physicsSystem, const JPH::BodyID& a_bodyID);

	public:

				 PhysicsBodyBase();
		virtual ~PhysicsBodyBase();

		
		PhysicsBodyBase(const PhysicsBodyBase&)          = delete;
		PhysicsBodyBase(      PhysicsBodyBase&& a_other) = delete;

		PhysicsBodyBase& operator=(const PhysicsBodyBase&)          = delete;
		PhysicsBodyBase& operator=(      PhysicsBodyBase&& a_other) = delete;

		TypeAlias::Math::Vector3 FetchVALWorldPosition() const;

		bool FetchVALIsValid() const;

	protected:

		const auto& GetREFBodyID() const { return m_bodyID; }

	private:

		void ReleaseBody();

		JPH::BodyID m_bodyID;
	};
}