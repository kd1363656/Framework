#pragma once

//===============================================================================
// Utilityクラス
//===============================================================================
#include "Utility/Singleton/SingletonUtility.h"
#include "Definition/Enum/TypeTrait/TypeTraitPTREnum.h"
#include "Definition/Type/Trait/PTR/TypeTraitPTR.h"
#include "Definition/Concept/IsSmartPTR/IsSmartPTRConcept.h"
#include "Definition/Macros/Assert/AssertReturnMacros.h"
#include "Utility/Array/SmartPointerVectorArrayUtility.h"

//===============================================================================
// 型情報
//===============================================================================
#include "Definition/Type/Alias/StaticTypeIDTypeAlias.h"
#include "TypeINFO/StaticTypeID/StaticTypeIDGenerator.h"
#include "Definition/Struct/String/StringHashStruct.h"
#include "TypeINFO/Registry/TypeINFORegistry.h"
#include "Definition/Macros/TypeINFO/TypeINFORegistryMacros.h"

//===============================================================================
// ウィンドウクラス
//===============================================================================
#include "Definition/Macros/Json/JsonSerializeEnumMacros.h"
#include "Definition/Enum/Window/WindowStyleEnum.h"
#include "Window/Converter/Json/WindowJsonConverter.h"
#include "Definition/Constant/Utility/String/StringUtilityConstant.h"
#include "Utility/String/StringUtility.h"
#include "Utility/File/CanLoadFileUtility.h"
#include "Definition/Constant/Utility/File/Json/JsonLoadAndSaveFileUtilityConstant.h"
#include "Utility/File/Json/JsonLoadAndSaveFileUtility.h"
#include "Window/Window.h"

//===============================================================================
// グラフィックス(エディターに必要な定義だけここで済ませる)
//===============================================================================
#include "Definition/Type/Alias/ComPTRTypeAlias.h"
#include "Graphics/Hardware/Factory/Factory.h"
#include "Graphics/Hardware/Device/Device.h"
#include "Definition/Type/Alias/DescriptorIndexTypeAlias.h"
#include "Graphics/Resource/Descriptor/Heap/DescriptorHeap.h"
#include "Graphics/Resource/Descriptor/Heap/Allocator/Converter/Json/DescriptorHeapIndexAllocatorJsonConverter.h"
#include "Graphics/Resource/Descriptor/Heap/Allocator/DescriptorHeapIndexAllocator.h"
#include "Graphics/Resource/Descriptor/Converter/Json/DescriptorPoolJsonConverter.h"
#include "Graphics/Resource/Descriptor/DescriptorPool.h"
#include "Definition/Type/Alias/DescriptorPoolTypeAlias.h"

//===============================================================================
// エディター
//===============================================================================
#include "Definition/Type/Alias/MathTypeAlias.h"
#include "Editor/Style/EditorStyle.h"
#include "Editor/Window/Log/LogEditorWindow.h"
#include "Editor/Window/EditorWindowBase.h"
#include "Editor/Converter/Json/EditorManagerJsonConverter.h"
#include "Definition/Concept/IsDerivedBase/IsDerivedBaseConcept.h"
#include "Definition/Concept/IsDerivedBase/Editor/IsDerivedEditorWindowBaseConcept.h"
#include "Editor/Window/Viewport/Toolbar/ViewportToolbar.h"
#include "Editor/MainMenu/EditorMainMenuBase.h"
#include "Definition/Concept/IsDerivedBase/Editor/IsDerivedEditorMainMenuBaseConcept.h"
#include "Editor/MainMenu/Converter/Json/MainMenubarEditorJsonConverter.h"
#include "Editor/MainMenu/MainMenubarEditor.h"
#include "Editor/EditorManager.h"

//===============================================================================
// 凡庸ファクトリー
//===============================================================================
#include "Factory/GenericFactory.h"
#include "Definition/Macros/Factory/FactoryMacros.h"

//===============================================================================
// エディター(ファクトリーを介さないといけないもの)
//===============================================================================
#include "Definition/Type/Alias/Factory/Shared/EditorWindowSharedFactory.h"
#include "Definition/Constant/Utility/Json/JsonUtilityConstant.h"
#include "Utility/UUID/UUIDUtility.h"
#include "Utility/Json/JsonUtility.h"
#include "Editor/Window/Viewport/ViewportEditorWindow.h"
#include "Definition/Type/Alias/Factory/Unique/EditorMainMenuUniqueFactory.h"
#include "Editor/MainMenu/File/FileMainMenuEditor.h"

