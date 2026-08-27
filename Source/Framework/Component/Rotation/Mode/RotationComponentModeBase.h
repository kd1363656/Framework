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
		
		void ResetRotationDirection();

		virtual void EditInspector() { /*必要に応じてオーバーライドしてください*/ };

		virtual nlohmann::json SerializePrefab() { return {}; }

	private:

		std::vector<Enum::Axis> m_rotationApplyAxisList = {};

		Utility::FetchTransformComponentFromSelfGameObjectHelper m_fetchTransformComponentFromSelfGameObjectHelper = {};

		TypeAlias::Math::Vector3 m_rotationDirection = TypeAlias::Math::Vector3::Zero;

		FWK_DEFINE_TYPE_INFO_ROOT(RotationComponentModeBase)
	};
}