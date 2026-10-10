#include "DirectCommandList.h"

void FWK::Graphics::DirectCommandList::Reset(const TypeAlias::DirectCommandAllocator& a_directCommandAllocator)
{
    DirectAndComputeCommandListBase::Reset(a_directCommandAllocator);
}

void FWK::Graphics::DirectCommandList::SetupRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex) const
{
    FWK_ASSERT_RETURN_IF(a_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "ディスクリプタヒープインデックスが無効な値のため、レンダーターゲットの設定に失敗しました。");

    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、レンダーターゲットの設定に失敗しました。");

    const auto& l_handle = a_rtvDescriptorPool.FetchVALCPUDescriptorHandle(a_rtvDescriptorIndex);

    // OMステージにレンダーターゲットを設定する関数
    // OMSetRenderTargets(設定するレンダーターゲット数、
    //                    レンダーターディスクリプタ配列の先頭アドレス、
    //                    ディスクリプタが連続は位置かどうか、
    //                  　深度ステンシルビューのアドレス);
    l_directCommandList->OMSetRenderTargets(k_singleSetupRenderTargetNUM,
                                            &l_handle,
                                            FALSE,
                                            nullptr);
}
void FWK::Graphics::DirectCommandList::ClearRenderTarget(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool, const UINT a_rtvDescriptorIndex, const TypeAlias::Math::Color& a_clearColor) const
{
    FWK_ASSERT_RETURN_IF(a_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "デスクリプタヒープインデックスが無効な値のため、レンダーターゲットのクリアに失敗しました。");

    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、レンダーターゲットのクリアに失敗しました。");

    const auto& l_handle = a_rtvDescriptorPool.FetchVALCPUDescriptorHandle(a_rtvDescriptorIndex);

    // 現在のレンダーターゲットを指定色でクリアする関数
    // ClearRenderTargetView(クリア対象のRTVハンドル、
    //                       クリア色RGBA配列、
    //                       部分クリアする矩形数(0の場合は矩形指定なしとみなし全面クリア)、
    //                       矩形配列の先頭アドレス);
    l_directCommandList->ClearRenderTargetView(l_handle,
                                               &a_clearColor.x,
                                               k_allRECTClear,
                                               nullptr);
}

void FWK::Graphics::DirectCommandList::SetupDepthStencil(const TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool, const TypeAlias::DescriptorIndex a_dsvDescriptorIndex) const
{
    FWK_ASSERT_RETURN_IF(a_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "DSVDescriptorIndexが無効のため、DepthStencilの設定に失敗しました。");

    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "DirectCommandListが無効のため、DepthStencilの設定に失敗しました。");

    const auto& l_dsvHandle = a_dsvDescriptorPool.FetchVALCPUDescriptorHandle(a_dsvDescriptorIndex);

    // Color Render Targetを設定せず、
    // DepthStencilだけをOutput Mergerへ設定する
    // ShadowMap PassはDepth値だけを書き込むため、
    // RTVは必要ない
    l_directCommandList->OMSetRenderTargets(k_emptySetupRenderTargetNUM,
                                            nullptr,
                                            FALSE,
                                            &l_dsvHandle);
}

void FWK::Graphics::DirectCommandList::SetupRenderTargetAndDepthStencil(const TypeAlias::RTVDescriptorPool& a_rtvDescriptorPool,
                                                                        const TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
                                                                        const TypeAlias::DescriptorIndex    a_rtvDescriptorIndex,
                                                                        const TypeAlias::DescriptorIndex    a_dsvDescriptorIndex) const
{
    FWK_ASSERT_RETURN_IF(a_rtvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "RTVDescriptorIndexが無効のため、RenderTargetとDepthStencilの設定に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "DSVDescriptorIndexが無効のため、RenderTargetとDepthStencilの設定に失敗しました。");

    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "DirectCommandListが無効のため、RenderTargetとDepthStencilの設定に失敗しました。");

    const auto& l_rtvHandle = a_rtvDescriptorPool.FetchVALCPUDescriptorHandle(a_rtvDescriptorIndex);
    const auto& l_dsvHandle = a_dsvDescriptorPool.FetchVALCPUDescriptorHandle(a_dsvDescriptorIndex);

    // OMステージにレンダーターゲットを設定する関数
    // OMSetRenderTargets(設定するレンダーターゲット数、
    //                    レンダーターゲットディスクリプタ配列の先頭アドレス、
    //                    ディスクリプタ連続配置かどうか、
    //                    深度ステンシルビューのアドレス);
    l_directCommandList->OMSetRenderTargets(k_singleSetupRenderTargetNUM,
                                            &l_rtvHandle,
                                            FALSE,
                                            &l_dsvHandle);
}

