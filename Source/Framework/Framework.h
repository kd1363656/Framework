#pragma once

//===============================================================================
// Utilityクラス
//===============================================================================
#include "Utility/Singleton/SingletonUtility.h"
#include "Definition/Enum/TypeTrait/TypeTraitPTREnum.h"
#include "Definition/Type/Trait/PTR/TypeTraitPTR.h"
#include "Definition/Concept/IsSmartPTR/IsSmartPTRConcept.h"
#include "Definition/Macros/Assert/AssertReturnMacros.h"
#include "Utility/Array/VectorArray.h"

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
#include "Editor/Window/Scene/Toolbar/SceneViewToolbar.h"
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
#include "Utility/Json/JsonUtility.h"
#include "Editor/Window/Scene/SceneViewEditorWindow.h"
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
#include "Graphics/Resource/Record/AssetRecordBase.h"

// ストレージIDアロケータークラス
#include "Utility/StorageAllocator/Converter/Json/StorageIDAllocatorJsonConverter.h"
#include "Utility/StorageAllocator/StorageIDAllocator.h"

// スワップチェイン
#include "Graphics/Render/SwapChain/Converter/Json/SwapChainJsonConverter.h"
#include "Graphics/Render/SwapChain/SwapChain.h"

// レンダーエリア
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
#include "Graphics/Resource/Texture/DepthStencil/DepthStencilTexture.h"
#include "Definition/Struct/String/WStringHashStruct.h"
#include "Graphics/Resource/Texture/TextureSystem.h"
#include "Graphics/Resource/Texture/Texture.h"

// モデル(Static)
#include "Definition/Struct/Graphics/ModelCommonStruct.h"
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
#include "Graphics/Resource/Buffer/Dynamic/Structured/DynamicReadWriteStructuredBuffer.h"
#include "Graphics/Resource/Model/SkeletalAnimation/Record/SkeletalAnimationModelRecord.h"
#include "Graphics/Resource/Model/SkeletalAnimation/Converter/Binary/SkeletalAnimationModelBinaryConverter.h"
#include "Definition/Constant/Graphics/SkeletalAnimationModelFBXMotionExtractorConstant.h"
#include "Graphics/Resource/Model/SkeletalAnimation/FBXLoader/Extractor/Skeleton/SkeletalAnimationModelFBXSkeletonExtractor.h"
#include "Graphics/Resource/Model/SkeletalAnimation/FBXLoader/Extractor/Motion/SkeletalAnimationModelFBXMotionExtractor.h"
#include "Graphics/Resource/Model/SkeletalAnimation/FBXLoader/SkeletalAnimationModelFBXLoader.h"
#include "Graphics/Resource/Model/SkeletalAnimation/Builder/SkeletalAnimationModelBatchUploadRecordBuilder.h"
#include "Definition/Struct/Graphics/SkeletalAnimationModelSystemStruct.h"
#include "Graphics/Resource/Model/SkeletalAnimation/SkeletalAnimationModelSystem.h"

// アップロードシステム
#include "Graphics/Command/List/Copy/CopyCommandList.h"
#include "Graphics/Resource/Upload/Converter/Json/UploadSystemJsonConverter.h"
#include "Graphics/Resource/Upload/UploadSystem.h"

// リソースコンテキストの定数
#include "Graphics/Resource/Converter/Json/ResourceContextJsonConverter.h"
#include "Graphics/Resource/ResourceContext.h"

// 定数バッファークラス
#include "Graphics/Resource/Buffer/Dynamic/Converter/Json/DynamicBufferUploaderJsonConverter.h"
#include "Graphics/Resource/Buffer/Dynamic/DynamicBufferUploaderBase.h"
#include "Definition/Constant/Utility/Math/MathAlignUpUtilityConstant.h"
#include "Utility/Math/MathAlignUpUtility.h"
#include "Graphics/Resource/Buffer/Dynamic/Constant/DynamicConstantBufferUploaderBase.h"
#include "Definition/Type/Alias/Factory/Shared/DynamicBufferSharedFactory.h"

// バーテックスバッファー
#include "Graphics/Resource/Buffer/Dynamic/Vertex/DynamicVertexBufferUploaderBase.h"

#include "Definition/Struct/Graphics/Buffer/Vertex/VBPhysicsDebugStruct.h"
#include "Graphics/Resource/Buffer/Dynamic/Vertex/PhysicsDebug/PhysicsDebugDynamicVertexBufferUploader.h"

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
#include "Definition/Constant/Graphics/Buffer/Constant/CBCameraPassConstant.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBCameraPassStruct.h"
#include "Graphics/Render/Camera/Camera.h"

// ライト
#include "Definition/Constant/Graphics/LightSystemConstant.h"
#include "Definition/Struct/Graphics/LightSystemStruct.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBLightStruct.h"
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
#include "Definition/Struct/Graphics/Buffer/Constant/CBFinalColorRenderTargetPassStruct.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/Buffer/FinalColorRenderTargetPassConstantBufferUploader.h"
#include "Definition/Constant/Graphics/RenderTargetFullScreenTriangleDrawConstant.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/FinalColorRenderTargetPassDrawRequest.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBFinalPresentRenderTargetPassStruct.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/Buffer/FinalPresentRenderTargetPassConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Pass/RenderTarget/Final/FinalPresentRenderTargetPassDrawRequest.h"

