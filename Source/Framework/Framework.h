#pragma once

//===============================================================================
// Utilityクラス
//===============================================================================
#include "Utility/Singleton/SingletonUtility.h"
#include "Utility/Stopwatch/Stopwatch.h"

//===============================================================================
// 型情報
//===============================================================================
#include "Definition/Type/Alias/StaticTypeIDTypeAlias.h"
#include "Definition/Macros/Assert/AssertReturnMacros.h"
#include "Definition/Constant/StaticTypeIDConstant.h"
#include "TypeINFO/StaticTypeID/StaticTypeIDGenerator.h"
#include "Definition/Struct/TypeINFOStruct.h"
#include "Definition/Struct/StringHashStruct.h"
#include "TypeINFO/Registry/TypeINFORegistry.h"
#include "Definition/Macros/TypeINFO/TypeINFORegistryMacros.h"

//===============================================================================
// エディター
//===============================================================================
#include "Definition/Type/Alias/MathTypeAlias.h"
#include "Definition/Type/Alias/DescriptorIndexTypeAlias.h"
#include "Definition/Constant/DescriptorHeapConstant.h"
#include "Editor/Style/EditorStyle.h"
#include "Editor/Window/EditorWindowBase.h"
#include "Editor/Window/Log/LogEditorWindow.h"
#include "Editor/Converter/Json/EditorManagerJsonConverter.h"
#include "Definition/Type/Alias/ComPTRTypeAlias.h"
#include "Graphics/Hardware/Factory/Factory.h"
#include "Graphics/Hardware/Device/Device.h"
#include "Graphics/Resource/Descriptor/Heap/DescriptorHeap.h"
#include "Graphics/Resource/Descriptor/Heap/Allocator/Converter/Json/DescriptorHeapIndexAllocatorJsonConverter.h"
#include "Graphics/Resource/Descriptor/Heap/Allocator/DescriptorHeapIndexAllocator.h"
#include "Graphics/Resource/Descriptor/Converter/Json/DescriptorPoolJsonConverter.h"
#include "Graphics/Resource/Descriptor/DescriptorPool.h"
#include "Definition/Type/Alias/DescriptorPoolTypeAlias.h"

#include "Definition/Concept/IsDerivedBase/IsDerivedBaseConcept.h"
#include "Definition/Concept/IsDerivedBase/IsDerivedEditorWindowBaseConcept.h"
#include "Editor/EditorManager.h"

//===============================================================================
// 凡庸ファクトリー
//===============================================================================
#include "Definition/Enum/TypeTraitPTREnum.h"
#include "Definition/Type/Trait/PTR/TypeTraitPTR.h"
#include "Definition/Concept/IsSmartPTR/IsSmartPTRConcept.h"
#include "Factory/GenericFactory.h"
#include "Definition/Macros/Factory/FactoryMacros.h"

#include "Definition/Type/Alias/Factory/Shared/EditorWindowSharedFactory.h"
#include "Editor/Window/Scene/SceneViewEditorWindow.h"

//===============================================================================
// バイナリーファイル化基底クラス
//===============================================================================
#include "Converter/Binary/BinaryFileConverterBase.h"

//===============================================================================
// ウィンドウクラス
//===============================================================================
#include "Definition/Macros/Json/JsonSerializeEnumMacros.h"
#include "Definition/Enum/WindowStyleEnum.h"
#include "Definition/Constant/WindowConstant.h"
#include "Definition/Struct/WindowStruct.h"
#include "Window/Converter/Json/WindowJsonConverter.h"
#include "Definition/Constant/WindowStringUtilityConstant.h"
#include "Window/Utility/WindowStringUtility.h"
#include "Window/Utility/CanLoadFileUtility.h"
#include "Definition/Constant/JsonLoadAndSaveFileUtilityConstant.h"
#include "Window/Utility/JsonLoadAndSaveFileUtility.h"
#include "Window/Window.h"

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
#include "Definition/Enum/RootSignatureEnum.h"
#include "Definition/Enum/RootParameterEnum.h"
#include "Definition/Constant/RootSignatureConstant.h"
#include "Definition/Struct/RootSignatureStruct.h"
#include "Definition/Macros/Json/RootSignatureJsonConverterMacros.h"
#include "Graphics/Render/Pipeline/Converter/Json/Utility/IsArrayJsonUtility.h"
#include "Graphics/Render/Pipeline/Converter/Json/RootSignatureJsonConverter.h"
#include "Graphics/Render/Pipeline/RootSignature.h"

