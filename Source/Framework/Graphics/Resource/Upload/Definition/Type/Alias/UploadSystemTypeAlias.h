#pragma once

namespace FWK::TypeAlias
{
	using PendingTextureBatchUploadRecordMap = std::unordered_map<std::wstring, Struct::TextureBatchUploadRecord, Struct::WStringHash, std::equal_to<>>;
}