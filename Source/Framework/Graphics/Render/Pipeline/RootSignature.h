#pragma once

namespace FWK::Graphics
{
	class RootSignature final
	{
	public:

		 RootSignature() = default;
		~RootSignature() = default;

		void SetCreateDesc(const Struct::RootSignatureCreateDesc& a_createDesc);

		void BuildRootParameterIndexMap();
		
		const auto& GetREFCreateDesc() const { return m_createDesc; }

		const auto& GetREFRootSignature() const { return m_rootSignature; }

		UINT FindVALRootParameterIndex(Enum::RootParameterSlot a_rootParameterSlot) const;

	private:

		std::unordered_map<Enum::RootParameterSlot, UINT> m_rootParameterIndexMap = {};

		TypeAlias::ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

		Struct::RootSignatureCreateDesc m_createDesc = {};
	};
}