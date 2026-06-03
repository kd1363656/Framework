#include "Device.h"

bool FWK::Graphics::Device::Create(const Factory& a_factory)
{
	// ファクトリーが未作成ならGPU列挙ができないのでreturn
	const auto& l_factory = a_factory.GetREFFactory();

	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_factory, "ファクトリーの作成ができておらず、デバイスの作成に失敗しました。", false);

	const auto l_preferredFeatureLevelList = std::to_array<D3D_FEATURE_LEVEL>
	({
			D3D_FEATURE_LEVEL_12_2,
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
	});

	// 現在調査中のGPUアダプターを一時的に受け取る変数
	TypeAlias::ComPtr<IDXGIAdapter4> l_adapter = nullptr;

	// 最終的に使うGPUアダプターの格納先
	TypeAlias::ComPtr<IDXGIAdapter4> l_selectedAdapter = nullptr;

	// 最終的に採用したフィーチャーレベルを保持する変数
	D3D_FEATURE_LEVEL l_selectedFeatureLevel = {};

#if defined(_DEBUG)
	// 最終的に採用したGPU情報
	DXGI_ADAPTER_DESC3 l_selectedDesc = {};
#endif

	bool l_isFound = false;

	// D3D12CreateDevice(どのGPUを使ってデバイスを作るか、
	//					 どのフィーチャーレベルで作りたいか、
	//					 受け取りたいCOMインターフェース型のID、
	//					 作成結果のポインタを書き込むアドレス);
	// 指定した優先条件で、GPU一覧の中からa_index番目のGPUを取り出す関数
	// EnumAdapterByGpuPreference(何番目のGPUアダプターを取得するか、
	//							  どういう種類のGPUを優先して列挙するか、
	//							  受け取りたいCOMインターフェース型のID、
	//							  作成結果のポインタを書き込むアドレス)
	auto l_adapterIndex = k_firstAdapterIndex;

	while (SUCCEEDED(l_factory->EnumAdapterByGpuPreference(l_adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(l_adapter.ReleaseAndGetAddressOf()))))
	{
		// 現在取得したGPUの詳細情報を受け取る構造体
		DXGI_ADAPTER_DESC3 l_desc = {};

		// 念のため、アダプター取得結果がnullptrならこの要素は無視する
		if (!l_adapter) 
		{
			++l_adapterIndex;

			continue; 
		}

		// 現在のGPUアダプターの情報を取得する関数
		// GetDesc3(取得したGPU情報を書き込む構造体のアドレス);

		auto l_getDescResult = l_adapter->GetDesc3(&l_desc);

		if (FAILED(l_getDescResult)) 
		{
			++l_adapterIndex;

			continue; 
		}

		// ソフトウェアアダプターは除外する
		if (l_desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
		{
			++l_adapterIndex;

			continue; 
		}

		// このGPUでどのフィーチャーレベルまで使えるかを高い順に調べる
		for (auto l_level : l_preferredFeatureLevelList)
		{
			// 指定したGPUとフィーチャーレベルでDirectX12デバイスが作成できるかを確認する
			// 第四引数にnullptrを渡しているため実際にデバイスを受け取るのではなく
			// 「この条件でデバイス作成が可能かどうかのテスト」だけを行っている
			auto l_createDeviceResult = D3D12CreateDevice(l_adapter.Get(),
														  l_level,
														  __uuidof(ID3D12Device),	
														  nullptr);
			
			// もし作成テストに失敗したらもう一段階フィーチャーレベルを落として再確認する
			if (FAILED(l_createDeviceResult)) { continue; }

			// 指定のGPUは指定のフィーチャーレベルで使用可能なので採用候補として保存する
			l_selectedAdapter      = l_adapter;
			l_selectedFeatureLevel = l_level;
			l_isFound              = true;

#if defined(_DEBUG)
			l_selectedDesc = l_desc;
#endif
			break;
		}

		// 使用可能なGPUが見つかったので、これ以上探さずループ終了
		if (l_isFound) { break; }

		++l_adapterIndex;
	}

	// 使用可能なGPUが一つも見つからなかった場合は失敗
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(!l_isFound, "対応するGPUが見つかっておらず、デバイスの作成に失敗しました。", false);


#if defined(_DEBUG)
	std::wstring l_outputLog = {};

	l_outputLog += k_deviceDebugLogSeparator;
	l_outputLog += k_selectedGPUNameDebugLogLabel;
	l_outputLog += l_selectedDesc.Description;
	l_outputLog += k_deviceDebugLogSeparator;

	OutputDebugString(l_outputLog.c_str());
#endif

	// 事前チェックで使用可能と分かったGPUとフィーチャーレベルを使って、
	// 実際にDirectX12デバイスを作成する
	auto l_hr = D3D12CreateDevice(l_selectedAdapter.Get(), l_selectedFeatureLevel, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf()));

	// 作成が失敗した場合はエラー
	FWK_ASSERT_RETURN_VALUE_IF_FAILED(FAILED(l_hr), "デバイスの作成に失敗しました。", false);

	// デバイス作成に成功した際のアダプタを格納
	m_adapter = l_selectedAdapter;

	return true;
}