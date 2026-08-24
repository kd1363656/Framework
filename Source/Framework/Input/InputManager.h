#pragma once

namespace FWK
{
	class InputManager final : public Utility::SingletonBase<InputManager>
	{
	private:

		friend class Utility::SingletonBase<InputManager>;

		 InputManager()          = default;
		~InputManager() override = default;

	public:

		void INIT();

		void Update();

		const auto& GetREFKeyboardController() const { return m_keyboardController; }
		const auto& GetREFMouseController   () const { return m_mouseController; }

	private:

		KeyboardController m_keyboardController = {};
		MouseController    m_mouseController    = {};
	};
}