#pragma once

namespace FWK::Utility
{
	inline bool IsSameSize(const UINT a_sourceWidth,
						   const UINT a_sourceHeight,
						   const UINT a_targetWidth,
						   const UINT a_targetHeight) 
	{
		if (a_sourceWidth  != a_targetWidth ||
			a_sourceHeight != a_targetHeight)
		{
			return false;
		}

		return true;
	}
}