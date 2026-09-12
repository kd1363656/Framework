#pragma once

namespace FWK
{
	class RotationComponentConstantMouseMode;
}

namespace FWK::Converter
{
	class RotationComponentConstantMouseModeJsonConverter final
	{
	public:

		 RotationComponentConstantMouseModeJsonConverter() = default;
		~RotationComponentConstantMouseModeJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, RotationComponentConstantMouseMode& a_rotationComponentConstantMouseMode) const;

		nlohmann::json Serialize(const RotationComponentConstantMouseMode& a_rotationComponentConstantMouseMode) const;

	private:

		static constexpr std::string_view k_xAxisRotatableRangeMAXJsonKey = "XAxisRotatableRangeMAX";
		static constexpr std::string_view k_xAxisRotatableRangeMINJsonKey = "XAxisRotatableRangeMIN";
	};
}