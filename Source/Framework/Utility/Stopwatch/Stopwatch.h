#pragma once

namespace FWK::Utility
{
	class Stopwatch final
	{
	public:

		 Stopwatch();
		~Stopwatch();

		double FetchElapsedSecond() const;

	private:

		std::chrono::steady_clock::time_point m_startTime = {};
	};
}