void FWK::Graphics::DirectCommandList::ClearDepthStencil(const TypeAlias::DSVDescriptorPool& a_dsvDescriptorPool,
                                                         const FLOAT                         a_depthClearValue,
                                                         const TypeAlias::DescriptorIndex    a_dsvDescriptorIndex,
                                                         const UINT8                         a_stencilClearValue) const
{
    FWK_ASSERT_RETURN_IF(a_dsvDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "DSVDescriptorIndexが無効のため、DepthStencilのClearに失敗しました。");

    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "DirectCommandListが無効のため、DepthStencilのClearに失敗しました。");

    const auto& l_dsvHandle = a_dsvDescriptorPool.FetchVALCPUDescriptorHandle(a_dsvDescriptorIndex);

    // ClearDepthStencilView(クリアするDSV,
    //                       クリア対象フラグ、
    //                       深度クリア値、
    //                       ステンシルクリア値、
    //                       クリア範囲数、
    //                       クリア範囲);
    l_directCommandList->ClearDepthStencilView(l_dsvHandle,
                                               D3D12_CLEAR_FLAG_DEPTH,
                                               a_depthClearValue,
                                               a_stencilClearValue,
                                               k_allRECTClear,
                                               nullptr);
}

void FWK::Graphics::DirectCommandList::SetupRenderArea(const RenderArea& a_renderArea) const
{
    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ビューポートとシザー矩形の設定が出来ませんでした。");

    // Viewportは頂点変換後の座標(NDC -1.0F ~ 1.0F)を、実際の画面上のどこに描くかを決める設定
    // Viewportをコマンドリストへ設定する関数
    // RSSetViewports(設定するViewportの数、
    //                Viewport構造体のアドレス);
    l_directCommandList->RSSetViewports(k_setViewportNUM, &a_renderArea.GetREFViewport());

    // ScissorRectをコマンドリストへ設定する
    // ScissorRectは実際に描画してよいピクセル範囲を制限する四角形
    // RSSetScissorRects(設定するScissorRectの数、
    //                   ScissorRectの先頭アドレス)
    l_directCommandList->RSSetScissorRects(k_setScissorRECTNUM, &a_renderArea.GetREFScissorRECT());
}

void FWK::Graphics::DirectCommandList::SetupRenderPipeline(const std::weak_ptr<GraphicsPipelineStateBase>& a_pipelineState)
{
    DirectAndComputeCommandListBase::SetupPipeline(a_pipelineState);
}

void FWK::Graphics::DirectCommandList::SetupConstantBufferView(const RootSignature& a_rootSignature, const D3D12_GPU_VIRTUAL_ADDRESS& a_gpuVirtualAddress, const Enum::RootParameterType a_rootParameterType) const
{
    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、定数バッファビュー設定に失敗しました。");

    const auto& l_rootParameterIndex = a_rootSignature.FindVALRootParameterIndex(a_rootParameterType);

    FWK_ASSERT_RETURN_IF(l_rootParameterIndex == Converter::RootSignatureJsonConverter::k_invalidRootParameterIndex, "パラメータインデックスが無効なため、定数バッファビュー設定に失敗しました。");

    // RootSignature側でD3D12_ROOT_PARAMETER_TYPE_CBVにした場所へ、
    // UploadBuffer上の定数バッファ位置を直接結びつける
    // SetGraphicsRootConstantBufferView(ルートパラメータ番号、
    //                                   CBVとして参照させるGPU仮想アドレス);
    l_directCommandList->SetGraphicsRootConstantBufferView(l_rootParameterIndex, a_gpuVirtualAddress);
}

void FWK::Graphics::DirectCommandList::SetupPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY a_primitiveTopology) const
{
    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、プリミティブトポロジーの設定に失敗しました。");

    // InputAssemblerに、これから描画する頂点をどの形として扱うかを設定する
    // SpriteScreenはTriangleStripで四角形を描画する
    l_directCommandList->IASetPrimitiveTopology(a_primitiveTopology);
}

void FWK::Graphics::DirectCommandList::SetupVertexBufferView(const D3D12_VERTEX_BUFFER_VIEW& a_vertexBufferView) const
{
    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList,                                                                       "ダイレクトコマンドリストが作成されておらず、VertexBufferViewの設定に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_vertexBufferView.BufferLocation == DynamicBufferUploaderBase::k_invalidGPUVirtualAddress, "VertexBufferViewのGPU仮想アドレスが無効のため、VertexBufferViewの設定に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_vertexBufferView.SizeInBytes    == k_invalidSizeInBytes,                                  "VertexBufferViewのSizeInBytes無効のため、VertexBufferViewの設定に失敗しました。");

    l_directCommandList->IASetVertexBuffers(k_vertexBufferViewStartSlot, k_vertexBufferViewCount, &a_vertexBufferView);
}

void FWK::Graphics::DirectCommandList::DrawIndexedInstanced(const UINT a_indexCount,
                                                            const UINT a_instanceCount,
                                                            const UINT a_startIndexLocation,
                                                            const UINT a_startInstanceLocation,
                                                            const INT  a_baseVertexLocation)
{
    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList,                      "ダイレクトコマンドリストが作成されておらず、DrawIndexedInstancedの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_indexCount    == k_invalidIndexCount,    "Index数が0のため、DrawIndexedInstancedの実行に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_instanceCount == k_invalidInstanceCount, "Instance数が0のため、DrawIndexedInstancedの実行に失敗しました。");

    l_directCommandList->DrawIndexedInstanced(a_indexCount,
                                              a_instanceCount,
                                              a_startIndexLocation,
                                              a_baseVertexLocation,
                                              a_startInstanceLocation);
}

