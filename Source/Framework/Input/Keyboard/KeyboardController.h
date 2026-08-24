#pragma once

namespace FWK
{
	class KeyboardController final
	{
	public:

		 KeyboardController() = default;
		~KeyboardController() = default;

		void Update();

		bool IsKeyDown    (const DirectX::Keyboard::Keys a_key) const;
		bool IsKeyUp      (const DirectX::Keyboard::Keys a_key) const;
		bool IsKeyPressed (const DirectX::Keyboard::Keys a_key) const;
		bool IsKeyReleased(const DirectX::Keyboard::Keys a_key) const;

	private:

		DirectX::Keyboard                       m_keyboard     = {};
		DirectX::Keyboard::KeyboardStateTracker m_stateTracker = {};
		DirectX::Keyboard::State                m_state        = {};
	};
}