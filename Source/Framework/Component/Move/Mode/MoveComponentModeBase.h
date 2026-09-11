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

		void AddCanApplyMoveAxisBitShiftFlag(const Enum::AxisBitShiftFlag a_canApplyMoveAxisBitShiftFlag);

		const auto& GetREFCanApplyMoveAxisAxisBitShiftFlagList() const { return m_canApplyMoveAxisBitShiftFlagList; }

		auto& GetMutableREFCanApplyMoveAxisAxisBitShiftFlagList() { return m_canApplyMoveAxisBitShiftFlagList; }

	private:

		std::vector<Enum::AxisBitShiftFlag> m_canApplyMoveAxisBitShiftFlagList = {};

		Utility::FetchComponentEventObserverFromSelfGameObjectHelper m_fetchComponentEventObserverFromSelfGameObjectHelper = {};
		Utility::FetchTransformComponentFromSelfGameObjectHelper     m_fetchTransformComponentFromSelfGameObjectHelper     = {};

		TypeAlias::Math::Vector3 m_moveDirection = TypeAlias::Math::Vector3::Zero;

		std::uint32_t m_canApplyMoveAxis = static_cast<std::uint32_t>(Enum::AxisBitShiftFlag::Invalid);

		FWK_DEFINE_TYPE_INFO_ROOT(MoveComponentModeBase)
	};
}