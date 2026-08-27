#pragma once

namespace FWK::Enum
{
	enum class Axis : std::uint32_t
	{
		Invalid = 0U,
		X       = 1U << 0U,
		Y       = 2U << 0U,
		Z       = 3U << 0U
	};
}