//===============================================================================
// バイナリーファイル化基底クラス
//===============================================================================
#include "Converter/Binary/BinaryConverterBase.h"

//===============================================================================
// FPS管理クラス
//===============================================================================
#include "FPS/Converter/Json/FPSControllerJsonConverter.h"
#include "FPS/FPSController.h"

//===============================================================================
// 描画管理クラス
//===============================================================================

// シェーダーコンパイラクラス
#include "Graphics/Resource/Shader/Compiler/ShaderCompiler.h"
#include "Graphics/Resource/Shader/Converter/Json/ShaderJsonConverter.h"
#include "Graphics/Resource/Shader/Shader.h"

// ルートシグネチャ
#include "Definition/Enum/Graphics/RootSignatureEnum.h"
#include "Definition/Enum/Graphics/RootParameterEnum.h"
#include "Definition/Macros/Json/RootSignatureJsonConverterMacros.h"
#include "Graphics/Render/Pipeline/Converter/Json/RootSignatureJsonConverter.h"
#include "Graphics/Render/Pipeline/RootSignature.h"

// パイプラインステート
#include "Definition/Enum/Graphics/PipelineStateBaseEnum.h"
#include "Definition/Macros/Json/PipelineStateJsonConverterMacros.h"
#include "Graphics/Render/Pipeline/Converter/Json/PipelineStateBaseJsonConverter.h"
#include "Graphics/Render/Pipeline/PipelineStateBase.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedPipelineStateBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/PipelineStateSharedFactory.h"
#include "Definition/Macros/Json/StandardShaderPipelineStateJsonConverterMacros.h"
#include "Utility/Graphics/DeserializeOptionalShaderUtility.h"
#include "Graphics/Render/Pipeline/Graphics/Converter/Json/GraphicsPipelineStateBaseJsonConverter.h"
#include "Graphics/Render/Pipeline/Graphics/GraphicsPipelineStateBase.h"
#include "Graphics/Render/Pipeline/Graphics/Standard/Converter/Json/StandardPipelineStateJsonConverter.h"
#include "Graphics/Render/Pipeline/Graphics/Standard/StandardPipelineState.h"
#include "Graphics/Render/Pipeline/Graphics/Mesh/Converter/Json/MeshShaderPipelineStateJsonConverter.h"
#include "Graphics/Render/Pipeline/Graphics/Mesh/MeshShaderPipelineState.h"
#include "Graphics/Render/Pipeline/Compute/Converter/Json/ComputePipelineStateJsonConverter.h"
#include "Graphics/Render/Pipeline/Compute/ComputePipelineState.h"

// アップロードバッファークラス
#include "Graphics/Resource/Buffer/UploadBuffer.h"

// コマンドキュー、リスト、アロケータの基底クラスとフェンスクラス
#include "Definition/Constant/Graphics/CommandAllocatorConstant.h"
#include "Graphics/Command/Queue/Fence/Fence.h"
#include "Graphics/Command/Allocator/CommandAllocator.h"
#include "Definition/Type/Alias/CommandAllocatorTypeAlias.h"
#include "Graphics/Command/List/CommandListBase.h"
#include "Graphics/Command/List/DirectAndComputeCommandListBase.h"
#include "Graphics/Command/Queue/CommandQueue.h"
#include "Definition/Type/Alias/CommandQueueTypeAlias.h"

//リソースリリースクラス
#include "Definition/Struct/Graphics/GPUResourceStruct.h"
#include "Graphics/Resource/ReleaseContext/ResourceReleaseContext.h"

// レコードクラス
#include "Definition/Type/Alias/StorageIDTypeAlias.h"
#include "Definition/Constant/Graphics/AssetRecordBaseCosntant.h"
#include "Graphics/Resource/Record/AssetRecordBase.h"

// ストレージIDアロケータークラス
#include "Definition/Constant/Utility/Storage/StorageIDAllocatorUtilityConstant.h"
#include "Utility/Storage/Converter/Json/StorageIDAllocatorJsonConverter.h"
#include "Utility/Storage/StorageIDAllocator.h"

