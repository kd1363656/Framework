#pragma once

namespace FWK::Graphics
{
    // GPUパーティクルのExecuteIndirectで使う2つのCommandSignatureを作成するクラス
    //
    // CommandSignatureは「GPU上のBufferに書かれた命令1件がどんな並びのデータか」を表す
    // Dispatch     : Simulate_CSを「生きている粒子の数 ÷ 1Groupのスレッド数」のGroup数で起動する
    // DispatchMesh : ビルボードのメッシュシェーダーを「生きている粒子の数 ÷ 1Groupの粒子数」のGroup数で起動する
    // どちらもRootSignatureへ値を設定する引数を持たないため、RootSignatureを指定せずに作れる
    class GPUParticleIndirectCommandSignature final
    {
    public:

         GPUParticleIndirectCommandSignature() = default;
        ~GPUParticleIndirectCommandSignature() = default;

        bool Create(const Device& a_device);

        const auto& GetREFDispatchCommandSignature    () const { return m_dispatchCommandSignature; }
        const auto& GetREFDispatchMeshCommandSignature() const { return m_dispatchMeshCommandSignature; }

    private:

        bool CreateCommandSignature(const Device& a_device, const D3D12_INDIRECT_ARGUMENT_TYPE a_argumentType, TypeAlias::ComPtr<ID3D12CommandSignature>& a_commandSignature) const;

        static constexpr std::size_t k_argumentDescCount = 1ULL;
        static constexpr std::size_t k_argumentIndex     = 0ULL;

        static constexpr UINT k_commandSignatureNodeMask = 0U;

        TypeAlias::ComPtr<ID3D12CommandSignature> m_dispatchCommandSignature     = nullptr;
        TypeAlias::ComPtr<ID3D12CommandSignature> m_dispatchMeshCommandSignature = nullptr;
    };
}