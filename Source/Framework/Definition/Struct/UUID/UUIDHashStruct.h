#pragma once

namespace FWK::Struct
{
	struct UUIDHashStruct final
	{
		std::size_t operator()(const UUID& a_uuid) const
		{
			RPC_STATUS l_status = {};

			// UuidHashはUUID*を要求するため、
			// const_castせずローカル変数へコピーして渡す
			UUID l_uuid = a_uuid;

			const auto l_hash = UuidHash(&l_uuid, &l_status);

			FWK_ASSERT_RETURN_VALUE_IF(l_status != RPC_S_OK, "ハッシュ化に失敗しました。", l_hash);

			return l_hash;
		}
	};
}