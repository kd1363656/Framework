#pragma once

namespace FWK
{
	class RotationComponentModeBase;
}

namespace FWK::Converter
{
	class RotationComponentModeBaseJsonConverter final
	{
	public:

		 RotationComponentModeBaseJsonConverter() = default;
		~RotationComponentModeBaseJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_rotationComponentModeBase) const;

		nlohmann::json Serialize(const RotationComponentModeBase& a_rotationComponentModeBase) const;

	private:

		void DeserializeRotationApplyAxisList(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_rotationComponentModeBase) const;

		nlohmann::json SerializeRotationApplyAxisList(const RotationComponentModeBase& a_rotationComponentModeBase) const;

		static constexpr std::string_view k_rotationApplyAxisListJsonKey = "RotationApplyAxisList";
		static constexpr std::string_view k_rotationApplyAxisJsonKey     = "RotationApplyAxis";
	};
}