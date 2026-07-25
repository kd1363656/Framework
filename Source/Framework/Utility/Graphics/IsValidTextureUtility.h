#pragma once

namespace FWK::Utility
{
	inline bool IsValidTextureSize(const UINT a_width, const UINT a_height) 
	{
		if (a_width  == Constant::k_invalidTextureWidth ||
			a_height == Constant::k_invalidTextureHeight)
		{
			return false;
		}

		return true;
	}
}