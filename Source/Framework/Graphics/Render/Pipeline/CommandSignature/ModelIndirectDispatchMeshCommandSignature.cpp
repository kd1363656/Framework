#include "ModelIndirectDispatchMeshCommandSignature.h"

bool FWK::Graphics::ModelIndirectDispatchMeshCommandSignature::Create(const Device& a_device, const RootSignature& a_rootSignature)
{
    const auto& l_device           = a_device.GetREFDevice              ();
    const auto& l_d3dRootSignature = a_rootSignature.GetREFRootSignature();

    FWK_ASSERT_RETURN_VALUE_IF(!l_device,           "デバイスが作成されておらず、CommandSignatureの作成に失敗しました。",       false);
    FWK_ASSERT_RETURN_VALUE_IF(!l_d3dRootSignature, "RootSignatureが作成されておらず、CommandSignatureの作成に失敗しました。", false);

    const auto l_rootParameterIndex = a_rootSignature.FindVALRootParameterIndex(Enum::RootParameterType::CBModelPerObject);

    FWK_ASSERT_RETURN_VALUE_IF(l_rootParameterIndex == Constant::k_invalidRootParameterIndex, "RootSignatureにCBModelPerObjectが無いため、CommandSignatureの作成に失敗しました。", false);

    // 描画コマンド1件に含まれる引数を、並んでいる順に指定する
    // D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW : 定数バッファのGPUアドレス(RootSignatureの指定番号へ設定される)
    // D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH        : DispatchMeshの引数(Group数X、Y、Z)
    std::array<D3D12_INDIRECT_ARGUMENT_DESC, k_argumentDescCount> l_argumentDescList = {};

    l_argumentDescList[k_constantBufferViewArgumentIndex].Type                                  = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
    l_argumentDescList[k_constantBufferViewArgumentIndex].ConstantBufferView.RootParameterIndex = l_rootParameterIndex;
    l_argumentDescList[k_dispatchMeshArgumentIndex].Type                                        = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;

    // D3D12_COMMAND_SIGNATURE_DESCについての説明
    // ByteStride       : 描画コマンド1件のバイトサイズ(次のコマンドまでの間隔)
    // NumArgumentDescs : 引数の数
    // pArgumentDescs   : 引数の並び
    // NodeMask         : 使用するGPU(GPUが1つなら0)
    D3D12_COMMAND_SIGNATURE_DESC l_commandSignatureDesc = {};

    l_commandSignatureDesc.ByteStride       = static_cast<UINT>(sizeof(Struct::ModelGPUDrivenIndirectCommand));
    l_commandSignatureDesc.NumArgumentDescs = static_cast<UINT>(l_argumentDescList.size());
    l_commandSignatureDesc.pArgumentDescs   = l_argumentDescList.data();
    l_commandSignatureDesc.NodeMask         = k_commandSignatureNodeMask;

    // CreateCommandSignature(CommandSignatureの設定、
    //                        引数がRootSignatureへ値を設定する場合はそのRootSignature、
    //                        受け取りたいCOMインターフェース型のID、
    //                        作成結果のポインタを書き込むアドレス);
    const auto l_hr = l_device->CreateCommandSignature(&l_commandSignatureDesc, l_d3dRootSignature.Get(), IID_PPV_ARGS(m_commandSignature.ReleaseAndGetAddressOf()));

    FWK_ASSERT_RETURN_VALUE_IF(FAILED(l_hr), "CommandSignatureの作成に失敗しました。", false);

    m_createdRootSignature = l_d3dRootSignature.Get();

    return true;
}

bool FWK::Graphics::ModelIndirectDispatchMeshCommandSignature::IsCreatedFor(const RootSignature& a_rootSignature) const
{
    if (!m_commandSignature) { return false; }

    const auto& l_rootSignature = a_rootSignature.GetREFRootSignature();

    return m_createdRootSignature == l_rootSignature.Get();
}