// スワップチェイン
#include "Definition/Constant/Graphics/SwapChainJsonConverterConstasnt.h"
#include "Graphics/Render/SwapChain/Converter/Json/SwapChainJsonConverter.h"
#include "Graphics/Render/SwapChain/SwapChain.h"

// レンダーエリア
#include "Definition/Constant/Graphics/RendereAreaConstant.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBSpritePassStruct.h"
#include "Graphics/Render/Rasterizer/RenderArea.h"

// ダイレクトコマンドリスト
#include "Graphics/Command/List/Direct/DirectCommandList.h"

// Storageクラス
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedAssetRecordBaseConcept.h"
#include "Graphics/Resource/Storage/Converter/Json/AssetStorageJsonConverter.h"
#include "Graphics/Resource/Storage/AssetStorage.h"

// メモリアロケータクラス
#include "Graphics/Resource/Memory/GPUMemoryAllocator.h"

// ストラクチャードバッファー
#include "Graphics/Resource/Buffer/StructuredBufferBase.h"
#include "Graphics/Resource/Buffer/Static/Structured/StaticStructuredBuffer.h"

// テクスチャ
#include "Definition/Enum/Graphics/TextureLoadColorSpaceEnum.h"
#include "Graphics/Resource/Texture/Loader/TextureLoader.h"
#include "Definition/Constant/Graphics/TextureBinaryConverterConstant.h"
#include "Graphics/Resource/Texture/Converter/Binary/TextureBinaryConverter.h"
#include "Graphics/Resource/Texture/Record/TextureRecord.h"
#include "Graphics/Resource/Texture/Builder/TextureBatchUploadRecordBuilder.h"
#include "Graphics/Resource/Texture/Converter/Json/TextureSystemJsonConverter.h"
#include "Definition/Enum/Graphics/DefaultTextureEnum.h"
#include "Definition/Constant/Utility/String/WStringUtilityConstant.h"
#include "Utility/String/WStringUtility.h"
#include "Graphics/Resource/Texture/Default/Converter/Json/DefaultTextureJsonConverter.h"
#include "Graphics/Resource/Texture/Default/DefaultTexture.h"
#include "Utility/Graphics/IsSameTextureSizeUtility.h"
#include "Utility/Graphics/IsValidTextureUtility.h"
#include "Graphics/Resource/Texture/RenderTarget/RenderTargetTexture.h"
#include "Definition/Struct/Graphics/DepthStencilTextureStruct.h"
#include "Graphics/Resource/Texture/DepthStencil/DepthStencilTexture.h"
#include "Definition/Struct/String/WStringHashStruct.h"
#include "Graphics/Resource/Texture/TextureSystem.h"
#include "Graphics/Resource/Texture/Texture.h"

// モデル(Static)
#include "Definition/Struct/Graphics/ModelCommonStruct.h"
#include "Definition/Constant/Graphics/FBXModelLoaderBaseConstant.h"
#include "Graphics/Resource/Model/FBXLoader/FBXModelLoaderBase.h"
#include "Graphics/Resource/Model/Static/Record/StaticModelRecord.h"
#include "Definition/Constant/Graphics/TriangleVertexCountConstant.h"
#include "Definition/Constant/Graphics/ModelBinaryConverterConstant.h"
#include "Graphics/Resource/Model/Converter/Binary/ModelBinaryConverterBase.h"
#include "Definition/Constant/Graphics/UFBXConvertUtilityConstant.h"
#include "Utility/Graphics/UFBXConvertUtility.h"
#include "Graphics/Resource/Model/Static/Converter/Binary/StaticModelBinaryConverter.h"
#include "Graphics/Resource/Model/Static/FBXLoader/StaticModelFBXLoader.h"
#include "Graphics/Resource/Model/Mesh/ModelMeshOptimizer.h"
#include "Definition/Constant/Graphics/ModelMeshletBuilderConstant.h"
#include "Graphics/Resource/Model/Meshlet/ModelMeshletBuilder.h"
#include "Graphics/Resource/Model/Builder/ModelBatchUploadRecordBuilder.h"
#include "Graphics/Resource/Model/Static/Builder/StaticModelBatchUploadRecordBuilder.h"
#include "Definition/Struct/Graphics/StaticModelSystemStruct.h"
#include "Graphics/Resource/Model/Static/Converter/Json/StaticModelSystemJsonConverter.h"
#include "Graphics/Resource/Model/Material/ModelMaterialRuntimeTextureBuilder.h"
#include "Graphics/Resource/Model/Static/StaticModelSystem.h"
#include "Graphics/Resource/Model/Static/StaticModel.h"

