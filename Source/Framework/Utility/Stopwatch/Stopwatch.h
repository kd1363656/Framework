#pragma once

namespace FWK::Utility
{
	class Stopwatch final
	{
	public:

		 Stopwatch();
		~Stopwatch();

		Stopwatch(const Stopwatch&)			  = delete;
		Stopwatch(	    Stopwatch&&) noexcept = delete;

		Stopwatch& operator=(const Stopwatch&)			 = delete;
		Stopwatch& operator=(	   Stopwatch&&) noexcept = delete;

		double FetchElapsedSecond() const;

	private:

		std::chrono::steady_clock::time_point m_startTime = {};
	};
}