#pragma once

namespace FWK
{
	class RotationComponentConstantModeBase;
}

namespace FWK::Converter
{
	class RotationComponentConstantModeBaseJsonConverter final
	{
	public:

		 RotationComponentConstantModeBaseJsonConverter() = default;
		~RotationComponentConstantModeBaseJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, RotationComponentConstantModeBase& a_rotationComponentConstantModeBase) const;

		nlohmann::json Serialize(const RotationComponentConstantModeBase& a_rotationComponentConstantModeBase) const;

	private:

		static constexpr std::string_view k_rotationSpeedJsonKey = "RotationSpeed";
	};
}