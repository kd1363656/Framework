#include "SkeletalAnimationComputePass.h"

FWK::Graphics::SkeletalAnimationComputePass::SkeletalAnimationComputePass()
{
	// SkeletalModelの描画より前に、
	// AnimationのBoneMatrix計算を完了させる
	SetupExecutionLayer(Enum::RenderGraphPassExecutionLayer::Animation);
}

FWK::Graphics::SkeletalAnimationComputePass::~SkeletalAnimationComputePass() = default;

void FWK::Graphics::SkeletalAnimationComputePass::Execute(Renderer& a_renderer, RenderGraph& a_renderGraph)
{
	// 定数バッファは現在FrameResourceが所有しているため、
	// Dispatch前に現在のFrameResourceを取得する
	const auto& l_currentFrameResource = a_renderer.GetREFCurrentFrameResource().lock();

	FWK_ASSERT_RETURN_IF(!l_currentFrameResource, "現在FrameResourceを取得できないため、SkeletalAnimationComputePassを実行できません。");

	// Sceneなどから登録されたSkeletalAniamtionPlayer一覧を取得する
	const auto& l_skeletalAnimationPerObjectComputeRequest = a_renderGraph.FindVALComputeRequestPerObject<SkeletalAnimationPerObjectComputeRequest>().lock();

	FWK_ASSERT_RETURN_IF(!l_skeletalAnimationPerObjectComputeRequest, "SkeletalAnimationPerObjectComputeRequestを取得できないため、SkeletalAnimationComputePassを実行できません。");

	// MeshごとのVertex Skinning用定数バッファを書き込むUploaderを
	// 現在FrameResourceから取得する
	const auto& l_vertexSkinningConstantBufferUploader = l_currentFrameResource->FindPTRDynamicBufferUploader<SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader>().lock();

	FWK_ASSERT_RETURN_IF(!l_vertexSkinningConstantBufferUploader, "SkeletalAnimationVertexSkinning用DynamicConstantBufferUploaderを取得できません。");

	const auto& l_meshletBoundsUpdateConstantBufferUploader = l_currentFrameResource->FindPTRDynamicBufferUploader<SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader>().lock();

	FWK_ASSERT_RETURN_IF(!l_meshletBoundsUpdateConstantBufferUploader, "SkeletalAnimationMeshletBoundsUpdate用DynamicConstantBufferUploaderを取得できません。");

	const auto& l_skeletalAnimationPlayerList = l_skeletalAnimationPerObjectComputeRequest->GetREFSkeletalAnimationPlayerList().GetREFArrayElementDataList();
	const auto& l_computeCommandList          = a_renderer.GetREFComputeCommandList                                          ();

	// 最初にすべてのPlayerへVertex Skinningを行う。
	// PlayerごとにVertexSkinningとBounds更新のPSOを
	// 交互に切り替えず、同じPSOの処理をまとめて実行する
	const auto& l_vertexSkinningRootSignature = SetupComputeRenderPipeline(a_renderer, Enum::PipelineStateType::SkeletalAnimationVertexSkinning).lock();

	FWK_ASSERT_RETURN_IF(!l_vertexSkinningRootSignature, "SkeletalAnimationVertexSkinning用RootSignatureを取得できないため、VertexSkinningを実行できません。");

	for (const auto& l_skeletalAnimationPlayerData : l_skeletalAnimationPlayerList)
	{
		const auto& l_skeletalAnimationPlayer = l_skeletalAnimationPlayerData.m_type.lock();

		// BeginFrame後にPlayerが破棄された場合は、
		// 無効なPlayerを処理せず次へ進む
		if (!l_skeletalAnimationPlayer) { continue; }

		const auto& l_skeletalAnimationModelRecord = l_skeletalAnimationPlayer->GetREFSkeletalAnimationModelRecord().lock();

		FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効なため、VertexSkinningを実行できません。");

		auto* l_frameData = l_skeletalAnimationPlayer->FindMutablePTRCurrentFrameData();

		FWK_ASSERT_RETURN_IF(!l_frameData, "現在FrameDataを取得できないため、Vertex Skinningを実行できません。");

		// CPUで計算した現在のPoseのBoneMatrixをGPUへ転送する
		FWK_ASSERT_RETURN_IF(!UploadBoneMatrix(l_computeCommandList, *l_frameData), "BoneMatrixのGPU転送に失敗しました。");

		// Model内の各MeshへLBSVertexSkinningを実行する。
		FWK_ASSERT_RETURN_IF(!DispatchVertexSkinning(l_skeletalAnimationModelRecord->GetREFModelData(),
				                                     *l_vertexSkinningRootSignature,
				                                     l_computeCommandList,
				                                     *l_frameData,
				                                     *l_vertexSkinningConstantBufferUploader),
			                                         "SkeletalAnimationModelのVertexSkinningに失敗しました。");
	}

	// 全PlayerのVertexSkinningがCommandLIstへ記録された後、
	// Bounds更新用Pipelineへ一度だけ切り替える
	const auto& l_meshletBoundsUpdateRootSignature = SetupComputeRenderPipeline(a_renderer, Enum::PipelineStateType::SkeletalAnimationMeshletBoundsUpdate).lock();

	FWK_ASSERT_RETURN_IF(!l_meshletBoundsUpdateRootSignature, "SkeletalAnimationMeshletBoundsUpdate用RootSignatureを取得できないため、Meshlet Boundsを更新できません。");

	for (const auto& l_skeletalAnimationPlayerData : l_skeletalAnimationPlayerList)
	{
		const auto& l_skeletalAnimationPlayer = l_skeletalAnimationPlayerData.m_type.lock();

		// BeginFrame後にPlayerが破棄された場合は、
		// 無効なPlayerを処理せず次へ進む
		if (!l_skeletalAnimationPlayer) { continue; }

		const auto& l_skeletalAnimationModelRecord = l_skeletalAnimationPlayer->GetREFSkeletalAnimationModelRecord().lock();

		FWK_ASSERT_RETURN_IF(!l_skeletalAnimationModelRecord, "SkeletalAnimationModelRecordが無効なため、MeshletBoundsを更新できません。" );

		auto* l_frameData = l_skeletalAnimationPlayer->FindMutablePTRCurrentFrameData();

		FWK_ASSERT_RETURN_IF(!l_frameData, "現在FrameDataを取得できないため、MeshletBoundsを更新できません。");

		// Model内の各Meshについて、
		// スキニング後頂点から現在PoseのMeshletBoundsを計算する。
		FWK_ASSERT_RETURN_IF(!DispatchMeshletBoundsUpdate(l_skeletalAnimationModelRecord->GetREFModelData(),
				                                          *l_meshletBoundsUpdateRootSignature,
				                                          l_computeCommandList,
				                                          *l_frameData,
				                                          *l_meshletBoundsUpdateConstantBufferUploader),
			                                              "SkeletalAnimationModelのMeshletBounds更新に失敗しました。");
	}
}

