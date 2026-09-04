#pragma once

namespace FWK
{
	class RotationComponent;
}

namespace FWK
{
	class RotationComponentInspector final
	{
	public:

		 RotationComponentInspector() = default;
		~RotationComponentInspector() = default;

		void EditInspector(RotationComponent& a_rotationComponent);

	private:

		static constexpr std::string_view k_rotationModeRadioButtonSelectorLabel = "回転モードラジオボタンセレクター";
	};
}