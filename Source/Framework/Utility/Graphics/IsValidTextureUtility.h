#pragma once

namespace FWK::Utility
{
	inline bool IsValidTextureSize(const UINT a_width, const UINT a_height) 
	{
		if (a_width  == Converter::TextureBinaryConverter::k_emptyTextureWidth ||
			a_height == Converter::TextureBinaryConverter::k_emptyTextureHeight)
		{
			return false;
		}

		return true;
	}
}