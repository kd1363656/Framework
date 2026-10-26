#include "GPUParticlePerObjectDrawRequest.h"

void FWK::Graphics::GPUParticlePerObjectDrawRequest::BeginFrame()
{
    // 発生源は「このフレームに出す物」なので、フレームごとに空にする(粒子そのものはGPU上に残る)
    m_emitterList.clear();

    m_totalEmitCount = Constant::k_gpuParticleInitialEmitCount;

    m_gpuParticleSystem.BeginFrame();
}

void FWK::Graphics::GPUParticlePerObjectDrawRequest::SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource&)
{
    m_gpuParticleSystem.Draw(a_renderer, a_rootSignature);
}

bool FWK::Graphics::GPUParticlePerObjectDrawRequest::AddEmitRequest(const Struct::SBGPUParticleEmitter& a_emitter)
{
    // 出す数が0の発生源は、送っても何も起きないため追加しない
    if (a_emitter.m_emitCount == Constant::k_gpuParticleInitialEmitCount) { return false; }

    FWK_ASSERT_RETURN_VALUE_IF(m_emitterList.size() >= Constant::k_gpuParticleMaxEmitterCount,                                   "発生源の数がGPUパーティクルの最大数に達しているため、発生申請を追加できません。",       false);
    FWK_ASSERT_RETURN_VALUE_IF(a_emitter.m_emitCount > Constant::k_gpuParticleMaxEmitCountPerFrame - m_totalEmitCount,              "今フレームに出す粒子の数がGPUパーティクルの最大数を超えるため、発生申請を追加できません。", false);

    auto& l_emitter = m_emitterList.emplace_back(a_emitter);

    // 今フレームに出す全粒子を1列に並べたときの開始位置(GPU側はこの値を二分探索して、粒子がどの発生源の物かを調べる)
    l_emitter.m_emitOffset = m_totalEmitCount;

    m_totalEmitCount += a_emitter.m_emitCount;

    return true;
}

bool FWK::Graphics::GPUParticlePerObjectDrawRequest::Simulate(Renderer& a_renderer, const float a_deltaTime)
{
    return m_gpuParticleSystem.Simulate(a_renderer, m_emitterList, m_totalEmitCount, a_deltaTime);
}