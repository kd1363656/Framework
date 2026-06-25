#pragma once

namespace FWK::Struct
{
	struct RootParameterRecord final
	{
		// 使わない可能性を考慮してディスクリプタテーブル用のD3D12_DESCRIPTOR_RANGEをまとめたリストはポインタとして保持しておく
		// コピーコンストラクタ禁止を防ぐためにshared_ptrを使用
		std::shared_ptr<std::vector<D3D12_DESCRIPTOR_RANGE>> m_descriptorRangeList = nullptr;

		D3D12_ROOT_PARAMETER m_rootParameter = {};
 	};
}