bool FWK::Graphics::SkeletalAnimationComputePass::UploadBoneMatrix(const ComputeCommandList& a_computeCommandList, SkeletalAnimationPlayer::FrameData& a_frameData) const
{
	const auto& l_globalBoneMatrixList     = a_frameData.m_globalBoneMatrixList;
	      auto& l_boneMatrixBuffer         = a_frameData.m_boneMatrixBuffer;
		  auto& l_boneMatrixBufferUploader = a_frameData.m_boneMatrixBufferUploader;

	FWK_ASSERT_RETURN_VALUE_IF(l_globalBoneMatrixList.empty(), "GlobalBoneMatrixListが空のため、Bone MatrixをGPUへ転送できません。", false);

	// CPU側のMatrix数とGPU Bufferの要素数が異なると、
	// Buffer外へコピーする可能性がある
	FWK_ASSERT_RETURN_VALUE_IF(l_globalBoneMatrixList.size() != l_boneMatrixBuffer.GetVALElementCount(), "GlobalBoneMatrixListとBoneMatrixBufferの要素数が一致しません。", false);

	// CPU側で計算済みのGlobal Bone Matrixを、
	// 現在FrameData専用のUploadBufferへ書き込む
	FWK_ASSERT_RETURN_VALUE_IF(!l_boneMatrixBufferUploader.Write(l_globalBoneMatrixList), "BoneMatrixのUploadBufferへの書き込みに失敗しました。", false);

	const auto& l_boneMatrixBufferResource       = l_boneMatrixBuffer.GetREFBufferGPUResource   ().m_resource;
	const auto& l_boneMatrixUploadBufferResource = l_boneMatrixBufferUploader.GetREFUploadBuffer().GetREFUploadBuffer();

	FWK_ASSERT_RETURN_VALUE_IF(!l_boneMatrixBufferResource,       "BoneMatrixBufferのGPUResourceが無効です。",         false);
	FWK_ASSERT_RETURN_VALUE_IF(!l_boneMatrixUploadBufferResource, "BoneMatrix用UploadBufferのGPUResourceが無効です。", false);

	// Matrix1個のByteSizeとBone数から、
	// 今回コピーするBuffer全体のByteSizeを求める
	const auto& l_boneMatrixBufferSize    = sizeof                                       (TypeAlias::Math::Matrix) * l_globalBoneMatrixList.size();
	const auto  l_beforeCopyResourceState = l_boneMatrixBuffer.GetVALCurrentResourceState();

	// CopyBufferRegionのコピー先として使用するため
	// BoneMatrixBufferをCOPY_DEST状態へ遷移
	a_computeCommandList.TransitionResourceBarrier(l_boneMatrixBufferResource, l_beforeCopyResourceState, D3D12_RESOURCE_STATE_COPY_DEST);

	// UploadBufferからDefaultHeap上のBoneMatrixBufferへコピーする
	// コピー命令とComputeDispatchは同じCommandListへ記録されるため、
	// GPUは記録された順番で処理する
	a_computeCommandList.CopyBufferRegion(k_boneMatrixBufferCopyDestinationOffset,
		                                  k_boneMatrixBufferCopySourceOffset,
		                                  l_boneMatrixBufferSize,
		                                  *l_boneMatrixBufferResource.Get(),
		                                  *l_boneMatrixUploadBufferResource.Get());

	// ComputeShaderからStructuredBufferとして読み取れる状態へ遷移する
	a_computeCommandList.TransitionResourceBarrier(l_boneMatrixBufferResource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	l_boneMatrixBuffer.SetCurrentResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	return true;
}

bool FWK::Graphics::SkeletalAnimationComputePass::DispatchVertexSkinning(const SkeletalAnimationModelRecord::ModelData& a_modelData, const RootSignature& a_rootSignature, const ComputeCommandList& a_computeCommandList, SkeletalAnimationPlayer::FrameData& a_frameData, SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader& a_constantBufferUploader)
{
	const auto& l_modelMeshList           = a_modelData.m_modelMeshList;
	      auto& l_skinnedVertexBufferList = a_frameData.m_skinnedVertexBufferList;

	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(), "ModelMeshListが空のため、VertexSkinningを実行出来ません", false);

	// Player作成時にはModelMesh一個につき、
	// SkinnedVertexBufferを一個作成している
	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.size() != l_skinnedVertexBufferList.size(), "ModelMeshListとSkinnedVertexBufferListの要素数が一致しません。", false);

	const auto& l_boneMatrixBuffer                   = a_frameData.m_boneMatrixBuffer;
	const auto  l_boneMatrixBufferSRVDescriptorIndex = l_boneMatrixBuffer.GetVALSRVDescriptorIndex();

	FWK_ASSERT_RETURN_VALUE_IF(l_boneMatrixBufferSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "BoneMatrixBufferのSRVDescriptorIndexが無効です。", false);

	for (auto l_modelMeshIndex = k_firstModelMeshIndex; l_modelMeshIndex < l_modelMeshList.size(); ++l_modelMeshIndex) 
	{
		const auto& l_modelMesh            = l_modelMeshList[l_modelMeshIndex];
		const auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;
		const auto& l_modelVertexList      = l_modelMesh.m_modelVertexList;

		FWK_ASSERT_RETURN_VALUE_IF(l_modelVertexList.empty(),                                            "ModelVertexListが空のため、Vertex Skinningを実行できません。", false);
		FWK_ASSERT_RETURN_VALUE_IF(l_modelVertexList.size() > std::numeric_limits<std::uint32_t>::max(), "ModelVertexListの要素数がuint32_tの最大値を超えています。",    false);

		const auto& l_sourceVertexBuffer  = l_modelMeshRuntimeData.m_vertexBuffer;
		const auto& l_bonePaletteBuffer   = l_modelMeshRuntimeData.m_bonePaletteBuffer;
			  auto& l_skinnedVertexBuffer = l_skinnedVertexBufferList[l_modelMeshIndex];

		FWK_ASSERT_RETURN_VALUE_IF(l_skinnedVertexBuffer.GetVALElementCount() != l_modelVertexList.size(), "SkinnedVertexBufferとModelVertexListの要素数が一致しません。", false);

		const auto l_sourceVertexBufferSRVDescriptorIndex  = l_sourceVertexBuffer.GetVALSRVDescriptorIndex ();
		const auto l_bonePaletteBufferSRVDescriptorIndex   = l_bonePaletteBuffer.GetVALSRVDescriptorIndex  ();
		const auto l_skinnedVertexBufferUAVDescriptorIndex = l_skinnedVertexBuffer.GetVALUAVDescriptorIndex();

		FWK_ASSERT_RETURN_VALUE_IF(l_sourceVertexBufferSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex,  "SourceVertexBufferのSRVDescriptorIndexが無効です。",  false);
		FWK_ASSERT_RETURN_VALUE_IF(l_bonePaletteBufferSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex,   "BonePaletteBufferのSRVDescriptorIndexが無効です。",   false);
		FWK_ASSERT_RETURN_VALUE_IF(l_skinnedVertexBufferUAVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "SkinnedVertexBufferのUAVDescriptorIndexが無効です。", false);

		const auto& l_skinnedVertexBufferResource = l_skinnedVertexBuffer.GetREFBufferGPUResource().m_resource;

		FWK_ASSERT_RETURN_VALUE_IF(!l_skinnedVertexBufferResource, "SkinnedVertexBufferのGPUResourceが無効です。", false);

		const auto l_currentResourceState = l_skinnedVertexBuffer.GetVALCurrentResourceState();

		// ComputeShaderから書き込めるように
		// SkinnedVertexBufferをUNORDERED_ACCESSへ遷移する
		a_computeCommandList.TransitionResourceBarrier(l_skinnedVertexBufferResource, l_currentResourceState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		l_skinnedVertexBuffer.SetCurrentResourceState (D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		Struct::CBSkeletalAnimationVertexSkinningPerObject l_cbSkeletalAnimationVertexSkinningPerObject = {};

		// HLSLがResourceDescriptorHeapから参照する
		// SRVとUAVのDescriptorIndexを設定する
		l_cbSkeletalAnimationVertexSkinningPerObject.m_sourceVertexBufferSRVDescriptorIndex  = l_sourceVertexBufferSRVDescriptorIndex;
		l_cbSkeletalAnimationVertexSkinningPerObject.m_bonePaletteBufferSRVDescriptorIndex   = l_bonePaletteBufferSRVDescriptorIndex;
		l_cbSkeletalAnimationVertexSkinningPerObject.m_boneMatrixBufferSRVDescriptorIndex    = l_boneMatrixBufferSRVDescriptorIndex;
		l_cbSkeletalAnimationVertexSkinningPerObject.m_skinnedVertexBufferUAVDescriptorIndex = l_skinnedVertexBufferUAVDescriptorIndex;

		// size_tからuint32_tへの返還は、
		// 上で最大値を検査済みなので安全
		l_cbSkeletalAnimationVertexSkinningPerObject.m_vertexCount = static_cast<std::uint32_t>(l_modelVertexList.size());

		const auto& l_gpuVirtualAddress = a_constantBufferUploader.Write(l_cbSkeletalAnimationVertexSkinningPerObject);

		FWK_ASSERT_RETURN_VALUE_IF(l_gpuVirtualAddress == DynamicBufferUploaderBase::k_invalidGPUVirtualAddress, "SkeletalAnimationVertexSkinning用定数バッファの書き込みに失敗しました。", false);

		a_computeCommandList.SetupConstantBufferView(l_gpuVirtualAddress, a_rootSignature, Enum::RootParameterType::CBSkeletalAnimationVertexSkinningPerObject);

		// VertexCountをThread数で割り、
		// 完全に割り切れなかった場合だけ一つThread Groupを追加する
		// VertexCount + ThreadCount - Oneという計算を使わないため、
		// uint32_t最大値付近でも加算Overflowが起きない
		const auto l_completeThreadGroupCount    = l_cbSkeletalAnimationVertexSkinningPerObject.m_vertexCount / k_vertexSkinningThreadCountX;
		const auto l_remainingVertexCount        = l_cbSkeletalAnimationVertexSkinningPerObject.m_vertexCount % k_vertexSkinningThreadCountX;
		const auto l_additionalThreadGroupCount  = l_remainingVertexCount == Constant::k_noRemainder ? Constant::k_noRemainder : k_singleThreadGroupCount;
		const auto l_threadGroupCountX           = l_completeThreadGroupCount + l_additionalThreadGroupCount;

		// 一つのThreadが一つの頂点を処理する
		// Y方向とZ方向には一つのThread Groupだけを使用する
		a_computeCommandList.Dispatch(l_threadGroupCountX, k_singleThreadGroupCount, k_singleThreadGroupCount);

		// UAV書き込み後、Mesh ShaderからSRVとして読み取れる状態へ遷移する。
		// UAVからNON_PIXEL_SHADER_RESOURCEへのTransition Barrierが
		// 書き込み完了と後続読み取りの順序を保証するため、
		// 追加のUAV Barrierは不要
		a_computeCommandList.TransitionResourceBarrier(l_skinnedVertexBufferResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		l_skinnedVertexBuffer.SetCurrentResourceState (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	return true;
}

bool FWK::Graphics::SkeletalAnimationComputePass::DispatchMeshletBoundsUpdate(const SkeletalAnimationModelRecord::ModelData&                                    a_modelData, 
	                                                                          const RootSignature&                                                              a_rootSignature, 
	                                                                          const ComputeCommandList&                                                         a_computeCommandList, 
	                                                                                SkeletalAnimationPlayer::FrameData&                                         a_frameData,
	                                                                                SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader& a_constantBufferUploader) const
{
	const auto& l_modelMeshList           = a_modelData.m_modelMeshList;
	const auto& l_skinnedVertexBufferList = a_frameData.m_skinnedVertexBufferList;
	      auto& l_meshletBoundsBufferList = a_frameData.m_meshletBoundsBufferList;

	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.empty(),                                    "ModelMeshListが空のため、MeshletBoundsを更新できません。",       false);
	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.size() != l_skinnedVertexBufferList.size(), "ModelMeshListとSkinnedVertexBufferListの要素数が一致しません。", false);
	FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshList.size() != l_meshletBoundsBufferList.size(), "ModelMeshListとMeshletBoundsBufferListの要素数が一致しません。", false);

	for (auto l_modelMeshIndex = k_firstModelMeshIndex; l_modelMeshIndex < l_modelMeshList.size(); ++l_modelMeshIndex)
	{
		const auto& l_modelMesh            = l_modelMeshList[l_modelMeshIndex];
		const auto& l_modelMeshRuntimeData = l_modelMesh.m_modelMeshRuntimeData;
		const auto& l_modelMeshletList     = l_modelMesh.m_modelMeshletData.m_meshletList;
		const auto& l_skinnedVertexBuffer  = l_skinnedVertexBufferList[l_modelMeshIndex];
		      auto& l_meshletBoundsBuffer  = l_meshletBoundsBufferList[l_modelMeshIndex];

		FWK_ASSERT_RETURN_VALUE_IF(l_modelMeshletList.empty(),                                                                           "ModelMeshletListが空のため、MeshletBoundsを更新できません。",                                    false);
		FWK_ASSERT_RETURN_VALUE_IF(l_skinnedVertexBuffer.GetVALCurrentResourceState() != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, "SkinnedVertexBufferがMeshlet Bounds Compute Shaderから読み取れるResource Stateではありません。", false);
		FWK_ASSERT_RETURN_VALUE_IF(l_meshletBoundsBuffer.GetVALElementCount() != l_modelMeshletList.size(),                              "MeshletBoundsBufferとModelMeshletListの要素数が一致しません。",                                  false);

		const auto& l_meshletBuffer                             = l_modelMeshRuntimeData.m_meshletBuffer;
		const auto& l_uniqueVertexIndexBuffer                   = l_modelMeshRuntimeData.m_uniqueVertexIndexBuffer;
		const auto& l_primitiveIndexBuffer                      = l_modelMeshRuntimeData.m_primitiveIndexBuffer;
		const auto  l_vertexBufferSRVDescriptorIndex            = l_skinnedVertexBuffer.GetVALSRVDescriptorIndex    ();
		const auto  l_meshletBufferSRVDescriptorIndex           = l_meshletBuffer.GetVALSRVDescriptorIndex          ();
		const auto  l_uniqueVertexIndexBufferSRVDescriptorIndex = l_uniqueVertexIndexBuffer.GetVALSRVDescriptorIndex();
		const auto  l_primitiveIndexBufferSRVDescriptorIndex    = l_primitiveIndexBuffer.GetVALSRVDescriptorIndex   ();
		const auto  l_meshletBoundsBufferUAVDescriptorIndex     = l_meshletBoundsBuffer.GetVALUAVDescriptorIndex    ();

		FWK_ASSERT_RETURN_VALUE_IF(l_vertexBufferSRVDescriptorIndex            == DescriptorHeap::k_invalidDescriptorIndex, "SkinnedVertexBufferのSRVDescriptorIndexが無効です。",     false);
		FWK_ASSERT_RETURN_VALUE_IF(l_meshletBufferSRVDescriptorIndex           == DescriptorHeap::k_invalidDescriptorIndex, "MeshletBufferのSRVDescriptorIndexが無効です。",           false);
		FWK_ASSERT_RETURN_VALUE_IF(l_uniqueVertexIndexBufferSRVDescriptorIndex == DescriptorHeap::k_invalidDescriptorIndex, "UniqueVertexIndexBufferのSRVDescriptorIndexが無効です。", false);
		FWK_ASSERT_RETURN_VALUE_IF(l_primitiveIndexBufferSRVDescriptorIndex    == DescriptorHeap::k_invalidDescriptorIndex, "PrimitiveIndexBufferのSRVDescriptorIndexが無効です。",    false);
		FWK_ASSERT_RETURN_VALUE_IF(l_meshletBoundsBufferUAVDescriptorIndex     == DescriptorHeap::k_invalidDescriptorIndex, "MeshletBoundsBufferのUAVDescriptorIndexが無効です。",     false);

		const auto& l_meshletBoundsBufferResource = l_meshletBoundsBuffer.GetREFBufferGPUResource().m_resource;

		FWK_ASSERT_RETURN_VALUE_IF(!l_meshletBoundsBufferResource, "MeshletBoundsBufferのGPUResourceが無効です。", false);

		const auto l_beforeWriteResourceState = l_meshletBoundsBuffer.GetVALCurrentResourceState();

		// Bounds更新ComputeShaderから書き込めるように
		// MeshletBoundsBufferをUAV状態へ遷移する
		a_computeCommandList.TransitionResourceBarrier(l_meshletBoundsBufferResource, l_beforeWriteResourceState, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		l_meshletBoundsBuffer.SetCurrentResourceState(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		Struct::CBSkeletalAnimationMeshletBoundsUpdatePerObject l_cbSkeletalAnimationMeshletBoundsUpdatePerObject = {};

		l_cbSkeletalAnimationMeshletBoundsUpdatePerObject.m_vertexBufferSRVDescriptorIndex            = l_vertexBufferSRVDescriptorIndex;
		l_cbSkeletalAnimationMeshletBoundsUpdatePerObject.m_meshletBufferSRVDescriptorIndex           = l_meshletBufferSRVDescriptorIndex;
		l_cbSkeletalAnimationMeshletBoundsUpdatePerObject.m_uniqueVertexIndexBufferSRVDescriptorIndex = l_uniqueVertexIndexBufferSRVDescriptorIndex;
		l_cbSkeletalAnimationMeshletBoundsUpdatePerObject.m_primitiveIndexBufferSRVDescriptorIndex    = l_primitiveIndexBufferSRVDescriptorIndex;
		l_cbSkeletalAnimationMeshletBoundsUpdatePerObject.m_meshletBoundsBufferUAVDescriptorIndex     = l_meshletBoundsBufferUAVDescriptorIndex;

		const auto& l_gpuVirtualAddress = a_constantBufferUploader.Write(l_cbSkeletalAnimationMeshletBoundsUpdatePerObject);

		FWK_ASSERT_RETURN_VALUE_IF(l_gpuVirtualAddress == DynamicBufferUploaderBase::k_invalidGPUVirtualAddress, "SkeletalAnimationMeshletBoundsUpdate用定数バッファの書き込みに失敗しました。", false);

		a_computeCommandList.SetupConstantBufferView(l_gpuVirtualAddress, a_rootSignature, Enum::RootParameterType::CBSkeletalAnimationMeshletBoundsUpdatePerObject);

		const auto l_meshletCount = l_meshletBoundsBuffer.GetVALElementCount();

		FWK_ASSERT_RETURN_VALUE_IF(l_meshletCount > D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION, "Meshlet数がCompute ShaderのX方向最大Dispatch Group数を超えています。", false);

		a_computeCommandList.Dispatch(l_meshletCount, k_singleThreadGroupCount, k_singleThreadGroupCount);

		// UAV書き込み後、後続のAmplificationShaderから
		// StructuredBufferとして読み取れる状態へ遷移する
		a_computeCommandList.TransitionResourceBarrier(l_meshletBoundsBufferResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		l_meshletBoundsBuffer.SetCurrentResourceState(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	return true;
}