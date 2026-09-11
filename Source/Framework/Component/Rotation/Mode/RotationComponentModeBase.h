#pragma once

namespace FWK
{
	class RotationComponentModeBase
	{
	public:

		         RotationComponentModeBase() = default;
		virtual ~RotationComponentModeBase() = default;

		virtual void INIT();

		virtual void Deserialize(const nlohmann::json& a_rootJson);

		virtual void PostDeserialize(const GameObject& a_owner);
		
		virtual void Update() = 0;

		virtual void EditInspector();

		void ResetRotationDirection();

		virtual nlohmann::json Serialize() const;

		void AddCanApplyRotationAxisBitShiftFlag(const Enum::AxisBitShiftFlag a_canApplyRotationAxisBitShiftFlag);

		const auto& GetREFCanApplyRotationAxisBitShiftFlagList() const { return m_canApplyRotationAxisBitShiftFlagList; }

		auto& GetMutableREFCanApplyRotationAxisBitShiftFlagList() { return m_canApplyRotationAxisBitShiftFlagList; }

	protected:

		bool CanUpdate();

	private:

		std::vector<Enum::AxisBitShiftFlag> m_canApplyRotationAxisBitShiftFlagList = {};

		Utility::FetchComponentEventObserverFromSelfGameObjectHelper m_fetchComponentEventObserverFromSelfGameObjectHelper = {};
		Utility::FetchTransformComponentFromSelfGameObjectHelper     m_fetchTransformComponentFromSelfGameObjectHelper     = {};

		RotationComponentModeBaseInspector m_inspector = {};

		Converter::RotationComponentModeBaseJsonConverter m_jsonConverter = {};

		TypeAlias::Math::Vector3 m_rotationDirection = TypeAlias::Math::Vector3::Zero;

		std::uint32_t m_canApplyRotationAxisBitShiftFlag = static_cast<std::uint32_t>(Enum::AxisBitShiftFlag::Invalid);

		FWK_DEFINE_TYPE_INFO_ROOT(RotationComponentModeBase)
	};
}