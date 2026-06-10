#include "RootSignatureJsonConverter.h"

void FWK::Converter::RootSignatureJsonConverter::Deserialize(const nlohmann::json& a_rootJson, Graphics::RootSignature& a_rootSignature) const
{
	if (a_rootJson.is_null()) { return; }

	// タグに対応したルートパラメータアクセス用インデックスを読み込む
	if (Utility::IsArray(a_rootJson, k_rootParameterIndexMapJsonKey))
	{
		DeserializeRootParameterIndexMap(a_rootJson[k_rootParameterIndexMapJsonKey], a_rootSignature);
	}

	// ルートパラメータを読み込む
	if (Utility::IsArray(a_rootJson, k_rootParameterListJsonKey))
	{
		DeserializeRootParameterList(a_rootJson[k_rootParameterListJsonKey], a_rootSignature);
	}

	// StaticSamplerDescを読み込む
	if (Utility::IsArray(a_rootJson, k_staticSamplerDescListJsonKey))
	{
		DeserializeStaticSamplerDescList(a_rootJson[k_staticSamplerDescListJsonKey], a_rootSignature);
	}

	// このルートシグネチャをパイプラインからどう使うかを決定するフラグ
	// どのシェーダーステージからアクセスするか、InputAssemblerを使うかLocalRootSignatureかを決める
	const auto l_flags = a_rootJson.value(k_rootSignatureFlagsJsonKey, D3D12_ROOT_SIGNATURE_FLAG_NONE);

	// どのバージョンのルートシグネチャ仕様でシリアライズするかを決める
	const auto l_version = a_rootJson.value(k_rootSignatureVersionJsonKey, D3D_ROOT_SIGNATURE_VERSION_1);

	a_rootSignature.SetRootSignatureFlags  (l_flags);
	a_rootSignature.SetRootSignatureVersion(l_version);
}
nlohmann::json FWK::Converter::RootSignatureJsonConverter::Serialize(const Graphics::RootSignature& a_rootSignature) const
{
	nlohmann::json l_rootJson = {};

	// ルートパラメータインデックス、ルートパラメータリスト、
	l_rootJson[k_rootParameterIndexMapJsonKey] = SerializeRootParameterIndexMap(a_rootSignature);
	l_rootJson[k_rootParameterListJsonKey]     = SerializeRootParameterList    (a_rootSignature);
	l_rootJson[k_staticSamplerDescListJsonKey] = SerializeStaticSamplerDescList(a_rootSignature);

	l_rootJson[k_rootSignatureFlagsJsonKey]   = a_rootSignature.GetVALRootSignatureFlags  ();
	l_rootJson[k_rootSignatureVersionJsonKey] = a_rootSignature.GetVALRootSignatureVersion();
	
	return l_rootJson;
}