// 描画リクエスト(共通していないパス)
#include "Graphics/Render/Graph/Request/Object/DrawRequestPerObjectBase.h"
#include "Definition/Concept/IsDerivedBase/Graphics/IsDerivedDrawRequestPerObjectBaseConcept.h"
#include "Definition/Type/Alias/Factory/Shared/DrawRequestPerObjectSharedFactory.h"
#include "Definition/Constant/Graphics/SpriteScreenConstant.h"
#include "Definition/Struct/Graphics/SpriteRECTStruct.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBSpritePerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/Buffer/SpriteScreenPerObjectConstantBufferUploader.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/SpriteScreenPerObjectDrawRequest.h"
#include "Definition/Struct/Graphics/StaticModelStandardPerObjectDrawRequestDataStruct.h"
#include "Definition/Constant/Graphics/StaticModelPerObjectConstantBufferUploaderConstant.h"
#include "Definition/Struct/Graphics/Buffer/Constant/CBStaticModelPerObjectStruct.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/Buffer/StaticModelPerObjectConstantBufferUploader.h"
#include "Utility/Math/MathMatrixUtility.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/StaticModelStandardPerObjectDrawRequestBase.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/UnLit/StaticModelStandardUnLitPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/Model/Static/Standard/Lit/StaticModelStandardLitPerObjectDrawRequest.h"

// 描画リクエスト制御パス
#include "Definition/Enum/Graphics/RenderGraphPassEnum.h"
#include "Definition/Struct/Graphics/RenderGraphPassStruct.h"
#include "Graphics/Render/Graph/Pass/RenderGraphPassBase.h"
#include "Definition/Type/Alias/Factory/Unique/RenderGraphPassUniqueFactory.h"
#include "Graphics/Render/Graph/Pass/Sprite/Screen/SpriteScreenPass.h"
#include "Graphics/Render/Graph/Pass/Final/FinalColorPass.h"
#include "Graphics/Render/Graph/Pass/Final/FinalPresentPass.h"
#include "Graphics/Render/Graph/Pass/Model/Static/Standard/UnLit/StaticModelStandardUnLitPass.h"
#include "Graphics/Render/Graph/Pass/Model/Static/Standard/Lit/StaticModelStandardLitPass.h"
#include "Graphics/Render/Graph/Pass/PhysicsDebug/PhysicsDebugPass.h"

// トポロジカルソート便利クラス
#include "Utility/Sorter/Topologycal/TopologicalSorter.h"

#include "Graphics/Render/Graph/Converter/Json/RenderGraphJsonConverter.h"
#include "Graphics/Render/Graph/Resource/Clearer/RenderGraphResourceClearer.h"
#include "Graphics/Render/Graph/Resource/Transitioner/RenderGraphResourceTransitioner.h"
#include "Graphics/Render/Graph/Resource/Binder/RenderGraphResourceBinder.h"
#include "Graphics/Render/Graph/Pass/Sorter/RenderGraphPassSorter.h"
#include "Graphics/Render/Graph/RenderGraph.h"

// コンピュートコマンドリスト
#include "Graphics/Command/List/Compute/ComputeCommandList.h"

// レンダーラー
#include "Utility/TypeINFO/TypeINFOUtility.h"
#include "Graphics/Render/Converter/Json/RendererJsonConverter.h"
#include "Graphics/Render/Renderer.h"

// グラフィックスマネージャー
#include "Graphics/Converter/Json/GraphicsManagerJsonConverter.h" 
#include "Graphics/GraphicsManager.h"

// 当たり判定
#include "Definition/Enum/Physics/PhysicsLayerEnum.h"
#include "Physics/Layer/PhysicsLayerSetting.h"
#include "Utility/Physics/PhysicsConvertVec3Utility.h"
#include "Physics/Body/PhysicsBodyBase.h"
#include "Physics/Body/Static/PhysicsStaticBodyBase.h"
#include "Physics/Body/Static/Sphere/PhysicsStaticSphereBody.h"
#include "Physics/Body/Static/Box/PhysicsStaticBoxBody.h"
#include "Physics/Body/Static/Capsule/PhysicsStaticCapsuleBody.h"
#include "Physics/Body/Static/Mesh/PhysicsStaticMeshBody.h"
#include "Physics/DebugRenderer/Queue/PhysicsDebugRendererQueue.h"
#include "Physics/DebugRenderer/Batch/PhysicsDebugTriangleBatch.h"
#include "Physics/DebugRenderer/PhysicsDebugRenderer.h"
#include "Definition/Struct/Physics/PhysicsCharacterVirtualStruct.h"
#include "Physics/CharacterVirtual/PhysicsCharacterVirtualBase.h"
#include "Physics/CharacterVirtual/Gravity/Affected/PhysicsCharacterVirtualAffectedByGravity.h"
#include "Physics/CharacterVirtual/Gravity/Unaffected/PhysicsCharacterVirtualUnaffectedByGravity.h"
#include "Physics/PhysicsManager.h"

// コンポーネント
#include "Component/ComponentBase.h"
#include "Strategy/StrategyBase.h"
#include "Definition/Constant/IMGUI/IMGUIConstant.h"
#include "Utility/Math/MathQuaternionUtility.h"
#include "Utility/IMGUI/IMGUIUtility.h"
#include "Component/Transform/TransformComponent.h"
#include "Component/Transform/Strategy/MatrixStrategyBase.h"
#include "Definition/Type/Alias/Factory/Unique/MatrixStrategyUniqueFactory.h"
#include "Component/Transform/Strategy/Standalone/StandaloneMatrixStrategy.h"
#include "Component/Transform/Strategy/Hierarchical/HierarchicalMatrixStrartegy.h"

// ゲームオブジェクト
#include "GameObject/GameObject.h"

// シーン
#include "Scene/Converter/Json/SceneJsonConveter.h"
#include "Scene/Scene.h"
#include "Scene/Converter/Json/SceneManagerJsonConveter.h"
#include "Scene/SceneManager.h"