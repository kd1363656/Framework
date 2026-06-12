#include "RendererJsonConverter.h"

void FWK::Converter::RendererJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const
{
	if (a_rootJson.is_null()) { return; }

	// フレームリソースのデシリアライズ
	// json上ではフレームリソースで管理するデータは共通のため、一つの共通設定だけ持ち、復元時にCount側へ展開する
	if (const auto& l_json = a_rootJson.value(k_frameResourceListJsonKey, nlohmann::json{}); 
		!l_json.is_null())
	{
		DeserializeFrameResourceList(l_json, a_renderer);
	}

	// スワップチェインのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_swapChainJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeSwapChain(l_json, a_renderer);
	}

	// ルートシグネチャのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_rootSignatureMapJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeRootSignatureMap(l_json, a_renderer);
	}

	// パイプラインステートのデシリアライズ
	if (const auto& l_json = a_rootJson.value(k_pipelineStateMapJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializePipelineStateMap(l_json, a_renderer);
	}

	if (const auto& l_json = a_rootJson.value(k_renderGraphJsonKey, nlohmann::json{});
		!l_json.is_null())
	{
		DeserializeRenderGraph(l_json, a_renderer);
	}
}

nlohmann::json FWK::Converter::RendererJsonConverter::Serialize(const Graphics::Renderer& a_renderer) const
{
	nlohmann::json l_rootJson = {};

	// フレームリソースのシリアライズ
	// 同じ設定を持つFrameResourceを個別にすべて保存せず、Count + Template形式で保存
	l_rootJson[k_frameResourceListJsonKey] = SerializeFrameResourceList(a_renderer);

	// スワップチェインのシリアライズ
	l_rootJson[k_swapChainJsonKey] = SerializeSwapChain(a_renderer);

	// ルートシグネチャマップのシリアライズ
	l_rootJson[k_rootSignatureMapJsonKey] = SerializeRootSignatureMap(a_renderer);

	// パイプラインステートのシリアライズ
	l_rootJson[k_pipelineStateMapJsonKey] = SerializePipelineStateMap(a_renderer);

	// RenderGraphのシリアライズ
	l_rootJson[k_renderGraphJsonKey] = SerializeRenderGraph(a_renderer);

	return l_rootJson;
}

void FWK::Converter::RendererJsonConverter::DeserializeFrameResourceList(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const
{
	if (a_rootJson.is_null()) { return; }
	
	const auto& l_frameResourceCount		= a_rootJson.value(k_frameResourceCountJsonKey,    k_defaultFrameResourceListCount);
	const auto& l_frameResourceTemplateJson = a_rootJson.value(k_frameResourceTemplateJsonKey, nlohmann::json());

	FWK_ASSERT_RETURN_IF_FAILED(l_frameResourceCount == k_emptyFrameResourceListCount, "フレームリソースの作成数が0となっており、デシリアライズ処理に失敗しました。");

	for (std::size_t l_i = 0ULL; l_i < l_frameResourceCount; ++l_i)
	{
		const auto& l_frameResource = std::make_shared<Graphics::FrameResource>();

		// 初期化してからデシリアライズ
		l_frameResource->INIT();

		if (!l_frameResourceTemplateJson.is_null())
		{
			l_frameResource->Deserialize(l_frameResourceTemplateJson);
		}

		a_renderer.AddFrameResource(l_frameResource);
	}
}
void FWK::Converter::RendererJsonConverter::DeserializeSwapChain(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_swapChain = a_renderer.GetMutableREFSwapChain();

	l_swapChain.Deserialize(a_rootJson);
}
void FWK::Converter::RendererJsonConverter::DeserializeRootSignatureMap(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		const auto l_rootSignatureType = l_json.value(k_rootSignatureTypeJsonKey, Enum::RootSignatureType::Invalid);

		if (l_rootSignatureType == Enum::RootSignatureType::Invalid ||
			!l_json.contains(k_rootSignatureJsonKey))
		{
			continue; 
		}
		
		// ルートシグネチャのポインタを作成してレンダラー側に追加
		const auto& l_rootSignature = std::make_shared<Graphics::RootSignature>();

		l_rootSignature->Deserialize(l_json[k_rootSignatureJsonKey]);
		
		a_renderer.AddRootSignature(l_rootSignature, l_rootSignatureType);
	}
}
void FWK::Converter::RendererJsonConverter::DeserializePipelineStateMap(const nlohmann::json & a_rootJson, Graphics::Renderer & a_renderer) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }

	for (const auto& l_json : a_rootJson)
	{
		const auto l_pipelineStateType = l_json.value(k_pipelineStateTypeJsonKey, Enum::PipelineStateType::Invalid);

		if (l_pipelineStateType == Enum::PipelineStateType::Invalid ||
			!l_json.contains(k_pipelineStateJsonKey))
		{
			continue; 
		}
		
		// パイプラインステートを作成してレンダラー側に追加
		const auto& l_pipelineState = std::make_shared<Graphics::PipelineState>();

		l_pipelineState->Deserialize(l_json[k_pipelineStateJsonKey]);
		
		a_renderer.AddPipelineState(l_pipelineState, l_pipelineStateType);
	}
}
void FWK::Converter::RendererJsonConverter::DeserializeRenderGraph(const nlohmann::json& a_rootJson, Graphics::Renderer& a_renderer) const
{
	if (a_rootJson.is_null()) { return; }

	auto& l_renderGraph = a_renderer.GetMutableREFRenderGraph();

	l_renderGraph.Deserialize(a_rootJson);
}

nlohmann::json FWK::Converter::RendererJsonConverter::SerializeFrameResourceList(const Graphics::Renderer& a_renderer) const
{
	// フレームリソースリストの保存
	nlohmann::json l_rootJson = {};

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
nlohmann::json FWK::Converter::RendererJsonConverter::SerializeSwapChain(const Graphics::Renderer& a_renderer) const
{
	const auto& l_swapChain = a_renderer.GetREFSwapChain();

	return l_swapChain.Serialize();
}
nlohmann::json FWK::Converter::RendererJsonConverter::SerializeRootSignatureMap(const Graphics::Renderer& a_renderer) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_rootSignatureMap = a_renderer.GetREFRootSignatureMap();

	for (const auto& [l_type, l_rootSignature] : l_rootSignatureMap)
	{
		if (!l_rootSignature) { continue; }

		nlohmann::json l_json = {};

		l_json[k_rootSignatureTypeJsonKey] = l_type;
		l_json[k_rootSignatureJsonKey]     = l_rootSignature->Serialize();

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}
nlohmann::json FWK::Converter::RendererJsonConverter::SerializePipelineStateMap(const Graphics::Renderer& a_renderer) const
{
	auto l_rootJsonArray = nlohmann::json::array();

	const auto& l_pipelineStateMap = a_renderer.GetREFPipelineStateMap();

	for (const auto& [l_type, l_pipelineState] : l_pipelineStateMap)
	{
		if (!l_pipelineState) { continue; }

		nlohmann::json l_json = {};

		l_json[k_pipelineStateTypeJsonKey] = l_type;
		l_json[k_pipelineStateJsonKey]     = l_pipelineState->Serialize();

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}
nlohmann::json FWK::Converter::RendererJsonConverter::SerializeRenderGraph(const Graphics::Renderer& a_renderer) const
{
	const auto& l_renderGraph = a_renderer.GetREFRenderGraph();

	return l_renderGraph.Serialize();
}