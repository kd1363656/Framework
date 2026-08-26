#pragma once

namespace FWK
{
	class RotationComponentModeBase
	{
	public:

		         RotationComponentModeBase() = default;
		virtual ~RotationComponentModeBase() = default;

		virtual void INIT() { /*必要に応じてオーバーライドしてください*/ };

		virtual void DeserializePrefab(const nlohmann::json&) { /*必要に応じてオーバーライドしてください*/ };

		virtual void PostDeserialize(const std::shared_ptr<GameObject>& a_owner);

		virtual nlohmann::json SerializePrefab() { return {}; }

	private:

		Utility::FetchTransformComponentFromSelfGameObjectHelper m_fetchTransformComponentFromSelfGameObjectHelper = {};

		TypeAlias::Math::Vector3 m_rotationDirection = TypeAlias::Math::Vector3::Zero;

		FWK_DEFINE_TYPE_INFO_ROOT(RotationComponentModeBase)
	};
}