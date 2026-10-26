#include "GPUParticleIndirectCommandSignature.h"

bool FWK::Graphics::GPUParticleIndirectCommandSignature::Create(const Device& a_device)
{
    FWK_ASSERT_RETURN_VALUE_IF(!CreateCommandSignature(a_device, D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH,      m_dispatchCommandSignature),     "GPUパーティクルのDispatch用CommandSignatureの作成に失敗しました。",     false);
    FWK_ASSERT_RETURN_VALUE_IF(!CreateCommandSignature(a_device, D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH, m_dispatchMeshCommandSignature), "GPUパーティクルのDispatchMesh用CommandSignatureの作成に失敗しました。", false);

    return true;
}

bool FWK::Graphics::GPUParticleIndirectCommandSignature::CreateCommandSignature(const Device& a_device, const D3D12_INDIRECT_ARGUMENT_TYPE a_argumentType, TypeAlias::ComPtr<ID3D12CommandSignature>& a_commandSignature) const
{
    const auto& l_device = a_device.GetREFDevice();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device, "デバイスが作成されておらず、GPUパーティクル用CommandSignatureの作成に失敗しました。", false);

    // 命令1件に含まれる引数は1つだけ(DispatchのGroup数、又はDispatchMeshのGroup数)
    std::array<D3D12_INDIRECT_ARGUMENT_DESC, k_argumentDescCount> l_argumentDescList = {};

    l_argumentDescList[k_argumentIndex].Type = a_argumentType;

    // D3D12_COMMAND_SIGNATURE_DESCについての説明
    // ByteStride       : 命令1件のバイトサイズ(次の命令までの間隔。引数は12byteだが、16byteの区切りに置いている)
    // NumArgumentDescs : 引数の数
    // pArgumentDescs   : 引数の並び
    // NodeMask         : 使用するGPU(GPUが1つなら0)
    D3D12_COMMAND_SIGNATURE_DESC l_commandSignatureDesc = {};

    l_commandSignatureDesc.ByteStride       = Constant::k_gpuParticleIndirectArgumentsStride;
    l_commandSignatureDesc.NumArgumentDescs = static_cast<UINT>(l_argumentDescList.size());
    l_commandSignatureDesc.pArgumentDescs   = l_argumentDescList.data();
    l_commandSignatureDesc.NodeMask         = k_commandSignatureNodeMask;

    // CreateCommandSignature(CommandSignatureの設定、
    //                        引数がRootSignatureへ値を設定する場合はそのRootSignature(今回は設定しないためnullptr)、
    //                        受け取りたいCOMインターフェース型のID、
    //                        作成結果のポインタを書き込むアドレス);
    const auto l_hr = l_device->CreateCommandSignature(&l_commandSignatureDesc, nullptr, IID_PPV_ARGS(a_commandSignature.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "GPUパーティクル用CommandSignatureの作成に失敗しました。", false);

    return true;
}