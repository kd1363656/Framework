#pragma once

namespace FWK::Physics
{
	class PhysicsBodyBase
	{
	protected:

		// 派生クラスだけがBodyを設定できるようにprotectedにする
		explicit PhysicsBodyBase(const JPH::BodyID a_bodyID);

	public:

				 PhysicsBodyBase();
		virtual ~PhysicsBodyBase();

		
		PhysicsBodyBase(const PhysicsBodyBase&)  = delete;
		PhysicsBodyBase(      PhysicsBodyBase&&) = delete;

		PhysicsBodyBase& operator=(const PhysicsBodyBase&)  = delete;
		PhysicsBodyBase& operator=(      PhysicsBodyBase&&) = delete;

		TypeAlias::Math::Vector3 FetchVALWorldPosition() const;

		bool FetchVALIsValid() const;

	protected:

		const auto& GetREFBodyID() const { return m_bodyID; }

	private:

		void ReleaseBody();

		JPH::BodyID m_bodyID;
	};
}