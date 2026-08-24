#pragma once

namespace FWK
{
	class MouseController final
	{
	public:

		enum class MouseButton : std::uint8_t
		{
			Left,
			Middle,
			Right,
			XButton1,
			XButton2
		};

	public:

		MouseController() = default;
		~MouseController() = default;

		void INIT();

		void Update();

		bool IsButtonDown    (const MouseButton a_button) const;
		bool IsButtonUp      (const MouseButton a_button) const;
		bool IsButtonPressed (const MouseButton a_button) const;
		bool IsButtonReleased(const MouseButton a_button) const;

		void SetVisible(const bool a_set);

		const auto& GetREFCurrentPosition() const { return m_position; }
		const auto& GetREFMovement       () const { return m_movement; }

		int FetchVALScrollWheelValue() const;

	private:

		DirectX::Mouse::ButtonStateTracker::ButtonState FetchVALButtonState(const MouseButton a_button) const;

		DirectX::Mouse                     m_mouse              = {};
		DirectX::Mouse::ButtonStateTracker m_buttonStateTracker = {};
		DirectX::Mouse::State              m_state              = {};

		TypeAlias::Math::Vector2 m_currentPosition  = TypeAlias::Math::Vector2::Zero;
		TypeAlias::Math::Vector2 m_previousPosition = TypeAlias::Math::Vector2::Zero;
		TypeAlias::Math::Vector2 m_movement         = TypeAlias::Math::Vector2::Zero;

		bool m_isPositionInitialized = false;
	};
}