// モデル(SkeletalAnimation)
#include "Graphics/Resource/Buffer/Dynamic/Structured/DynamicRWStructuredBuffer.h"
#include "Graphics/Resource/Model/Skeletal/Record/SkeletalAnimationModelRecord.h"
#include "Graphics/Resource/Model/Skeletal/Converter/Binary/SkeletalAnimationModelBinaryConverter.h"
#include "Definition/Constant/Graphics/SkeletalAnimationModelFBXMotionExtractorConstant.h"
#include "Graphics/Resource/Model/Skeletal/FBXLoader/Extractor/Skeleton/SkeletalAnimationModelFBXSkeletonExtractor.h"
#include "Graphics/Resource/Model/Skeletal/FBXLoader/Extractor/Motion/SkeletalAnimationModelFBXMotionExtractor.h"
#include "Graphics/Resource/Model/Skeletal/FBXLoader/SkeletalAnimationModelFBXLoader.h"
#include "Graphics/Resource/Model/Skeletal/Builder/SkeletalAnimationModelBatchUploadRecordBuilder.h"
#include "Definition/Struct/Graphics/SkeletalAnimationModelSystemStruct.h"
#include "Graphics/Resource/Model/Skeletal/Converter/Json/SkeletalAnimationModelSystemJsonConverter.h"
#include "Graphics/Resource/Model/Skeletal/SkeletalAnimationModelSystem.h"
#include "Graphics/Resource/Model/Skeletal/SkeletalAnimationModel.h"

// アップロードシステム
#include "Graphics/Command/List/Copy/CopyCommandList.h"
#include "Graphics/Resource/Upload/Converter/Json/UploadSystemJsonConverter.h"
#include "Graphics/Resource/Upload/UploadSystem.h"

// リソースコンテキストの定数
#include "Graphics/Resource/Converter/Json/ResourceContextJsonConverter.h"
#include "Graphics/Resource/ResourceContext.h"
#include "Graphics/Resource/Buffer/Dynamic/Converter/Json/DynamicBufferUploaderJsonConverter.h"
#include "Graphics/Resource/Buffer/Dynamic/DynamicBufferUploaderBase.h"

// モデルのアニメーション管理クラス
#include "Graphics/Resource/Model/Skeletal/Player/Evaluator/SkeletalAnimationPoseEvaluator.h"
#include "Graphics/Resource/Model/Skeletal/Player/Buffer/SkeletalAnimationBoneMatrixBufferUploader.h"
#include "Graphics/Resource/Model/Skeletal/Player/SkeletalAnimationPlayer.h"

// 定数バッファークラス
#include "Definition/Constant/Utility/Math/MathAlignUpUtilityConstant.h"
#include "Utility/Math/MathAlignUpUtility.h"
#include "Graphics/Resource/Buffer/Dynamic/Constant/DynamicConstantBufferUploaderBase.h"
#include "Definition/Type/Alias/Factory/Shared/DynamicBufferSharedFactory.h"
#include "Graphics/Resource/Buffer/Dynamic/Constant/DynamicConstantBufferAdvancingWritePositionUploaderBase.h"
#include "Graphics/Resource/Buffer/Dynamic/Constant/DynamicConstantBufferFixedWritePositionUploaderBase.h"

// バーテックスバッファー
#include "Graphics/Resource/Buffer/Dynamic/Vertex/DynamicVertexBufferUploaderBase.h"
#include "Graphics/Resource/Buffer/Dynamic/Vertex/DynamicVertexBufferAdvancingWritePositionUploaderBase.h"

// フレームリソース
#include "Graphics/Render/Frame/Converter/Json/FrameResourceJsonConverter.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedDynamicBufferUploaderBaseConcept.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/PassTextureBase.h"
#include "Definition/Enum/Graphics/RenderGraphPassTextureEnum.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/RenderTarget/Converter/Json/RenderTargetPassTextureJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/RenderTarget/RenderTargetPassTexture.h"
#include "Definition/Enum/Graphics/DepthStencilPassTextureEnum.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/DepthStencil/Converter/Json/DepthStencilPassTextureJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/DepthStencil/DepthStencilPassTexture.h"
#include "Graphics/Render/Frame/RenderGraph/Converter/Json/RenderGraphFrameResourceJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/RenderGraphFrameResource.h"
#include "Graphics/Render/Frame/FrameResource.h"

