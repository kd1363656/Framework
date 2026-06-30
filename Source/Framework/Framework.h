#pragma once

//===============================================================================
// Utilityクラス
//===============================================================================
#include "Utility/Singleton/SingletonUtility.h"

//===============================================================================
// 型情報
//===============================================================================
#include "Definition/Type/Alias/StaticTypeIDTypeAlias.h"
#include "Definition/Macros/Assert/AssertReturnMacros.h"
#include "Definition/Constant/Graphics/StaticTypeIDConstant.h"
#include "TypeINFO/StaticTypeID/StaticTypeIDGenerator.h"
#include "Definition/Struct/TypeINFO/TypeINFOStruct.h"
#include "Definition/Struct/Utility/String/StringHashStruct.h"
#include "TypeINFO/Registry/TypeINFORegistry.h"
#include "Definition/Macros/TypeINFO/TypeINFORegistryMacros.h"

//===============================================================================
// グラフィックス(エディターに必要な定義だけここで済ませる)
//===============================================================================
#include "Definition/Type/Alias/ComPTRTypeAlias.h"
#include "Graphics/Hardware/Factory/Factory.h"
#include "Graphics/Hardware/Device/Device.h"
#include "Definition/Type/Alias/DescriptorIndexTypeAlias.h"
#include "Definition/Constant/Graphics/DescriptorHeapConstant.h"
#include "Graphics/Resource/Descriptor/Heap/DescriptorHeap.h"
#include "Graphics/Resource/Descriptor/Heap/Allocator/Converter/Json/DescriptorHeapIndexAllocatorJsonConverter.h"
#include "Graphics/Resource/Descriptor/Heap/Allocator/DescriptorHeapIndexAllocator.h"
#include "Graphics/Resource/Descriptor/Converter/Json/DescriptorPoolJsonConverter.h"
#include "Graphics/Resource/Descriptor/DescriptorPool.h"
#include "Definition/Type/Alias/DescriptorPoolTypeAlias.h"

//===============================================================================
// ウィンドウクラス
//===============================================================================
#include "Definition/Macros/Json/JsonSerializeEnumMacros.h"
#include "Definition/Enum/Window/WindowStyleEnum.h"
#include "Definition/Constant/Window/WindowConstant.h"
#include "Definition/Struct/Window/WindowStruct.h"
#include "Window/Converter/Json/WindowJsonConverter.h"
#include "Definition/Constant/Utility/String/StringUtilityConstant.h"
#include "Utility/String/StringUtility.h"
#include "Utility/File/CanLoadFileUtility.h"
#include "Definition/Constant/Utility/File/Json/JsonLoadAndSaveFileUtilityConstant.h"
#include "Utility/File/Json/JsonLoadAndSaveFileUtility.h"
#include "Window/Window.h"

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
#include "Editor/EditorManager.h"

//===============================================================================
// 凡庸ファクトリー
//===============================================================================
#include "Definition/Enum/TypeTrait/TypeTraitPTREnum.h"
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
#include "Definition/Constant/Graphics/RootSignatureConstant.h"
#include "Definition/Struct/Graphics/RootSignatureStruct.h"
#include "Definition/Macros/Json/RootSignatureJsonConverterMacros.h"
#include "Utility/File/Json/IsArrayJsonUtility.h"
#include "Graphics/Render/Pipeline/Converter/Json/RootSignatureJsonConverter.h"
#include "Graphics/Render/Pipeline/RootSignature.h"

// パイプラインステート
#include "Definition/Enum/Graphics/PipelineStateBaseEnum.h"
#include "Definition/Macros/Json/PipelineStateJsonConverterMacros.h"
#include "Graphics/Render/Pipeline/Converter/Json/PipelineStateBaseJsonConverter.h"
#include "Graphics/Render/Pipeline/PipelineStateBase.h"
#include "Definition/Type/Alias/Factory/Shared/PipelineStateSharedFactory.h"
#include "Definition/Macros/Json/StandardShaderPipelineStateJsonConverterMacros.h"
#include "Definition/Struct/Graphics/StandardShaderPipelineStateJsonConverterStruct.h"
#include "Utility/Graphics/DeserializeOptionalShaderUtility.h"
#include "Graphics/Render/Pipeline/Standard/Converter/Json/StandardPipelineStateJsonConverter.h"
#include "Graphics/Render/Pipeline/Standard/StandardPipelineState.h"
#include "Graphics/Render/Pipeline/Converter/Json/Mesh/MeshShaderPipelineStateJsonConverter.h"
#include "Graphics/Render/Pipeline/Mesh/MeshShaderPipelineState.h"

