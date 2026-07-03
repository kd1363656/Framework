#include "FrameResourceJsonConverter.h"

void FWK::Converter::FrameResourceJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::FrameResource& a_frameResource) const
{
	if (a_rootJson.is_null()) { return; }

	// 定数バッファのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_dynamicBufferUploaderListJsonKey, nlohmann::json::array());
		!l_json.is_null())
	{
		DeserializeDynamicBuffer(l_json, a_frameResource);
	}

	// レンダーグラフ用レンダーターゲットテクスチャのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_renderGraphFrameResourceJsonkey, nlohmann::json{});
		!l_json.is_null())
	{
		auto& l_renderGraphFrameResource = a_frameResource.GetMutableREFRenderGraphFrameResource();

		l_renderGraphFrameResource.Deserialize(l_json);
	}
}

nlohmann::json FWK::Converter::FrameResourceJsonConverter::Serialize(const Graphics::FrameResource & a_frameResource) const
{
	nlohmann::json l_rootJson = {};

	auto& l_renderGraphFrameResource = a_frameResource.GetREFRenderGraphFrameResource();

	// 定数バッファのシリアライズ
	l_rootJson[k_dynamicBufferUploaderListJsonKey] = SerializeDynamicBuffer(a_frameResource);

	// レンダーグラフ用レンダーターゲットテクスチャのシリアライズ
	l_rootJson[k_renderGraphFrameResourceJsonkey] = l_renderGraphFrameResource.Serialize();

	return l_rootJson;
}

void FWK::Converter::FrameResourceJsonConverter::DeserializeDynamicBuffer(const nlohmann::json& a_rootJson, Graphics::FrameResource& a_frameResource) const
{
	if (a_rootJson.is_null())		       { return; }
	if (!Utility::IsJsonArray(a_rootJson)) { return; }
	
	for (const auto& l_json : a_rootJson)
	{
		std::shared_ptr<Graphics::DynamicBufferUploaderBase> l_dynamicBufferUploader = nullptr;

		// ダイナミックバッファクラスをデシリアライズ
		Utility::DeserializeInstanceType<TypeAlias::DynamicBufferSharedFactory>(l_json, k_dynamicBufferUploaderTypeNameJsonKey, l_dynamicBufferUploader);

		// 作製に成功していれば中身にポインタがしっかり入っているので初期化とデシリアライズを行う
		if (!l_dynamicBufferUploader) { continue; }
		
		if (l_json.contains(k_dynamicBufferUploaderJsonKey))
		{
			l_dynamicBufferUploader->Deserialize(l_json[k_dynamicBufferUploaderJsonKey]);
		}

		a_frameResource.AddDynamicBufferUploader(l_dynamicBufferUploader);
	}	
}

nlohmann::json FWK::Converter::FrameResourceJsonConverter::SerializeDynamicBuffer(const Graphics::FrameResource & a_frameResource) const
{
	nlohmann::json l_rootJsonArray = {};
	
	const auto& l_dynamicBufferUploaderList = a_frameResource.GetREFConstantBufferUploaderList();
	
	// 生成する定数バッファの名前とその定数バッファに必要な情報をSerialize
	for (const auto& l_dynamicBufferUploader : l_dynamicBufferUploaderList)
	{
		if (!l_dynamicBufferUploader) { continue; }

		nlohmann::json l_json = {};

		// インスタンスを復元できるようにシリアライズ
		Utility::UpdateJson(l_json, Utility::SerializeInstanceType(l_dynamicBufferUploader, k_dynamicBufferUploaderTypeNameJsonKey));

		l_json[k_dynamicBufferUploaderJsonKey] = l_dynamicBufferUploader->Serialize();

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}