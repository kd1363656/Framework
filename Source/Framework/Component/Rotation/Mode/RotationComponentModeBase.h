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

		virtual void PostDeserialize(const std::shared_ptr<GameObject>& a_owner);
		
		virtual void Update() = 0;

		virtual void EditInspector();

		void ResetRotationDirection();

		virtual nlohmann::json Serialize() const;

		void AddCanApplyRotationAxis(const Enum::Axis a_canApplyRotationAxis);

		const auto& GetREFCanApplyRotationAxisList() const { return m_canApplyRotationAxisList; }

		auto& GetMutableREFCanApplyRotationAxisList() { return m_canApplyRotationAxisList; }

	protected:

		bool CanUpdate();

	private:

		std::vector<Enum::Axis> m_canApplyRotationAxisList = {};

		Utility::FetchComponentEventObserverFromSelfGameObjectHelper m_fetchComponentEventObserverFromSelfGameObjectHelper = {};
		Utility::FetchTransformComponentFromSelfGameObjectHelper     m_fetchTransformComponentFromSelfGameObjectHelper     = {};

		RotationComponentModeBaseInspector m_inspector = {};

		Converter::RotationComponentModeBaseJsonConverter m_jsonConverter = {};

		TypeAlias::Math::Vector3 m_rotationDirection = TypeAlias::Math::Vector3::Zero;

		std::uint32_t m_canApplyRotationAxis = static_cast<std::uint32_t>(Enum::Axis::Invalid);

		FWK_DEFINE_TYPE_INFO_ROOT(RotationComponentModeBase)
	};
}