// カメラ
#include "Definition/Constant/Graphics/Buffer/Constant/CBCameraPassStructConstant.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBCameraPassStruct.h"
#include "Graphics/Render/Camera/Camera.h"

// ライト
#include "Definition/Constant/Graphics/LightSystemConstant.h"
#include "Definition/Struct/Graphics/LightSystemStruct.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBLightStruct.h"
#include "Graphics/Render/Light/LightSystem.h"

// 影
#include "Definition/Constant/Graphics/CascadeShadowMapJsonConverterConstant.h"
#include "Graphics/Render/Shadow/Cascade/Converter/Json/CascadeShadowMapJsonConverter.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBModelCascadeShadowPassStruct.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBCascadeShadowMapPassStruct.h"
#include "Graphics/Render/Shadow/Cascade/CascadeShadowMap.h"
#include "Graphics/Render/Shadow/Converter/Json/ShadowContextJsonConverter.h"
#include "Graphics/Render/Shadow/ShadowContext.h"

// レンダーグラフ
// 描画リクエスト(共通パス)
#include "Graphics/Render/Graph/Request/Pass/DrawRequestPassBase.h"
#include "Graphics/Render/Graph/Request/Pass/CachedPassConstantBufferDrawRequestBase.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedDrawRequestPassBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/DrawRequestPassSharedFactory.h"
#include "Graphics/Render/Graph/Request/Pass/Sprite/Screen/Buffer/Constant/SpriteScreenPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Sprite/Screen/SpriteScreenPassDrawRequest.h"
#include "Graphics/Render/Graph/Request/Pass/Camera/Buffer/Constant/CameraPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Camera/CameraPassDrawRequest.h"
#include "Graphics/Render/Graph/Request/Pass/Light/Buffer/Constant/LightPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Light/LightPassDrawRequest.h"
#include "Graphics/Render/Graph/Request/Pass/Shadow/Cascade/Buffer/Constant/CascadeShadowMapPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Shadow/Cascade/CascadeShadowMapPassDrawRequest.h"

// コンピュートコマンドリスト
#include "Graphics/Command/List/Compute/ComputeCommandList.h"

// コンピュートシェーダー計算リクエスト
#include "Graphics/Render/Graph/Request/Object/ComputeRequestPerObjectBase.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedComputeRequestPerObjectBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/ComputeRequestPerObjectSharedFactory.h"

// 描画リクエスト(レンダーターゲット共通パス)
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/DrawRequestRenderTargetPassBase.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBFinalColorRenderTargetPassStruct.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/Buffer/Constant/FinalColorRenderTargetPassConstantBufferUploader.h"
#include "Definition/Constant/Graphics/RenderTargetFullScreenTriangleDrawConstant.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/FinalColorRenderTargetPassDrawRequest.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBFinalPresentRenderTargetPassStruct.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/Buffer/Constant/FinalPresentRenderTargetPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/FinalPresentRenderTargetPassDrawRequest.h"
#include "Graphics/Render/Graph/Pass/Model/Shadow/Cascade/Buffer/Constant/ModelCascadeShadowPassDynamicConstantBufferUploader.h"

// コンピュートシェーダー計算リクエスト
#include "Definition/Struct/Graphics/Buffer/Constant/CBSkeletalAnimationVertexSkinningPerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Model/Skeletal/Buffer/Constant/SkeletalAnimationVertexSkinningPerObjectDynamicConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Object/Model/Skeletal/SkeletalAnimationPerObjectComputeRequest.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBSkeletalAnimationMeshletBoundsUpdatePerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Model/Skeletal/Buffer/Constant/SkeletalAnimationMeshletBoundsUpdatePerObjectDynamicConstantBufferUploader.h"

