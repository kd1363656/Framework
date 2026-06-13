#include "FrameResourceJsonConverter.h"

void FWK::Converter::FrameResourceJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::FrameResource& a_frameResource) const
{
	if (a_rootJson.is_null()) { return; }

	// 定数バッファのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_constantBufferUploaderListJsonKey, nlohmann::json::array());
		!l_json.is_null())
	{
		DeserializeConstantBuffer(l_json, a_frameResource);
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
	l_rootJson[k_constantBufferUploaderListJsonKey] = SerializeConstantBuffer(a_frameResource);

	// レンダーグラフ用レンダーターゲットテクスチャのシリアライズ
	l_rootJson[k_renderGraphFrameResourceJsonkey] = l_renderGraphFrameResource.Serialize();

	return l_rootJson;
}

void FWK::Converter::FrameResourceJsonConverter::DeserializeConstantBuffer(const nlohmann::json& a_rootJson, Graphics::FrameResource& a_frameResource) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }
	
	for (const auto& l_json : a_rootJson)
	{
		std::shared_ptr<Graphics::ConstantBufferUploaderBase> l_constantBufferUploader = nullptr;

		// 定数バッファクラスをデシリアライズ
		Utility::DeserializeInstanceType<TypeAlias::ConstantBufferSharedFactory>(l_json, k_constantBufferUploaderTypeNameJsonKey, l_constantBufferUploader);

		// 作製に成功していれば中身にポインタがしっかり入っているので初期化とデシリアライズを行う
		if (!l_constantBufferUploader) { continue; }
		
		if (l_json.contains(k_constantBufferUploaderJsonKey))
		{
			l_constantBufferUploader->Deserialize(l_json[k_constantBufferUploaderJsonKey]);
		}

		a_frameResource.AddConstantBufferUploader(l_constantBufferUploader);
	}
}

nlohmann::json FWK::Converter::FrameResourceJsonConverter::SerializeConstantBuffer(const Graphics::FrameResource & a_frameResource) const
{
	nlohmann::json l_rootJsonArray = {};
	
	const auto& l_constantBufferUploaderList = a_frameResource.GetREFConstantBufferUploaderList();
	
	// 生成する定数バッファの名前とその定数バッファに必要な情報をSerialize
	for (const auto& l_constantBufferUploader : l_constantBufferUploaderList)
	{
		if (!l_constantBufferUploader) { continue; }

		nlohmann::json l_json = {};

		// インスタンスを復元できるようにシリアライズ
		Utility::UpdateJson(l_json, Utility::SerializeInstanceType(l_constantBufferUploader, k_constantBufferUploaderTypeNameJsonKey));

		l_json[k_constantBufferUploaderJsonKey] = l_constantBufferUploader->Serialize();

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}