#pragma once

namespace FWK::Utility
{
	inline boost::uuids::uuid StringToUUID(const std::string& a_string)
	{
		// から文字列は有効なUUIDではないため、Boost.UUIDのnilUUIDを返す
		if (a_string.empty()) { return {}; }

		const boost::uuids::string_generator l_generator     = {};
		      std::ptrdiff_t                 l_errorPosition = {};
		      boost::uuids::from_chars_error l_error         = boost::uuids::from_chars_error::none;

		const auto l_uuid = l_generator(a_string.begin(),
			                            a_string.end(),
			                            l_errorPosition,
			                            l_error);

		// 文字列をUUIDへ変換できなかった場合はnilUUIDを渡す
		if (l_error != boost::uuids::from_chars_error::none) { return{}; }

		return l_uuid;
	}
}