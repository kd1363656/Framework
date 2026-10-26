#include "GPUParticle.hlsli"

// ExecuteIndirectが読む引数(Group数)をGPU上で作るComputeShader(1Threadだけで実行する)
//
// 生きている粒子の数はGPUのカウンターだけが知っていて、CPUは読めない(読むとGPUの完了を待つことになり遅い)
// そこで「カウンターの値からGroup数を求めてBufferへ書く」処理もGPUに任せ、
// C++側はそのBufferを指定してExecuteIndirectを呼ぶだけにする
//
// Simulateが対象のとき : 読み側の生存数からSimulateのGroup数を書き、書き側の生存数を0に戻す(Simulateがここへ積み直す)
// Drawが対象のとき     : 書き側の生存数から描画(DispatchMesh)のGroup数を書く

[numthreads(k_gpuParticleSingleThreadCountX, k_gpuParticleSingleThreadCountY, k_gpuParticleSingleThreadCountZ)]
void main()
{
    RWStructuredBuffer<uint>                         l_counterBuffer           = ResourceDescriptorHeap[g_counterBufferUAVDescriptorIndex];
    RWStructuredBuffer<GPUParticleIndirectArguments> l_indirectArgumentsBuffer = ResourceDescriptorHeap[g_indirectArgumentsBufferUAVDescriptorIndex];

    // 片方のGroup数だけを書き換えるため、一度読んでから書き戻す
    GPUParticleIndirectArguments l_indirectArguments = l_indirectArgumentsBuffer[k_gpuParticleIndirectArgumentsElementIndex];

    if (g_indirectArgumentsBuildTarget == k_gpuParticleIndirectArgumentsBuildTargetSimulate)
    {
        const uint l_aliveCount = l_counterBuffer[g_aliveReadCounterIndex];

        l_indirectArguments.simulateThreadGroupCount = uint3(CalculateGPUParticleThreadGroupCount(l_aliveCount, k_gpuParticleSimulateThreadCountX),
                                                             k_gpuParticleSingleThreadGroupCountY,
                                                             k_gpuParticleSingleThreadGroupCountZ);

        l_counterBuffer[g_aliveWriteCounterIndex] = k_gpuParticleInitialCounterValue;
    }
    else
    {
        const uint l_aliveCount = l_counterBuffer[g_aliveWriteCounterIndex];

        l_indirectArguments.drawThreadGroupCount = uint3(CalculateGPUParticleThreadGroupCount(l_aliveCount, k_gpuParticleMeshShaderThreadCountX),
                                                         k_gpuParticleSingleThreadGroupCountY,
                                                         k_gpuParticleSingleThreadGroupCountZ);
    }

    l_indirectArgumentsBuffer[k_gpuParticleIndirectArgumentsElementIndex] = l_indirectArguments;
}