// アップロードバッファークラス
#include "Definition/Constant/Graphics/UploadBufferConstant.h"
#include "Graphics/Resource/Buffer/UploadBuffer.h"

// コマンドキュー、リスト、アロケータの基底クラスとフェンスクラス
#include "Graphics/Command/Allocator/CommandAllocatorBase.h"
#include "Graphics/Command/List/CommandListBase.h"
#include "Definition/Constant/Graphics/FenceConstant.h"
#include "Graphics/Command/Queue/Fence/Fence.h"
#include "Graphics/Command/Queue/CommandQueueBase.h"

//リソースリリースクラス
#include "Definition/Struct/Graphics/GPUResourceStruct.h"
#include "Definition/Struct/Graphics/ResourceReleaseContextStruct.h"
#include "Graphics/Resource/ReleaseContext/ResourceReleaseContext.h"

// レコードクラス
#include "Definition/Type/Alias/StorageIDTypeAlias.h"
#include "Definition/Constant/Graphics/RecordBaseConstant.h"
#include "Graphics/Resource/Record/AssetRecordBase.h"

// ストレージIDアロケータークラス
#include "Graphics/Resource/Storage/Allocator/Converter/Json/StorageIDAllocatorJsonConverter.h"
#include "Graphics/Resource/Storage/Allocator/StorageIDAllocator.h"

// ダイレクトコマンドアロケータ
#include "Definition/Constant/Graphics/DirectCommandAllocatorConstant.h"
#include "Graphics/Command/Allocator/Direct/DirectCommandAllocator.h"

// ダイレクトコマンドキュー
#include "Graphics/Command/Queue/Direct/DirectCommandQueue.h"

// スワップチェイン
#include "Definition/Constant/Graphics/SwapChainConstant.h"
#include "Definition/Struct/Graphics/SwapChainStruct.h"
#include "Graphics/Render/SwapChain/Converter/Json/SwapChainJsonConverter.h"
#include "Graphics/Render/SwapChain/SwapChain.h"

// レンダーエリア
#include "Definition/Struct/Graphics/ConstantBuffer/CBSpritePassStruct.h"
#include "Graphics/Render/Rasterizer/RenderArea.h"

// ダイレクトコマンドリスト
#include "Definition/Constant/Graphics/DirectCommandListConstant.h"
#include "Graphics/Command/List/Direct/DirectCommandList.h"

// Storageクラス
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedAssetRecordBaseConcept.h"
#include "Graphics/Resource/Storage/Converter/Json/AssetStorageJsonConverter.h"
#include "Graphics/Resource/Storage/AssetStorage.h"

// メモリアロケータクラス
#include "Graphics/Resource/Memory/GPUMemoryAllocator.h"

// ストラクチャードバッファー
#include "Definition/Struct/Graphics/BufferUploadStruct.h"
#include "Graphics/Resource/Buffer/Structured/StructuredBuffer.h"

// テクスチャ
#include "Definition/Enum/Graphics/TextureLoadColorSpaceEnum.h"
#include "Graphics/Resource/Texture/Loader/TextureLoader.h"
#include "Definition/Constant/Graphics/TextureBinaryConstant.h"
#include "Graphics/Resource/Texture/Converter/Binary/TextureBinaryConverter.h"
#include "Graphics/Resource/Texture/Record/TextureRecord.h"
#include "Definition/Struct/Graphics/TextureBatchUploadRecordBuilderStruct.h"
#include "Graphics/Resource/Texture/Builder/TextureBatchUploadRecordBuilder.h"
#include "Graphics/Resource/Texture/Converter/Json/TextureSystemJsonConverter.h"
#include "Definition/Struct/Utility/String/WStringHashStruct.h"
#include "Definition/Enum/Graphics/DefaultTextureEnum.h"
#include "Definition/Constant/Utility/String/WStringUtilityConstant.h"
#include "Utility/String/WStringUtility.h"
#include "Definition/Constant/Graphics/DefaultTextureConstant.h"
#include "Graphics/Resource/Texture/Default/Converter/Json/DefaultTextureJsonConverter.h"
#include "Graphics/Resource/Texture/Default/DefaultTexture.h"
#include "Definition/Constant/Graphics/RenderTargetTextureConstant.h"
#include "Utility/Graphics/IsSameTextureSizeUtility.h"
#include "Utility/Graphics/IsValidTextureUtility.h"
#include "Graphics/Resource/Texture/RenderTarget/RenderTargetTexture.h"
#include "Definition/Struct/Graphics/TextureSystemStruct.h"
#include "Definition/Constant/Graphics/DepthStencilTextureConstant.h"
#include "Graphics/Resource/Texture/DepthStencil/DepthStencilTexture.h"
#include "Graphics/Resource/Texture/TextureSystem.h"
#include "Graphics/Resource/Texture/Texture.h"