// パイプラインステート
#include "Definition/Enum/PipelineStateBaseEnum.h"
#include "Definition/Macros/Json/PipelineStateJsonConverterMacros.h"
#include "Graphics/Render/Pipeline/Converter/Json/PipelineStateBaseJsonConverter.h"
#include "Graphics/Render/Pipeline/PipelineStateBase.h"
#include "Definition/Type/Alias/Factory/Shared/PipelineStateSharedFactory.h"
#include "Definition/Macros/Json/StandardShaderPipelineStateJsonConverterMacros.h"
#include "Definition/Struct/StandardShaderPipelineStateJsonConverterStruct.h"
#include "Graphics/Render/Pipeline/Standard/Converter/Json/Utility/DeserializeOptionalShaderUtility.h"
#include "Graphics/Render/Pipeline/Standard/Converter/Json/StandardPipelineStateJsonConverter.h"
#include "Graphics/Render/Pipeline/Standard/StandardPipelineState.h"
#include "Graphics/Render/Pipeline/Converter/Json/Mesh/MeshShaderPipelineStateJsonConverter.h"
#include "Graphics/Render/Pipeline/Mesh/MeshShaderPipelineState.h"

// アップロードバッファークラス
#include "Definition/Constant/UploadBufferConstant.h"
#include "Graphics/Resource/Buffer/UploadBuffer.h"

// コマンドキュー、リスト、アロケータの基底クラスとフェンスクラス
#include "Graphics/Command/Allocator/CommandAllocatorBase.h"
#include "Graphics/Command/List/CommandListBase.h"
#include "Definition/Constant/FenceConstant.h"
#include "Graphics/Command/Queue/Fence/Fence.h"
#include "Graphics/Command/Queue/CommandQueueBase.h"

//リソースリリースクラス
#include "Definition/Struct/GPUResourceStruct.h"
#include "Definition/Struct/ResourceReleaseContextStruct.h"
#include "Graphics/Resource/ReleaseContext/ResourceReleaseContext.h"

// レコードクラス
#include "Definition/Type/Alias/StorageIDTypeAlias.h"
#include "Definition/Constant/RecordBaseConstant.h"
#include "Graphics/Resource/Record/AssetRecordBase.h"

// ストレージIDアロケータークラス
#include "Graphics/Resource/Storage/Allocator/Converter/Json/StorageIDAllocatorJsonConverter.h"
#include "Graphics/Resource/Storage/Allocator/StorageIDAllocator.h"

// ダイレクトコマンドアロケータ
#include "Definition/Constant/DirectCommandAllocatorConstant.h"
#include "Graphics/Render/Frame/Command/Allocator/DirectCommandAllocator.h"

// ダイレクトコマンドキュー
#include "Graphics/Render/Command/Queue/DirectCommandQueue.h"

// スワップチェイン
#include "Definition/Constant/SwapChainConstant.h"
#include "Definition/Struct/SwapChainStruct.h"
#include "Graphics/Render/SwapChain/Converter/Json/SwapChainJsonConverter.h"
#include "Graphics/Render/SwapChain/SwapChain.h"

// レンダーエリア
#include "Definition/Struct/ConstantBuffer/CBSpritePassStruct.h"
#include "Graphics/Render/Rasterizer/RenderArea.h"

// ダイレクトコマンドリスト
#include "Definition/Constant/DirectCommandListConstant.h"
#include "Graphics/Render/Command/List/DirectCommandList.h"

