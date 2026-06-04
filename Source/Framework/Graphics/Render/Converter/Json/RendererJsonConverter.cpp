#include "RendererJsonConverter.h"

void FWK::Converter::RendererJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const
{
	if (a_rootJson.is_null()) { return; }

	// フレームリソースのデシリアライズ
	// json上ではフレームリソースで管理するデータは共通のため、一つの共通設定だけ持ち、復興時にCount側へ展開する
	if (a_rootJson.contains(k_frameResourceJsonKey))
	{
		DeserializeFrameResourceList(a_rootJson[k_frameResourceJsonKey], a_renderer);
	}
}

nlohmann::json FWK::Converter::RendererJsonConverter::Serialize(const Graphics::Renderer& a_renderer) const
{
	nlohmann::json l_rootJson = {};

	// フレームリソースのシリアライズ
	// 同じ設定を持つFrameResourceを個別にすべて保存せず、Count + Template形式で保存
	l_rootJson[k_frameResourceJsonKey] = SerializeFrameResourceList(a_renderer);

	return l_rootJson;
}

void FWK::Converter::RendererJsonConverter::DeserializeFrameResourceList(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const
{
	if (a_rootJson.is_null()) { return; }
	
	const auto& l_frameResourceCount		= a_rootJson.value(k_frameResourceCountJsonKey,    k_defaultFrameResourceCount);
	const auto& l_frameResourceTemplateJson = a_rootJson.value(k_frameResourceTemplateJsonKey, nlohmann::json());

	FWK_ASSERT_RETURN_IF_FAILED(l_frameResourceTemplateJson.is_null(),			    "フレームリソースのjsonデータが取得できません、デシリアライズ処理に失敗しました。");
	FWK_ASSERT_RETURN_IF_FAILED(!l_frameResourceCount == k_emptyFrameResourceCount, "フレームリソースの作成数が0でとなっており、デシリアライズ処理に失敗しました。");

	for (std::size_t l_i = 0ULL; l_i < l_frameResourceTemplateJson; ++l_i)
	{
		const auto& l_frameResource = std::make_shared<Graphics::FrameResource>();

		// 初期化してからデシリアライズ
		l_frameResource->INIT		();
		l_frameResource->Deserialize(l_frameResourceTemplateJson);

		a_renderer.AddFrameResource(l_frameResource);
	}
}

nlohmann::json FWK::Converter::RendererJsonConverter::SerializeFrameResourceList(const Graphics::Renderer& a_renderer) const
{
	// フレームリソースリストの保存
	auto l_rootJson = nlohmann::json::array();

	const auto& l_frameResourceList = a_renderer.GetREFFrameResourceList();

	l_rootJson[k_frameResourceCountJsonKey] = l_frameResourceList.size();

	if (l_frameResourceList.empty())
	{
		l_rootJson[k_frameResourceTemplateJsonKey] = nlohmann::json();

		return l_rootJson;
	}
	
	const auto& l_frameResource = l_frameResourceList.front();

	if (!l_frameResource)
	{
		l_rootJson[k_frameResourceTemplateJsonKey] = nlohmann::json();

		return l_rootJson;
	}

	// 全フレームリソースが同じパラメータを持つ前提なのでテンプレートとして保存
	l_rootJson[k_frameResourceTemplateJsonKey] = l_frameResource->Serialize();

	return l_rootJson;
}