// モデル(Static)
#include "Definition/Constant/Graphics/FBXModelLoaderBaseConstant.h"
#include "Graphics/Resource/Model/FBXLoader/FBXModelLoaderBase.h"
#include "Definition/Constant/Graphics/ModelMaterialConstant.h"
#include "Definition/Constant/Graphics/StaticModelRecordConstant.h"
#include "Definition/Struct/Graphics/ModelMaterialStruct.h"
#include "Definition/Struct/Graphics/StaticModelRecordStruct.h"
#include "Graphics/Resource/Model/Static/Record/StaticModelRecord.h"
#include "Definition/Constant/Graphics/StaticModelFBXLoaderConstant.h"
#include "Definition/Constant/Graphics/StaticModelBinaryConverterConstant.h"
#include "Graphics/Resource/Model/Static/Converter/Binary/StaticModelBinaryConverter.h"
#include "Graphics/Resource/Model/Static/FBXLoader/StaticModelFBXLoader.h"
#include "Graphics/Resource/Model/Static/Mesh/StaticModelMeshOptimizer.h"
#include "Definition/Constant/Graphics/StaticModelMeshletBuilderConstant.h"
#include "Graphics/Resource/Model/Static/Meshlet/StaticModelMeshletBuilder.h"
#include "Graphics/Resource/Model/Static/Builder/StaticModelBatchUploadRecordBuilder.h"
#include "Definition/Struct/Graphics/StaticModelSystemStruct.h"
#include "Graphics/Resource/Model/Static/Converter/Json/StaticModelSystemJsonConverter.h"
#include "Graphics/Resource/Model/Static/StaticModelSystem.h"
#include "Graphics/Resource/Model/Static/StaticModel.h"

// アップロードシステム
#include "Definition/Constant/Graphics/CopyCommandAllocatorConstant.h"
#include "Graphics/Resource/Upload/Command/Allocator/CopyCommandAllocator.h"
#include "Graphics/Resource/Upload/Command/List/CopyCommandList.h"
#include "Graphics/Resource/Upload/Command/Queue/CopyCommandQueue.h"
#include "Graphics/Resource/Upload/Converter/Json/UploadSystemJsonConverter.h"
#include "Graphics/Resource/Upload/UploadSystem.h"

// リソースコンテキストの定数
#include "Graphics/Resource/Converter/Json/ResourceContextJsonConverter.h"
#include "Graphics/Resource/ResourceContext.h"

// 定数バッファークラス
#include "Definition/Constant/Utility/Math/MathAlignUpUtilityConstant.h"
#include "Utility/Math/MathAlignUpUtility.h"
#include "Graphics/Resource/Buffer/Constant/Converter/Json/ConstantBufferUploaderJsonConverter.h"
#include "Graphics/Resource/Buffer/Constant/ConstantBufferUploaderBase.h"
#include "Definition/Type/Alias/Factory/Shared/ConstantBufferSharedFactory.h"
#include "Graphics/Resource/Buffer/Constant/ConstantBufferUploader.h"

// フレームリソース
#include "Definition/Constant/Graphics/RenderTargetPassTextureJsonConstant.h"
#include "Graphics/Render/Frame/Converter/Json/Utility/FrameResourceJsonUtility.h"
#include "Graphics/Render/Frame/Converter/Json/FrameResourceJsonConverter.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedConstantBufferUploaderBaseConcept.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/PassTextureBase.h"
#include "Definition/Enum/Graphics/RenderGraphPassTextureEnum.h"
#include "Utility/Graphics/ColorJsonUtility.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/RenderTarget/Converter/Json/RenderTargetPassTextureJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/RenderTarget/RenderTargetPassTexture.h"
#include "Definition/Enum/Graphics/DepthStencilPassTextureEnum.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/DepthStencil/Converter/Json/DepthStencilPassTextureJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/Pass/Texture/DepthStencil/DepthStencilPassTexture.h"
#include "Graphics/Render/Frame/RenderGraph/Converter/Json/RenderGraphFrameResourceJsonConverter.h"
#include "Graphics/Render/Frame/RenderGraph/RenderGraphFrameResource.h"
#include "Graphics/Render/Frame/FrameResource.h"