void FWK::Converter::RootSignatureJsonConverter::DeserializeRootParameterIndexMap(const nlohmann::json& a_rootJson, Graphics::RootSignature& a_rootSignature) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }
	
	auto& l_rootParameterIndexMap = a_rootSignature.GetMutableREFRootParameterIndexMap();

	// 要素が既にある可能性を考慮してClear
	l_rootParameterIndexMap.clear();

	for (const auto& l_json : a_rootJson)
	{
		const auto l_rootParameterType = l_json.value(k_rootParameterTypeJsonKey, Enum::RootParameterType::Invalid);
		const auto l_index			   = l_json.value(k_indexJsonKey,			  Constant::k_invalidRootParameterIndex);

		l_rootParameterIndexMap.try_emplace(l_rootParameterType, l_index);
	}
}
void FWK::Converter::RootSignatureJsonConverter::DeserializeRootParameterList(const nlohmann::json& a_rootJson, Graphics::RootSignature& a_rootSignature) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }
	
	const auto& l_jsonArraySize		      = a_rootJson.size								        ();
		  auto& l_rootParameterRecordList = a_rootSignature.GetMutableREFRootParameterRecordList();

	// 要素が既にある可能性を考慮してClear
	l_rootParameterRecordList.clear();

	// まずは配列の容量を決める
	l_rootParameterRecordList.resize(l_jsonArraySize);

	// リサイズをjson配列の大きさで行っているのでif文で配列外かどうかのインデックス確認を行わない
	// json内部で保存されているデータをl_rootParameterListにパースする
	for (auto l_rootParameterIndex = 0ULL; l_rootParameterIndex < l_jsonArraySize; ++l_rootParameterIndex)
	{
		const auto& l_json = a_rootJson[l_rootParameterIndex];
		
		auto& l_rootParameterRecord = l_rootParameterRecordList[l_rootParameterIndex].m_rootParameter;

		// ルートパラメータの種類、シェーダー可視性を格納
		l_rootParameterRecord.ParameterType    = l_json.value(k_parameterTypeJsonKey,    D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
		l_rootParameterRecord.ShaderVisibility = l_json.value(k_shaderVisibilityJsonKey, D3D12_SHADER_VISIBILITY_ALL);

		switch (l_rootParameterRecord.ParameterType)
		{
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			{
				DeserializeDescriptorRangeList(l_json, l_rootParameterRecordList[l_rootParameterIndex]);
			}
			break;
			
			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			{
				// b番号担当のシェーダーレジスタ番号
				l_rootParameterRecord.Constants.ShaderRegister = l_json.value(k_shaderRegisterJsonKey, k_defaultShaderRegister);

				// レジスタ空間
				l_rootParameterRecord.Constants.RegisterSpace = l_json.value(k_registerSpaceJsonKey, k_defaultRegisterSpace);

				// 32bit値の個数
				l_rootParameterRecord.Constants.Num32BitValues = l_json.value(k_num32BitValuesJsonKey, k_defaultRootConstantsNum32BitValues);
			}
			break;

			case D3D12_ROOT_PARAMETER_TYPE_CBV:
			case D3D12_ROOT_PARAMETER_TYPE_SRV:
			case D3D12_ROOT_PARAMETER_TYPE_UAV:
			{
				// b / t/ u 番号
				l_rootParameterRecord.Descriptor.ShaderRegister = l_json.value(k_shaderRegisterJsonKey, k_defaultShaderRegister);

				// レジスタ空間
				l_rootParameterRecord.Descriptor.RegisterSpace = l_json.value(k_registerSpaceJsonKey, k_defaultRegisterSpace);
			}
			break;

			default:
			{
				FWK_ASSERT_RETURN("未対応のRootParameterTypeが指定されており、デシリアライズ処理に失敗しました。");
			}
		}
	}
}
void FWK::Converter::RootSignatureJsonConverter::DeserializeStaticSamplerDescList(const nlohmann::json& a_rootJson, Graphics::RootSignature& a_rootSignature) const
{
	if (a_rootJson.is_null())		   { return; }
	if (!Utility::IsArray(a_rootJson)) { return; }
	
	auto& l_staticSamplerDescList = a_rootSignature.GetMutableREFStaticSamplerDescList();

	const auto& l_jsonArraySize = a_rootJson.size();

	// 要素が既にある可能性を考慮してClear
	l_staticSamplerDescList.clear();

	// StaticSamplerDescをjsonが保持している数分容量を確保する
	l_staticSamplerDescList.resize(l_jsonArraySize);

	// リサイズをjson配列の大きさで行っているのでif文で配列外かどうかのインデックス確認を行わない
	for (auto l_staticSamplerIndex = 0ULL; l_staticSamplerIndex < l_jsonArraySize; ++l_staticSamplerIndex)
	{
		const auto& l_json			    = a_rootJson			 [l_staticSamplerIndex];
			  auto& l_staticSamplerDesc = l_staticSamplerDescList[l_staticSamplerIndex];

		// サンプリング方法
		l_staticSamplerDesc.Filter = l_json.value(k_filterJsonKey, D3D12_FILTER_MIN_MAG_MIP_POINT);
		
		// U方向のアドレスモード
		l_staticSamplerDesc.AddressU = l_json.value(k_addressUJsonKey, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
		
		// V方向のアドレスモード
		l_staticSamplerDesc.AddressV = l_json.value(k_addressVJsonKey, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

		// W方向のアドレスモード
		// 2Dテクスチャでは直接使わないことが多いが、基本形として合わせて設定する
		l_staticSamplerDesc.AddressW = l_json.value(k_addressWJsonKey, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

		// MIPレベル計算時の補正値
		l_staticSamplerDesc.MipLODBias = l_json.value(k_mipLODBiasJsonKey, k_defaultStaticSamplerMIPLODBias);
		
		// 異方性フィルタ使用時の最大サンプル数
		l_staticSamplerDesc.MaxAnisotropy = l_json.value(k_maxAnisotropyJsonKey, k_defaultStaticSamplerMAXAnisotropy);
		
		// 比較サンプラーで使う比較関数
		l_staticSamplerDesc.ComparisonFunc = l_json.value(k_comparisonFUNCJsonKey, D3D12_COMPARISON_FUNC_ALWAYS);

		// アドレスモード使用時の境界色の色
		l_staticSamplerDesc.BorderColor = l_json.value(k_borderColorJsonKey, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
		
		// 参照可能な最小MIPレベル
		l_staticSamplerDesc.MinLOD = l_json.value(k_minLODJsonKey, k_defaultMINLOD);
		
		// 参照可能な最大MIPレベル
		l_staticSamplerDesc.MaxLOD = l_json.value(k_maxLODJsonKey, D3D12_FLOAT32_MAX);
		
		// シェーダー側のSamplerRegister番号
		l_staticSamplerDesc.ShaderRegister = l_json.value(k_shaderRegisterJsonKey, k_defaultShaderRegister);

		// Sampler用のRegisterSpace
		l_staticSamplerDesc.RegisterSpace = l_json.value(k_registerSpaceJsonKey, k_defaultRegisterSpace);
		
		// このSamplerをどのシェーダーステージから見えるようにするか
		l_staticSamplerDesc.ShaderVisibility = l_json.value(k_shaderVisibilityJsonKey, D3D12_SHADER_VISIBILITY_PIXEL);
	}
}

nlohmann::json FWK::Converter::RootSignatureJsonConverter::SerializeRootParameterIndexMap(const Graphics::RootSignature& a_rootSignature) const
{
	nlohmann::json l_rootJsonArray = nlohmann::json::array();
	
	const auto& l_rootParameterIndexMap = a_rootSignature.GetREFRootParameterIndexMap();

	// タグとルートインデックスを対応付けて保存
	for (const auto& [l_key, l_value] : l_rootParameterIndexMap)
	{
		nlohmann::json l_json = {};

		l_json[k_rootParameterTypeJsonKey] = l_key;
		l_json[k_indexJsonKey]			   = l_value;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}
nlohmann::json FWK::Converter::RootSignatureJsonConverter::SerializeRootParameterList(const Graphics::RootSignature& a_rootSignature) const
{
	auto l_jsonArray = nlohmann::json::array();

	const auto& l_rootParameterList = a_rootSignature.GetREFRootParameterRecordList();

	// ルートパラメータの配列を保存
	for (const auto& l_rootParameter : l_rootParameterList)
	{
		nlohmann::json l_json = {};

		l_json[k_parameterTypeJsonKey]    = l_rootParameter.m_rootParameter.ParameterType;
		l_json[k_shaderVisibilityJsonKey] = l_rootParameter.m_rootParameter.ShaderVisibility;

		switch(l_rootParameter.m_rootParameter.ParameterType)
		{
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			{
				l_json[k_descriptorRangeListJsonKey] = SerializeDescriptorRangeList(l_rootParameter);
			}
			break;

			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			{
				l_json[k_shaderRegisterJsonKey] = l_rootParameter.m_rootParameter.Constants.ShaderRegister;
				l_json[k_registerSpaceJsonKey]  = l_rootParameter.m_rootParameter.Constants.RegisterSpace;
				l_json[k_num32BitValuesJsonKey] = l_rootParameter.m_rootParameter.Constants.Num32BitValues;
			}
			break;

			case D3D12_ROOT_PARAMETER_TYPE_CBV:
			case D3D12_ROOT_PARAMETER_TYPE_SRV:
			case D3D12_ROOT_PARAMETER_TYPE_UAV:
			{
				l_json[k_shaderRegisterJsonKey] = l_rootParameter.m_rootParameter.Descriptor.ShaderRegister;
				l_json[k_registerSpaceJsonKey]  = l_rootParameter.m_rootParameter.Descriptor.RegisterSpace;
			}
			break;

			default:
			{
				FWK_ASSERT_RETURN_VALUE("未対応のRootParameterTypeが指定されており、デシリアライズ処理に失敗しました。", {});
			}
		}
		
		l_jsonArray.emplace_back(l_json);
	}

	return l_jsonArray;
}
nlohmann::json FWK::Converter::RootSignatureJsonConverter::SerializeStaticSamplerDescList(const Graphics::RootSignature& a_rootSignature) const
{
	auto l_rootJsonArray = nlohmann::json::array();
	
	const auto& l_staticSamplerDescList = a_rootSignature.GetREFStaticSamplerDescList();

	// スタティックサンプラーの設定リストを保存
	for (const auto& l_staticSamplerDesc : l_staticSamplerDescList)
	{
		nlohmann::json l_json = {};

		// サンプリング方法
		l_json[k_filterJsonKey] = l_staticSamplerDesc.Filter;
		
		// U方向のアドレスモード
		l_json[k_addressUJsonKey] = l_staticSamplerDesc.AddressU;
		
		// V方向のアドレスモード
		l_json[k_addressVJsonKey] = l_staticSamplerDesc.AddressV;

		// W方向のアドレスモード
		// 2Dテクスチャでは直接使わないことが多いが、基本形として合わせて設定する
		l_json[k_addressWJsonKey] = l_staticSamplerDesc.AddressW;

		// MIPレベル計算時の補正値
		l_json[k_mipLODBiasJsonKey] = l_staticSamplerDesc.MipLODBias;
		
		// 異方性フィルタ使用時の最大サンプル数
		l_json[k_maxAnisotropyJsonKey] = l_staticSamplerDesc.MaxAnisotropy;
		
		// 比較サンプラーで使う比較関数
		l_json[k_comparisonFUNCJsonKey] = l_staticSamplerDesc.ComparisonFunc;

		// アドレスモード使用時の境界色の色
		l_json[k_borderColorJsonKey] = l_staticSamplerDesc.BorderColor;
		
		// 参照可能な最小MIPレベル
		l_json[k_minLODJsonKey] = l_staticSamplerDesc.MinLOD;
		
		// 参照可能な最大MIPレベル
		l_json[k_maxLODJsonKey] = l_staticSamplerDesc.MaxLOD;
		
		// シェーダー側のSamplerRegister番号
		l_json[k_shaderRegisterJsonKey] = l_staticSamplerDesc.ShaderRegister;

		// Sampler用のRegisterSpace
		l_json[k_registerSpaceJsonKey] = l_staticSamplerDesc.RegisterSpace;
		
		// このSamplerをどのシェーダーステージから見えるようにするか
		l_json[k_shaderVisibilityJsonKey] = l_staticSamplerDesc.ShaderVisibility;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}

void FWK::Converter::RootSignatureJsonConverter::DeserializeDescriptorRangeList(const nlohmann::json& a_rootJson, Struct::RootParameterRecord& a_rootParameterRecord) const
{
	// jsonに保存されていなかったということはD3D12_DESCRIPTOR_RANGEは使わなかったということなので
	// NumDescriptorRangesは0になる
	if (a_rootJson.is_null())										
	{
		// 明示的にディスクリプタテーブルを使用しないように初期値を格納
		a_rootParameterRecord.m_rootParameter.DescriptorTable.NumDescriptorRanges = k_invalidNUMDescriptorRange;
		a_rootParameterRecord.m_rootParameter.DescriptorTable.pDescriptorRanges   = nullptr;
		return; 
	}

	if (!Utility::IsArray(a_rootJson, k_descriptorRangeListJsonKey)) 
	{
		// 明示的にディスクリプタテーブルを使用しないように初期値を格納
		a_rootParameterRecord.m_rootParameter.DescriptorTable.NumDescriptorRanges = k_invalidNUMDescriptorRange;
		a_rootParameterRecord.m_rootParameter.DescriptorTable.pDescriptorRanges   = nullptr;
		return; 
	}

	auto& l_descriptorRangeList = a_rootParameterRecord.m_descriptorRangeList;

	// もしインスタンス化されていなければする
	if (!l_descriptorRangeList)
	{
		l_descriptorRangeList = std::make_shared<std::vector<D3D12_DESCRIPTOR_RANGE>>();
	}

	const auto& l_jsonArray = a_rootJson[k_descriptorRangeListJsonKey];

	const auto& l_jsonArraySize = l_jsonArray.size();

	// 要素が既にある可能性を考慮してClear
	l_descriptorRangeList->clear();

	// ディスクリプタレンジをl_jsonArrayが持っているデータの個数分要素を確保
	l_descriptorRangeList->resize(l_jsonArraySize);

	// リサイズをjson配列の大きさで行っているのでif文で配列外かどうかのインデックス確認を行わない
	for (auto l_i = 0ULL; l_i < l_jsonArraySize; ++l_i)
	{
		auto& l_descriptorRange = (*l_descriptorRangeList)[l_i];

		// このレンジに入るディスクリプタの種類
		l_descriptorRange.RangeType = l_jsonArray[l_i].value(k_rangeTypeJsonKey, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);

		// このレンジに含めるディスクリプタ数
		l_descriptorRange.NumDescriptors = l_jsonArray[l_i].value(k_numDescriptorsJsonKey, k_defaultDescriptorRangeNUMDescriptors);

		// シェーダーレジスタの開始番号
		l_descriptorRange.BaseShaderRegister = l_jsonArray[l_i].value(k_baseShaderRegisterJsonKey, k_defaultBaseShaderRegister);

		// レジスタ空間
		l_descriptorRange.RegisterSpace = l_jsonArray[l_i].value(k_registerSpaceJsonKey, k_defaultRegisterSpace);

		// ディスクリプタテーブル先頭からのオフセット
		l_descriptorRange.OffsetInDescriptorsFromTableStart = l_jsonArray[l_i].value(k_offsetInDescriptorsFromTableStartJsonKey, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
	}

	// ディスクリプタテーブルで使用するディスクリプタレンジの数
	a_rootParameterRecord.m_rootParameter.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(l_descriptorRangeList->size());

	// ディスクリプタテーブルで参照するディスクリプタレンジのポインタ
	a_rootParameterRecord.m_rootParameter.DescriptorTable.pDescriptorRanges = l_descriptorRangeList->data();
}

nlohmann::json FWK::Converter::RootSignatureJsonConverter::SerializeDescriptorRangeList(const Struct::RootParameterRecord& a_rootParameterRecord) const
{
	// ディスクリプタレンジリストが存在しなければreturn
	if (!a_rootParameterRecord.m_descriptorRangeList) { return {}; }

	nlohmann::json l_rootJsonArray = nlohmann::json::array();
	
	// ディスクリプタレンジの内容をjsonに保存
	for (const auto& l_descriptorRange : *a_rootParameterRecord.m_descriptorRangeList)
	{
		nlohmann::json l_json = {};

		l_json[k_rangeTypeJsonKey]                         = l_descriptorRange.RangeType;
		l_json[k_numDescriptorsJsonKey]                    = l_descriptorRange.NumDescriptors;
		l_json[k_baseShaderRegisterJsonKey]                = l_descriptorRange.BaseShaderRegister;
		l_json[k_registerSpaceJsonKey]                     = l_descriptorRange.RegisterSpace;
		l_json[k_offsetInDescriptorsFromTableStartJsonKey] = l_descriptorRange.OffsetInDescriptorsFromTableStart;

		l_rootJsonArray.emplace_back(l_json);
	}

	return l_rootJsonArray;
}