// 描画リクエスト(共通していないパス)
#include "Graphics/Render/Graph/Request/Object/DrawRequestPerObjectBase.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedDrawRequestPerObjectBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/DrawRequestPerObjectSharedFactory.h"
#include "Definition/Constant/Graphics/SpriteScreenConstant.h"
#include "Definition/Struct/Graphics/SpriteRECTStruct.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBSpritePerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/Buffer/Constant/SpriteScreenPerObjectDynamicConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/SpriteScreenPerObjectDrawRequest.h"
#include "Definition/Constant/Graphics/ModelPerObjectConstantBufferUploaderConstant.h"
#include "Definition/Constant/Graphics/Buffer/Constant/CBModelPerObjectStructConstant.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBModelPerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Model/Buffer/Constant/ModelPerObjectDynamicConstantBufferUploader.h"
#include "Utility/Math/MathMatrixUtility.h"
#include "Definition/Struct/Graphics/ModelCascadeShadowPassStruct.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/StaticModelPerObjectDrawRequestBase.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/UnLit/StaticModelStandardUnLitPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/Lit/StaticModelStandardLitPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/Model/Skeletal/SkeletalAnimationModelPerObjectDrawRequestBase.h"
#include "Graphics/Render/Graph/Request/Object/Model/Skeletal/Standard/UnLit/SkeletalAnimationModelStandardUnLitPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/Model/Skeletal/Standard/Lit/SkeletalAnimationModelStandardLitPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Shadow/Cascade/StaticModelCascadeShadowPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/Model/Skeletal/Shadow/Cascade/SkeletalAnimationModelCascadeShadowPerObjectDrawRequest.h"

// 描画リクエスト制御パス
#include "Definition/Enum/Graphics/RenderGraphPassEnum.h"
#include "Definition/Enum/Graphics/RenderGraphShadowMapEnum.h"
#include "Definition/Struct/Graphics/RenderGraphPassStruct.h"
#include "Graphics/Render/Graph/Pass/RenderGraphPassBase.h"
#include "Definition/Type/Alias/Factory/Unique/RenderGraphPassUniqueFactory.h"
#include "Graphics/Render/Graph/Pass/Sprite/Screen/SpriteScreenPass.h"
#include "Graphics/Render/Graph/Pass/Final/FinalColorPass.h"
#include "Graphics/Render/Graph/Pass/Final/FinalPresentPass.h"
#include "Graphics/Render/Graph/Pass/Model/Static/Standard/UnLit/StaticModelStandardUnLitPass.h"
#include "Graphics/Render/Graph/Pass/Model/Static/Standard/Lit/StaticModelStandardLitPass.h"
#include "Graphics/Render/Graph/Pass/Model/Skeletal/SkeletalAnimationComputePass.h"
#include "Graphics/Render/Graph/Pass/Model/Shadow/Cascade/ModelCascadeShadowPass.h"

#include "Graphics/Render/Graph/Pass/Model/Skeletal/Standard/UnLit/SkeletalAnimationModelStandardUnLitPass.h"
#include "Graphics/Render/Graph/Pass/Model/Skeletal/Standard/Lit/SkeletalAnimationModelStandardLitPass.h"
#include "Definition/Struct/Graphics/Buffer/Vertex/VBPhysicsDebugStruct.h"
#include "Graphics/Render/Graph/Pass/PhysicsDebug/Buffer/PhysicsDebugDynamicVertexBufferUploader.h"
#include "Graphics/Render/Graph/Pass/PhysicsDebug/PhysicsDebugPass.h"

// トポロジカルソート便利クラス
#include "Utility/Sorter/Topologycal/TopologicalSorter.h"

#include "Graphics/Render/Graph/Converter/Json/RenderGraphJsonConverter.h"
#include "Graphics/Render/Graph/Resource/Clearer/RenderGraphResourceClearer.h"
#include "Graphics/Render/Graph/Resource/Transitioner/RenderGraphResourceTransitioner.h"
#include "Graphics/Render/Graph/Resource/Binder/RenderGraphResourceBinder.h"
#include "Graphics/Render/Graph/Sorter/RenderGraphPassSorter.h"
#include "Graphics/Render/Graph/RenderGraph.h"

// レンダーラー
#include "Utility/TypeINFO/TypeINFOUtility.h"
#include "Graphics/Render/Converter/Json/RendererJsonConverter.h"
#include "Graphics/Render/Renderer.h"

// グラフィックスマネージャー
#include "Graphics/Converter/Json/GraphicsManagerJsonConverter.h" 
#include "Graphics/GraphicsManager.h"

