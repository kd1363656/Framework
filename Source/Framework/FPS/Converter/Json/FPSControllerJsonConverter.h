#pragma once

namespace FWK
{
	class FPSController;
}

namespace FWK::Converter
{
	class FPSControllerJsonConverter final
	{
	public:

		 FPSControllerJsonConverter() = default;
		~FPSControllerJsonConverter() = default;

		void Deserialize(const nlohmann::json& a_rootJson, FPSController& a_fpsController) const;

		nlohmann::json Serialize(const FPSController& a_fpsController)	const;

	private:

		static constexpr std::string_view k_targetFPSJsonKey = "TargetFPS";
	};
}