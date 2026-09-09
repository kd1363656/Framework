#pragma once

namespace FWK
{
	class MoveComponentModeBase
	{
	public:

		         MoveComponentModeBase() = default;
		virtual ~MoveComponentModeBase() = default;

		virtual void INIT();

		virtual void Deserialize(const nlohmann::json& a_rootJson);

		virtual void PostDeserialize(const GameObject& a_owner);
		
		virtual void Update() = 0;

		virtual void EditInspector();

		void ResetMoveDirection();

		virtual nlohmann::json Serialize() const;

		void AddCanApplyMoveAxis(const Enum::Axis a_canApplyMoveAxis);

		const auto& GetREFCanApplyMoveAxisList() const { return m_canApplyMoveAxisList; }

		auto& GetMutableREFCanApplyMoveAxisList() { return m_canApplyMoveAxisList; }

	private:

		std::vector<Enum::Axis> m_canApplyMoveAxisList = {};

		Utility::FetchComponentEventObserverFromSelfGameObjectHelper m_fetchComponentEventObserverFromSelfGameObjectHelper = {};
		Utility::FetchTransformComponentFromSelfGameObjectHelper     m_fetchTransformComponentFromSelfGameObjectHelper     = {};

		TypeAlias::Math::Vector3 m_rotationDirection = TypeAlias::Math::Vector3::Zero;

		std::uint32_t m_canApplyMoveAxis = static_cast<std::uint32_t>(Enum::Axis::Invalid);

		FWK_DEFINE_TYPE_INFO_ROOT(MoveComponentModeBase)
	};
}