//===============================================================================
// 当たり判定
//===============================================================================
#include "Definition/Enum/Physics/PhysicsLayerEnum.h"
#include "Physics/Layer/PhysicsLayerSetting.h"
#include "Utility/Physics/PhysicsConvertVec3Utility.h"
#include "Physics/Body/PhysicsBodyBase.h"
#include "Physics/Body/Static/PhysicsStaticBodyBase.h"
#include "Definition/Constant/Physics/PhysicsStaticSphereBodyConstant.h"
#include "Physics/Body/Static/Sphere/PhysicsStaticSphereBody.h"
#include "Definition/Constant/Physics/PhysicsStaticBoxBodyConstant.h"
#include "Physics/Body/Static/Box/PhysicsStaticBoxBody.h"
#include "Definition/Constant/Physics/PhysicsStaticCapsuleBodyConstant.h"
#include "Physics/Body/Static/Capsule/PhysicsStaticCapsuleBody.h"
#include "Physics/Body/Static/Mesh/PhysicsStaticMeshBody.h"
#include "Physics/DebugRenderer/Queue/PhysicsDebugRendererQueue.h"
#include "Physics/DebugRenderer/Batch/PhysicsDebugTriangleBatch.h"
#include "Physics/DebugRenderer/PhysicsDebugRenderer.h"
#include "Definition/Struct/Physics/PhysicsCharacterVirtualStruct.h"
#include "Definition/Constant/Physics/PhysicsCharacterVirtualBaseConstant.h"
#include "Physics/CharacterVirtual/Converter/Json/PhysicsCharacterVirtualJsonConverter.h"
#include "Physics/CharacterVirtual/PhysicsCharacterVirtualBase.h"
#include "Physics/CharacterVirtual/Gravity/Affected/PhysicsCharacterVirtualAffectedByGravity.h"
#include "Physics/CharacterVirtual/Gravity/Unaffected/PhysicsCharacterVirtualUnaffectedByGravity.h"
#include "Definition/Struct/Physics/PhysicsRayCastStruct.h"
#include "Physics/RayCast/PhysicsRayCast.h"
#include "Definition/Constant/Physics/PhysicaManagerJsonConverterConstant.h"
#include "Physics/Converter/Json/PhysicsManagerConverterJson.h"
#include "Physics/PhysicsManager.h"

//===============================================================================
// 文字列をキーとして値を保持するRegistry
//===============================================================================
#include "Utility/String/StringValueBidirectionalRegistry.h"
#include "Definition/Macros/Utility/String/StringValueBidirectionalRegistryMacros.h"

//===============================================================================
// オブザーバー
//===============================================================================
#include "Definition/Enum/Observer/ObserverEnum.h"
#include "Definition/Constant/Utility/Enum/EnumBitShiftUtilityConstant.h"
#include "Utility/Enum/EnumBitShiftUtility.h"
#include "Definition/Constant/IMGUI/IMGUIConstant.h"
#include "Utility/IMGUI/Observer/IMGUIObserverUtility.h"
#include "Utility/IMGUI/Bool/IMGUIBoolToString.h"
#include "Observer/Converter/Json/ObserverJsonConverter.h"
#include "Observer/Observer.h"

//===============================================================================
// プレハブ
//===============================================================================
#include "Prefab/Allocator/Converter/Json/PrefabInstanceNUMAllocatorJsonConverter.h"
#include "Definition/Type/Alias/PrefabSceneInstanceNUMTypeAlias.h"
#include "Definition/Constant/Prefab/PrefabInstanceNUMAllocatorConstant.h"
#include "Prefab/Allocator/PrefabInstanceNUMAllocator.h"
#include "Definition/Enum/Prefab/PrefabEnum.h"
#include "Prefab/Converter/Json/PrefabJsonConverter.h"
#include "Prefab/Prefab.h"
#include "Definition/Struct/Prefab/PrefabSystemStruct.h"
#include "Prefab/Converter/Json/PrefabSystemJsonConverter.h"
#include "Prefab/PrefabSystem.h"

//===============================================================================
// コンポーネント(基底クラス)
//===============================================================================
#include "Component/ComponentBase.h"
#include "Definition/Concept/IsDerivedBase/Component/IsDerivedComponentBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/ComponentSharedFactory.h"