void FWK::Graphics::DirectCommandList::DrawInstanced(const UINT a_vertexCount,
                                                     const UINT a_instanceCount,
                                                     const UINT a_startVertexLocation,
                                                     const UINT a_startInstanceLocation) const
{
    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、DrawInstanceの実行に失敗しました。");

    // 通常のVS/PSパイプラインで描画する
    // SpriteScreenではVertexBufferを使わず、VS側のSV_VertexIDから4頂点を生成する
    l_directCommandList->DrawInstanced(a_vertexCount,
                                       a_instanceCount,
                                       a_startVertexLocation,
                                       a_startInstanceLocation);
}
void FWK::Graphics::DirectCommandList::DispatchMesh(const UINT a_threadCountGroupX, const UINT a_threadCountGroupY, const UINT a_threadCountGroupZ) const
{
    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、DispatchMesh処理に失敗しました。");

    // メッシュシェーダーを実行するための関数
    // DispatchMesh(X方向のグループ数、
    //              Y方向のグループ数、
    //              Z方向のグループ数);
    l_directCommandList->DispatchMesh(a_threadCountGroupX, a_threadCountGroupY, a_threadCountGroupZ);
}

void FWK::Graphics::DirectCommandList::SetupRoot32BitConstants(const RootSignature&          a_rootSignature,
                                                               const void*                   a_rootConstantData,
                                                               const Enum::RootParameterType a_rootParameterType,
                                                               const UINT                    a_rootConstantCount,
                                                               const UINT                    a_destinationOffset) const
{
    FWK_ASSERT_RETURN_IF(!a_rootConstantData,                               "ルート定数のデータが無効のため、ルート定数の設定に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_rootConstantCount == k_invalidRootConstantCount, "ルート定数の個数が0のため、ルート定数の設定に失敗しました。");

    const auto& l_directCommandList = GetREFCommandList();

    FWK_ASSERT_RETURN_IF(!l_directCommandList, "ダイレクトコマンドリストが作成されておらず、ルート定数の設定に失敗しました。");

    // ルートシグネチャの中で、この種類のルートパラメーターが何番目かを探す
    // 例 : ModelStandardのルートシグネチャなら、RCModelDrawItemは1番
    const auto& l_rootParameterIndex = a_rootSignature.FindVALRootParameterIndex(a_rootParameterType);

    FWK_ASSERT_RETURN_IF(l_rootParameterIndex == Converter::RootSignatureJsonConverter::k_invalidRootParameterIndex, "ルートパラメーターの番号が無効なため、ルート定数の設定に失敗しました。");

    const auto& l_rootParameterRecordList = a_rootSignature.GetREFRootParameterRecordList();

    FWK_ASSERT_RETURN_IF(l_rootParameterIndex >= l_rootParameterRecordList.size(), "ルートパラメーターの番号が一覧の範囲外のため、ルート定数の設定に失敗しました。");

    const auto& l_rootParameter = l_rootParameterRecordList[l_rootParameterIndex].m_rootParameter;

    // 指定したルートパラメーターが、本当にルート定数かを確かめる
    // CBVなどへ値を送ると、GPUが値をアドレスとして読んでしまい、描画の崩れやデバイスの消失になる
    FWK_ASSERT_RETURN_IF(l_rootParameter.ParameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS, "指定したルートパラメーターがルート定数ではないため、ルート定数の設定に失敗しました。");
    FWK_ASSERT_RETURN_IF(a_destinationOffset > l_rootParameter.Constants.Num32BitValues,             "書き込み開始位置がルート定数の数を超えているため、ルート定数の設定に失敗しました。");

    // 先に開始位置を検査しているので、Num32BitValues - 開始位置は符号なし整数のアンダーフローを起こさない
    FWK_ASSERT_RETURN_IF(a_rootConstantCount > l_rootParameter.Constants.Num32BitValues - a_destinationOffset, "送る値の数がルート定数の数を超えているため、ルート定数の設定に失敗しました。");

    // SetGraphicsRoot32BitConstants(ルートパラメーターの番号、
    //                               送る32ビットの値の個数、
    //                               送る値の先頭アドレス、
    //                               ルート定数の何番目の値から書くか);
    // 値そのものがコマンドリストに書き込まれるため、UPLOADへ書く必要がない
    l_directCommandList->SetGraphicsRoot32BitConstants(l_rootParameterIndex,
                                                       a_rootConstantCount,
                                                       a_rootConstantData,
                                                       a_destinationOffset);
}

void FWK::Graphics::DirectCommandList::SetupRootSignature(ID3D12GraphicsCommandList6& a_commandList, ID3D12RootSignature& a_rootSignature)
{
    a_commandList.SetGraphicsRootSignature(&a_rootSignature);
}