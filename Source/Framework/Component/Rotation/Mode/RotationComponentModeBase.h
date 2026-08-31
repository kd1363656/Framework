#pragma once

namespace FWK
{
	class RotationComponentModeBase
	{
	public:

		         RotationComponentModeBase() = default;
		virtual ~RotationComponentModeBase() = default;

		virtual void INIT();

		virtual void DeserializePrefab(const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };

		virtual void PostDeserialize(const std::shared_ptr<GameObject>& a_owner);
		
		virtual void EditInspector() { /*必要に応じてオーバーライドしてください*/ };

		void ResetRotationDirection();

		virtual nlohmann::json SerializePrefab() { return {}; }

	private:

		std::vector<Enum::Axis> m_rotationApplyAxisList = {};

		Utility::FetchTransformComponentFromSelfGameObjectHelper m_fetchTransformComponentFromSelfGameObjectHelper = {};

		TypeAlias::Math::Vector3 m_rotationDirection = TypeAlias::Math::Vector3::Zero;

		std::uint32_t m_applyRotationAxis = static_cast<std::uint32_t>(Enum::Axis::Invalid);

		FWK_DEFINE_TYPE_INFO_ROOT(RotationComponentModeBase)
	};
}