//===============================================================================
// ゲームオブジェクト
//===============================================================================
#include "Utility/UUID/UUIDManager.h"
#include "Utility/UUID/UUIDRegistry.h"
#include "Definition/Struct/GameObject/GameObjectJsonConverterStruct.h"
#include "Definition/Constant/GameObject/GameObjectPrefabJsonConverterConstant.h"
#include "GameObject/Converter/Json/Prefab/GameObjectPrefabJsonConverter.h"
#include "GameObject/Converter/Json/Scene/GameObjectSceneJsonConverter.h"
#include "GameObject/Converter/Json/GameObjectJsonConverter.h"
#include "GameObject/GameObject.h"
#include "Utility/GameObject/GameObjectUtility.h"

//===============================================================================
// コンポーネント
//===============================================================================
#include "Strategy/StrategyBase.h"
#include "Utility/Math/MathQuaternionUtility.h"
#include "Utility/IMGUI/Factory/IMGUIFactoryRadioButtonSelectorUtility.h"
#include "Definition/Struct/Component/TransformComponentStruct.h"
#include "Component/Transform/Converter/Json/TransformComponentJsonConverter.h"
#include "Component/Transform/Inspector/TransformComponentInspector.h"
#include "Component/Transform/TransformComponent.h"
#include "Component/Transform/Strategy/MatrixStrategyBase.h"
#include "Definition/Type/Alias/Factory/Unique/MatrixStrategyUniqueFactory.h"
#include "Component/Transform/Strategy/Standalone/StandaloneMatrixStrategy.h"
#include "Component/Transform/Strategy/Hierarchical/HierarchicalMatrixStrartegy.h"
#include "Utility/Helper/Asset/Converter/Json/AssetFilePathHelperUtilityJsonConverter.h"
#include "Utility/Helper/Asset/AssetFilePathHelperUtility.h"
#include "Component/Render/Model/Converter/Json/ModelComponentBaseJsonConverter.h"
#include "Component/Render/Model/ModelComponentBase.h"
#include "Component/Render/Model/Static/StaticModelComponent.h"
#include "Component/Render/Model/Static/Storategy/StaticModelRegisterDrawRequestStorategyBase.h"
#include "Definition/Type/Alias/Factory/Unique/StaticModelRegisterDrawRequestStorategyBaseUniqueFactory.h"
#include "Component/Render/Model/Static/Storategy/Standard/Lit/StaticModelRegisterDrawRequestStandardLitStorategy.h"
#include "Component/Render/Model/Static/Storategy/Standard/UnLit/StaticModelRegisterDrawRequestStandardUnLitStorategy.h"

//===============================================================================
// シーン
//===============================================================================
#include "Scene/Converter/Json/SceneJsonConverter.h"
#include "Scene/Scene.h"
#include "Scene/Converter/Json/SceneManagerJsonConveter.h"
#include "Scene/SceneManager.h"

//===============================================================================
// エディター
//===============================================================================
#include "Utility/IMGUI/DragDrop/IMGUIDragDropPayloadStorageUtility.h"
#include "Definition/Enum/Editor/Window/WorldOutlinerEditorWindowEnum.h"
#include "Definition/Struct/Editor/Window/WorldOutlinerEditorWindowStruct.h"
#include "Editor/Window/Outliner/Selection/WorldOutlinerEditorWindowGameObjectSelection.h"
#include "Editor/Window/Outliner/Hierarchy/WorldOutlinerEditorWindowGameObjectHierarchy.h"
#include "Editor/Window/Outliner/WorldOutlinerEditorWindow.h"
#include "Utility/IMGUI/Tooltip/IMGUITooltipUtility.h"
#include "Editor/Window/Details/DetailsEditorWindow.h"
#include "Editor/Window/ContentBrowser/Asset/Converter/Json/ContentBrowserEditorWindowAssetRegistryJsonConverter.h"
#include "Editor/Window/ContentBrowser/Asset/ContentBrowserEditorWindowAssetRegistry.h"
#include "Editor/Window/ContentBrowser/File/ContentBrowserEditorWindowFileSystem.h"
#include "Editor/Window/ContentBrowser/Converter/Json/ContentBrowserEditorWindowJsonConverter.h"
#include "Editor/Window/ContentBrowser/ContentBrowserEditorWindow.h"