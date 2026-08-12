#pragma once

namespace FWK::TypeAlias
{
	using PrefabUUIDSet = std::unordered_set<UUID, Struct::UUIDHashStruct, Struct::UUIDEqualStruct>;
}