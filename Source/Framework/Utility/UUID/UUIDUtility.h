#pragma once

namespace FWK::Utility
{
	inline std::string UUIDToString(const UUID& a_uuid)
	{
		RPC_CSTR l_str = nullptr;

		if (UuidToStringA(&a_uuid, &l_str) != RPC_S_OK ||
			!l_str)
		{
			return {};
		}

		std::string l_result(reinterpret_cast<char*>(l_str));

		RpcStringFreeA(&l_str);

		return l_result;
	}

	inline UUID StringToUUID(const std::string& a_string)
	{
		UUID l_uuid = GUID_NULL;

		if (auto l_rpcStr = reinterpret_cast<RPC_CSTR>(const_cast<char*>(a_string.c_str()));
			UuidFromStringA(l_rpcStr, &l_uuid) != RPC_S_OK)
		{
			return GUID_NULL;
		}

		return l_uuid;
	}
}