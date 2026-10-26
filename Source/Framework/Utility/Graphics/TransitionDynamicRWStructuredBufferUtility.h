#pragma once

namespace FWK::Utility
{
    // DynamicRWStructuredBufferのResourceStateを遷移し、Buffer側が覚えている現在の状態も更新する
    // (既に同じ状態なら、Barrierを記録せずに何もしない)
    inline void TransitionDynamicRWStructuredBuffer(const Graphics::DirectCommandList& a_directCommandList, Graphics::DynamicRWStructuredBuffer& a_buffer, const D3D12_RESOURCE_STATES a_afterState)
    {
        const auto l_beforeState = a_buffer.GetVALCurrentResourceState();

        if (l_beforeState == a_afterState) { return; }

        a_directCommandList.TransitionResourceBarrier(a_buffer.GetREFBufferGPUResource().m_resource, l_beforeState, a_afterState);

        a_buffer.SetCurrentResourceState(a_afterState);
    }
}