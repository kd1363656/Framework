#pragma once

namespace FWK
{
	class AudioManager final : public Utility::SingletonBase<AudioManager>
	{
	private:

		friend class SingletonBase<AudioManager>;

		 AudioManager()          = default;
		~AudioManager() override = default;

	private:

	};
}