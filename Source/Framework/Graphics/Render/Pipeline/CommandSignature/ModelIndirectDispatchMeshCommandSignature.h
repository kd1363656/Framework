#pragma once

namespace FWK::Graphics
{
    // ExecuteIndirect(GPU上のバッファに書かれた描画コマンドをまとめて実行する命令)で使う
    // 「描画コマンド1件がどんな並びのデータか」を表すCommandSignatureを作成するクラス
    //
    // 今回の描画コマンド1件(Struct::ModelGPUDrivenIndirectCommand)の並び :
    // 1. CBModelPerObjectとして使う定数バッファのGPUアドレス
    // 2. DispatchMeshの引数(Amplification Shader Groupの数)
    //
    // 定数バッファのアドレスはRootSignatureの何番目の引数へ設定するかを指定するため、
    // RootSignatureごとに1つ作成する必要がある
    class ModelIndirectDispatchMeshCommandSignature final
    {
    public:

         ModelIndirectDispatchMeshCommandSignature() = default;
        ~ModelIndirectDispatchMeshCommandSignature() = default;

        bool Create(const Device& a_device, const RootSignature& a_rootSignature);

        bool IsCreatedFor(const RootSignature& a_rootSignature) const;

        const auto& GetREFCommandSignature() const { return m_commandSignature; }

    private:

        // 描画コマンド1件に含まれる引数の数と、それぞれの並び順
        static constexpr std::size_t k_argumentDescCount               = 2ULL;
        static constexpr std::size_t k_constantBufferViewArgumentIndex = 0ULL;
        static constexpr std::size_t k_dispatchMeshArgumentIndex       = 1ULL;

        static constexpr UINT k_commandSignatureNodeMask = 0U;

        TypeAlias::ComPtr<ID3D12CommandSignature> m_commandSignature = nullptr;

        // どのRootSignature用に作成したか(作り直しが必要か判定するため)
        const ID3D12RootSignature* m_createdRootSignature = nullptr;
    };
}