// Storageクラス
#include "Definition/Concept/IsDerivedBase/IsDerivedAssetRecordBaseConcept.h"
#include "Graphics/Resource/Storage/Converter/Json/AssetStorageJsonConverter.h"
#include "Graphics/Resource/Storage/AssetStorage.h"

// メモリアロケータクラス
#include "Graphics/Resource/Memory/GPUMemoryAllocator.h"

// ストラクチャードバッファー
#include "Definition/Struct/BufferUploadStruct.h"
#include "Graphics/Resource/Buffer/Structured/StructuredBuffer.h"

// テクスチャ
#include "Definition/Enum/TextureLoadColorSpaceEnum.h"
#include "Graphics/Resource/Texture/Loader/TextureLoader.h"
#include "Definition/Constant/TextureBinaryConstant.h"
#include "Graphics/Resource/Texture/Converter/Binary/TextureBinaryConverter.h"
#include "Graphics/Resource/Texture/Record/TextureRecord.h"
#include "Definition/Struct/TextureBatchUploadRecordBuilderStruct.h"
#include "Graphics/Resource/Texture/Builder/TextureBatchUploadRecordBuilder.h"
#include "Graphics/Resource/Texture/Converter/Json/TextureSystemJsonConverter.h"
#include "Definition/Struct/WStringHashStruct.h"
#include "Definition/Enum/DefaultTextureEnum.h"
#include "Definition/Constant/WStringUtilityConstant.h"
#include "Graphics/Resource/Texture/Default/Utility/String/Definition/DefaultTextureStringUtility.h"
#include "Definition/Constant/DefaultTextureConstant.h"
#include "Graphics/Resource/Texture/Default/Converter/Json/DefaultTextureJsonConverter.h"
#include "Graphics/Resource/Texture/Default/DefaultTexture.h"
#include "Definition/Constant/RenderTargetTextureConstant.h"
#include "Graphics/Resource/Texture/RenderTarget/Utility/IsSameTextureSizeUtility.h"
#include "Graphics/Resource/Texture/RenderTarget/Utility/IsValidTextureUtility.h"
#include "Graphics/Resource/Texture/RenderTarget/RenderTargetTexture.h"
#include "Definition/Struct/TextureSystemStruct.h"
#include "Definition/Constant/DepthStencilTextureConstant.h"
#include "Graphics/Resource/Texture/DepthStencil/DepthStencilTexture.h"
#include "Graphics/Resource/Texture/TextureSystem.h"
#include "Graphics/Resource/Texture/Texture.h"

// モデル(Static)
#include "Definition/Constant/FBXModelLoaderBaseConstant.h"
#include "Graphics/Resource/Model/FBXLoader/FBXModelLoaderBase.h"
#include "Definition/Constant/ModelMaterialConstant.h"
#include "Definition/Constant/StaticModelRecordConstant.h"
#include "Definition/Struct/ModelMaterialStruct.h"
#include "Definition/Struct/StaticModelRecordStruct.h"
#include "Graphics/Resource/Model/Static/Record/StaticModelRecord.h"
#include "Definition/Constant/StaticModelFBXLoaderConstant.h"
#include "Definition/Constant/StaticModelBinaryConverterConstant.h"
#include "Graphics/Resource/Model/Static/Converter/Binary/StaticModelBinaryConverter.h"
#include "Graphics/Resource/Model/Static/FBXLoader/StaticModelFBXLoader.h"
#include "Graphics/Resource/Model/Static/Mesh/StaticModelMeshOptimizer.h"
#include "Definition/Constant/StaticModelMeshletBuilderConstant.h"
#include "Graphics/Resource/Model/Static/Meshlet/StaticModelMeshletBuilder.h"
#include "Graphics/Resource/Model/Static/Builder/StaticModelBatchUploadRecordBuilder.h"
#include "Definition/Struct/StaticModelSystemStruct.h"
#include "Graphics/Resource/Model/Static/Converter/Json/StaticModelSystemJsonConverter.h"
#include "Graphics/Resource/Model/Static/StaticModelSystem.h"
#include "Graphics/Resource/Model/Static/StaticModel.h"

