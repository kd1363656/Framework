#include "UploadSystemJsonConverter.h"

void FWK::Converter::UploadSystemJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::UploadSystem& a_uploadSystem) const
{
	if (a_rootJson.is_null()) { return; }

	const auto& l_copyCommandAllocatorListJson = a_rootJson.value(k_copyCommandAllocatorListJsonKey, nlohmann::json{});

	if (!l_copyCommandAllocatorListJson.is_null())
	{
		DeserializeCopyCommandAllocator(l_copyCommandAllocatorListJson, a_uploadSystem);
	}
}

nlohmann::json FWK::Converter::UploadSystemJsonConverter::Serialize(const Graphics::UploadSystem& a_uploadSystem) const
{
	nlohmann::json l_rootJson = {};

	l_rootJson[k_copyCommandAllocatorListJsonKey] = SerializeCopyCommandAllocator(a_uploadSystem);

	return l_rootJson;
}

void FWK::Converter::UploadSystemJsonConverter::DeserializeCopyCommandAllocator(const nlohmann::json& a_rootJson, Graphics::UploadSystem& a_uploadSystem) const
{
	auto l_copyCommandAllocatorCount = a_rootJson.value(k_copyCommandAllocatorListCountJsonKey, k_defaultCopyCommandAllocatorListCount);

	FWK_ASSERT_RETURN_IF_FAILED(l_copyCommandAllocatorCount == k_emptyCopyCommandAllocatorListCount, "コピーコマンドアロケータの作成数が0でとなっており、デシリアライズ処理に失敗しました。");
	
	for (std::size_t l_i = 0ULL; l_i < l_copyCommandAllocatorCount; ++l_i)
	{
		const auto& l_copyCommandAllocator = std::make_shared<Graphics::CopyCommandAllocator>();

		a_uploadSystem.AddCommandAllocator(l_copyCommandAllocator);
	}
}

nlohmann::json FWK::Converter::UploadSystemJsonConverter::SerializeCopyCommandAllocator(const Graphics::UploadSystem& a_uploadSystem) const
{
	const auto& l_copyCommandAllocatorList = a_uploadSystem.GetREFCopyCommandAllocatorList();

	nlohmann::json l_rootJson = {};

	l_rootJson[k_copyCommandAllocatorListCountJsonKey] = l_copyCommandAllocatorList.size();

	return l_rootJson;
}