// カメラ
#include "Definition/Constant/Graphics/CameraConstant.h"
#include "Definition/Struct/Graphics/ConstantBuffer/CBCameraPassStruct.h"
#include "Graphics/Render/Camera/Camera.h"

// ライト
#include "Definition/Constant/Graphics/LightSystemConstant.h"
#include "Definition/Struct/Graphics/LightSystemStruct.h"
#include "Definition/Struct/Graphics/ConstantBuffer/CBLightStruct.h"
#include "Graphics/Render/Light/LightSystem.h"

// レンダーグラフ
// 描画リクエスト(共通パス)
#include "Graphics/Render/Graph/Request/Pass/DrawRequestPassBase.h"
#include "Graphics/Render/Graph/Request/Pass/CachedPassConstantBufferDrawRequestBase.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedDrawRequestPassBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/DrawRequestPassSharedFactory.h"
#include "Graphics/Render/Graph/Request/Pass/Sprite/Screen/Buffer/SpriteScreenPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Sprite/Screen/SpriteScreenPassDrawRequest.h"
#include "Graphics/Render/Graph/Request/Pass/Camera/Buffer/CameraPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Camera/CameraPassDrawRequest.h"
#include "Graphics/Render/Graph/Request/Pass/Light/Buffer/LightPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/Light/LightPassDrawRequest.h"

// 描画リクエスト(レンダーターゲット共通パス)
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/DrawRequestRenderTargetPassBase.h"
#include "Definition/Constant/Graphics/FinalPresentRenderTargetPassDrawRequestConstant.h"
#include "Definition/Struct/Graphics/ConstantBuffer/CBFinalColorRenderTargetPassStruct.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/Buffer/FinalColorRenderTargetPassConstantBufferUploader.h"
#include "Definition/Constant/Graphics/RenderTargetFullScreenTriangleDrawConstant.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/FinalColorRenderTargetPassDrawRequest.h"

// 描画リクエスト(共通していないパス)
#include "Graphics/Render/Graph/Request/Object/DrawRequestPerObjectBase.h"
#include "Graphics/Render/Graph/Request/Object/DrawRequestPerObjectList.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedDrawRequestPerObjectBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/DrawRequestPerObjectSharedFactory.h"
#include "Definition/Constant/Graphics/SpriteScreenPerObjectConstantBufferUploaderConstant.h"
#include "Definition/Constant/Graphics/SpriteScreenConstant.h"
#include "Definition/Struct/Graphics/SpriteRECTStruct.h"
#include "Definition/Struct/Graphics/ConstantBuffer/CBSpritePerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/Buffer/SpriteScreenPerObjectConstantBufferUploader.h"
#include "Definition/Struct/Graphics/SpriteScreenPerObjectDrawRequestStruct.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/SpriteScreenPerObjectDrawRequest.h"
#include "Definition/Constant/Graphics/StaticModelStandardPerObjectDrawRequestDataConstant.h"
#include "Definition/Struct/Graphics/StaticModelStandardPerObjectDrawRequestDataStruct.h"
#include "Definition/Constant/Graphics/StaticModelPerObjectConstantBufferUploaderConstant.h"
#include "Definition/Struct/Graphics/ConstantBuffer/CBStaticModelPerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/Buffer/StaticModelPerObjectConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/StaticModelStandardPerObjectDrawRequestBase.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/UnLit/StaticModelStandardUnLitPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/Lit/StaticModelStandardLitPerObjectDrawRequest.h"

// 描画リクエスト制御パス
#include "Definition/Enum/Graphics/RenderGraphPassEnum.h"
#include "Definition/Struct/Graphics/RenderGraphPassStruct.h"
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
#include "Definition/Enum/Physics/PhysicsLayerEnum.h"
#include "Physics/Layer/PhysicsLayerSetting.h"
#include "Definition/Struct/Graphics/PhysicsBodyCreatorStruct.h"
#include "Utility/Physics/PhysicsConevrtVec3Uitlity.h"
#include "Physics/Body/PhysicsBodyCreator.h"
#include "Physics/PhysicsManager.h"

// コンポーネント
#include "Component/ComponentBase.h"

// ゲームオブジェクト
#include "GameObject/GameObject.h"

// シーン
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"