// アップロードシステム
#include "Definition/Constant/CopyCommandAllocatorConstant.h"
#include "Graphics/Resource/Upload/Command/Allocator/CopyCommandAllocator.h"
#include "Graphics/Resource/Upload/Command/List/CopyCommandList.h"
#include "Graphics/Resource/Upload/Command/Queue/CopyCommandQueue.h"
#include "Graphics/Resource/Upload/Converter/Json/UploadSystemJsonConverter.h"
#include "Graphics/Resource/Upload/UploadSystem.h"

// リソースコンテキストの定数
#include "Graphics/Resource/Converter/Json/ResourceContextJsonConverter.h"
#include "Graphics/Resource/ResourceContext.h"

// 定数バッファークラス
#include "Definition/Constant/ConstantBufferUploaderAlignUpUtilityConstant.h"
#include "Graphics/Render/Frame/Buffer/Utility/ConstantBufferUploaderAlignUpUtility.h"
#include "Graphics/Render/Frame/Buffer/Converter/Json/ConstantBufferUploaderJsonConverter.h"
#include "Graphics/Render/Frame/Buffer/ConstantBufferUploaderBase.h"
#include "Definition/Type/Alias/Factory/Shared/ConstantBufferSharedFactory.h"
#include "Graphics/Render/Frame/Buffer/ConstantBufferUploader.h"

// フレームリソース
#include "Definition/Constant/RenderTargetPassTextureJsonConstant.h"
#include "Graphics/Render/Frame/Converter/Json/Utility/FrameResourceJsonUtility.h"
#include "Graphics/Render/Frame/Converter/Json/FrameResourceJsonConverter.h"
#include "Definition/Concept/IsDerivedBase/IsDerivedConstantBufferUploaderBaseConcept.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/PassTextureBase.h"
#include "Definition/Enum/RenderGraphPassTextureEnum.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/RenderTarget/Converter/Json/Utility/ColorJsonUtility.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/RenderTarget/Converter/Json/RenderTargetPassTextureJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/RenderTarget/RenderTargetPassTexture.h"
#include "Definition/Enum/DepthStencilPassTextureEnum.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/DepthStencil/Converter/Json/DepthStencilPassTextureJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/DepthStencil/DepthStencilPassTexture.h"
#include "Graphics/Render/Frame/RenderGraph/Converter/Json/RenderGraphFrameResourceJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/RenderGraphFrameResource.h"
#include "Graphics/Render/Frame/FrameResource.h"

// カメラ
#include "Definition/Constant/CameraConstant.h"
#include "Definition/Struct/ConstantBuffer/CBCameraPassStruct.h"
#include "Graphics/Render/Camera/Camera.h"

// ライト
#include "Definition/Constant/LightSystemConstant.h"
#include "Definition/Struct/LightSystemStruct.h"
#include "Definition/Struct/ConstantBuffer/CBLightStruct.h"
#include "Graphics/Render/Light/LightSystem.h"

// レンダーグラフ
// 描画リクエスト(共通パス)
#include "Graphics/Render/Graph/Request/Pass/DrawRequestPassBase.h"
#include "Graphics/Render/Graph/Request/Pass/CachedPassConstantBufferDrawRequestBase.h"
#include "Definition/Concept/IsDerivedBase/IsDerivedDrawRequestPassBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/DrawRequestPassSharedFactory.h"
#include "Graphics/Render/Graph/Request/Pass/Sprite/Screen/Buffer/SpriteScreenPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Sprite/Screen/SpriteScreenPassDrawRequest.h"
#include "Graphics/Render/Graph/Request/Pass/Camera/Buffer/CameraPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Camera/CameraPassDrawRequest.h"
#include "Graphics/Render/Graph/Request/Pass/Light/Buffer/LightPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Light/LightPassDrawRequest.h"

// 描画リクエスト(レンダーターゲット共通パス)
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/DrawRequestRenderTargetPassBase.h"
#include "Definition/Constant/FinalPresentRenderTargetPassDrawRequestConstant.h"
#include "Definition/Struct/ConstantBuffer/CBFinalColorRenderTargetPassStruct.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/Buffer/FinalColorRenderTargetPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/FinalColorRenderTargetPassDrawRequest.h"

