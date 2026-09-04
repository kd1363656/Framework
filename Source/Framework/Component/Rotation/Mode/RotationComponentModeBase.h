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

		void AddRotationApplyAxis(const Enum::Axis a_applyRotationAxis);

		const auto& GetREFRotationApplyAxisList() const { return m_rotationApplyAxisList; }

		auto& GetMutableREFRotationApplyAxisList() { return m_rotationApplyAxisList; }

	private:

		std::vector<Enum::Axis> m_rotationApplyAxisList = {};

		Utility::FetchTransformComponentFromSelfGameObjectHelper m_fetchTransformComponentFromSelfGameObjectHelper = {};

		RotationComponentModeBaseInspector m_inspector = {};

		Converter::RotationComponentModeBaseJsonConverter m_jsonConverter = {};

		TypeAlias::Math::Vector3 m_rotationDirection = TypeAlias::Math::Vector3::Zero;

		std::uint32_t m_applyRotationAxis = static_cast<std::uint32_t>(Enum::Axis::Invalid);

		FWK_DEFINE_TYPE_INFO_ROOT(RotationComponentModeBase)
	};
}