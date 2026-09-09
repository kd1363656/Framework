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

		void Deserialize(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_moveComponentModeBase) const;

		nlohmann::json Serialize(const RotationComponentModeBase& a_moveComponentModeBase) const;

	private:

		void DeserializeCanApplyRotationAxisList(const nlohmann::json& a_rootJson, RotationComponentModeBase& a_moveComponentModeBase) const;

		nlohmann::json SerializeCanApplyRotationAxisList(const RotationComponentModeBase& a_moveComponentModeBase) const;

		static constexpr std::string_view k_canApplyRotationAxisListJsonKey = "RotationApplyAxisList";
		static constexpr std::string_view k_canApplyRotationAxisJsonKey     = "RotationApplyAxis";
	};
}