// 描画リクエスト(共通していないパス)
#include "Graphics/Render/Graph/Request/Object/DrawRequestPerObjectBase.h"
#include "Graphics/Render/Graph/Request/Object/DrawRequestPerObjectList.h"
#include "Definition/Concept/IsDerivedBase/IsDerivedDrawRequestPerObjectBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/DrawRequestPerObjectSharedFactory.h"
#include "Definition/Constant/SpriteScreenPerObjectConstantBufferUploaderConstant.h"
#include "Definition/Constant/SpriteScreenConstant.h"
#include "Definition/Struct/SpriteRECTStruct.h"
#include "Definition/Struct/ConstantBuffer/CBSpritePerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/Buffer/SpriteScreenPerObjectConstantBufferUploader.h"
#include "Definition/Struct/SpriteScreenPerObjectDrawRequestStruct.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/SpriteScreenPerObjectDrawRequest.h"
#include "Definition/Constant/StaticModelStandardPerObjectDrawRequestDataConstant.h"
#include "Definition/Struct/StaticModelStandardPerObjectDrawRequestDataStruct.h"
#include "Definition/Constant/StaticModelPerObjectConstantBufferUploaderConstant.h"
#include "Definition/Struct/ConstantBuffer/CBStaticModelPerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/Buffer/StaticModelPerObjectConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/StaticModelStandardPerObjectDrawRequestBase.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/UnLit/StaticModelStandardUnLitPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/Lit/StaticModelStandardLitPerObjectDrawRequest.h"

// 描画リクエスト制御パス
#include "Definition/Enum/RenderGraphPassEnum.h"
#include "Definition/Struct/RenderGraphPassStruct.h"
#include "Graphics/Render/Graph/Pass/RenderGraphPassBase.h"
#include "Definition/Type/Alias/Factory/Shared/RenderGraphPassUniqueFactory.h"
#include "Graphics/Render/Graph/Pass/Sprite/Screen/SpriteScreenPass.h"
#include "Graphics/Render/Graph/Pass/Final/FinalColorPass.h"
#include "Graphics/Render/Graph/Pass/Final/FinalPresentPass.h"
#include "Graphics/Render/Graph/Pass/Model/Static/Standard/UnLit/StaticModelStandardUnLitPass.h"
#include "Graphics/Render/Graph/Pass/Model/Static/Standard/Lit/StaticModelStandardLitPass.h"

// トポロジカルソート便利クラス
#include "Utility/Sorter/Topologycal/TopologicalSorter.h"

#include "Graphics/Render/Graph/Converter/Json/RenderGraphJsonConverter.h"
#include "Graphics/Render/Graph/Resource/Clearer/RenderGraphResourceClearer.h"
#include "Graphics/Render/Graph/Resource/Transitioner/RenderGraphResourceTransitioner.h"
#include "Graphics/Render/Graph/Resource/Binder/RenderGraphResourceBinder.h"
#include "Graphics/Render/Graph/Pass/Sorter/RenderGraphPassSorter.h"
#include "Graphics/Render/Graph/RenderGraph.h"

// レンダーラー
#include "Graphics/Render/Converter/Json/RendererJsonConverter.h"
#include "Graphics/Render/Renderer.h"

// グラフィックスマネージャー
#include "Graphics/Converter/Json/GraphicsManagerJsonConverter.h" 
#include "Graphics/GraphicsManager.h"

// 当たり判定
#include "Definition/Enum/PhysicsLayerEnum.h"
#include "Physics/Layer/PhysicsLayerSetting.h"
#include "Definition/Struct/PhysicsBodyCreatorStruct.h"
#include "Physics/Body/Utility/PhysicsBodyCreatorUitlity.h"
#include "Physics/Body/PhysicsBodyCreator.h"
#include "Physics/PhysicsManager.h"

// シーン
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"