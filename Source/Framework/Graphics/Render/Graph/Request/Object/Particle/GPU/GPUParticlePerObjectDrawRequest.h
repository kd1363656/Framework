#pragma once

namespace FWK::Graphics
{
    // GPUパーティクルの描画申請をまとめるクラス
    //
    // 粒子を出したい側(将来のパーティクル用Componentなど)は、毎フレームAddEmitRequestで「発生源の設定と出す数」を渡す
    // GPUParticleSimulatePassが集まった発生源をGPUへ送って粒子を動かし、
    // GPUParticleDrawPassがSetupPerObjectConstantBufferで生きている粒子をまとめて描画する
    class GPUParticlePerObjectDrawRequest final : public DrawRequestPerObjectBase
    {
    public:

         GPUParticlePerObjectDrawRequest()          = default;
        ~GPUParticlePerObjectDrawRequest() override = default;

        void BeginFrame() override;

        // 生きている粒子をExecuteIndirectで描画する(描画用PipelineStateとカメラの定数バッファを設定した後にPassが呼ぶ)
        void SetupPerObjectConstantBuffer(const Renderer& a_renderer, const RootSignature& a_rootSignature, const FrameResource&) override;

        // 今フレームに粒子を出す発生源を追加する(a_emitterのm_emitOffsetはここで決めるため、呼ぶ側は設定しなくてよい)
        bool AddEmitRequest(const Struct::SBGPUParticleEmitter& a_emitter);

        // 今フレームの発生と移動の命令を記録する(GPUParticleSimulatePassが1フレームに1回呼ぶ)
        bool Simulate(Renderer& a_renderer, const float a_deltaTime);

        bool IsCreated() const { return m_gpuParticleSystem.IsCreated(); }

        const auto& GetREFEmitterList() const { return m_emitterList; }

        auto GetVALTotalEmitCount() const { return m_totalEmitCount; }

    private:

        std::vector<Struct::SBGPUParticleEmitter> m_emitterList = {};

        GPUParticleSystem m_gpuParticleSystem = {};

        std::uint32_t m_totalEmitCount = Constant::k_gpuParticleInitialEmitCount;

        FWK_DEFINE_TYPE_INFO(GPUParticlePerObjectDrawRequest, DrawRequestPerObjectBase)
    };
}

FWK_REGISTER_FACTORY_METHOD(FWK::TypeAlias::DrawRequestPerObjectSharedFactory, FWK::Graphics::GPUParticlePerObjectDrawRequest)