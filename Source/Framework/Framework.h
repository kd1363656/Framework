#pragma once

//===============================================================================
// 型情報
//===============================================================================
#include "TypeINFO/StaticTypeID/Definition/Type/Alias/StaticTypeIDTypeAlias.h"
#include "TypeINFO/StaticTypeID/Definition/Macros/AssertReturnMacros.h"
#include "TypeINFO/StaticTypeID/Definition/Constant/StaticTypeIDConstant.h"
#include "TypeINFO/StaticTypeID/Utility/SingletonUtility.h"
#include "TypeINFO/StaticTypeID/StaticTypeIDGenerator.h"
#include "TypeINFO/Definition/Struct/TypeINFOStruct.h"
#include "TypeINFO/Registry/Definition/Struct/StringHashStruct.h"
#include "TypeINFO/Registry/TypeINFORegistry.h"
#include "TypeINFO/Registry/Definition/Macros/TypeINFORegistryMacros.h"

//===============================================================================
// 凡庸ファクトリー
//===============================================================================
#include "Factory/Definition/Enum/Type/Trait/PTR/TypeTraitPTREnum.h"
#include "Factory/Definition/Type/Trait/PTR/TypeTraitPTR.h"
#include "Factory/Definition/Concept/IsSmartPTR/IsSmartPTRConcept.h"
#include "Factory/Definition/Concept/IsDerivedBase/IsDerivedBaseConcept.h"
#include "Factory/GenericFactory.h"
#include "Factory/Definition/Macros/FactoryMacros.h"

//===============================================================================
// バイナリーファイル化基底クラス
//===============================================================================
#include "Converter/Binary/BinaryFileConverterBase.h"

//===============================================================================
// ウィンドウクラス
//===============================================================================
#include "Window/Converter/Json/Definition/Macros/WindowJsonConverterMacros.h"
#include "Window/Definition/Enum/WindowEnum.h"
#include "Window/Definition/Constant/WindowConstant.h"
#include "Window/Definition/Struct/WindowStruct.h"
#include "Window/Converter/Json/WindowJsonConverter.h"
#include "Window/Utility/Definition/Constant/WindowStringUtilityConstant.h"
#include "Window/Utility/WindowStringUtility.h"
#include "Window/Utility/Definition/Constant/WindowFileUtilityConstant.h"
#include "Window/Utility/WindowFileUtility.h"
#include "Window/Window.h"

//===============================================================================
// FPS管理クラス
//===============================================================================
#include "FPS/Converter/Json/FPSControllerJsonConverter.h"
#include "FPS/FPSController.h"

//===============================================================================
// 描画管理クラス
//===============================================================================

// グラフィックスデバイス管理クラス
#include "Graphics/Hardware/Factory/Definition/Type/Alias/ComPTRTypeAlias.h"
#include "Graphics/Hardware/Factory/Factory.h"
#include "Graphics/Hardware/Device/Device.h"

// シェーダーコンパイラクラス
#include "Graphics/Resource/Shader/Compiler/ShaderCompiler.h"
#include "Graphics/Resource/Shader/Converter/Json/ShaderJsonConverter.h"
#include "Graphics/Resource/Shader/Shader.h"

// ルートシグネチャ
#include "Graphics/Render/Pipeline/Definition/Enum/RootSignatureEnum.h"
#include "Graphics/Render/Pipeline/Definition/Constant/RootSignatureConstant.h"
#include "Graphics/Render/Pipeline/Definition/Struct/RootParameterStruct.h"
#include "Graphics/Render/Pipeline/Converter/Json/Definition/Macros/RootSignatureJsonConverterMacros.h"
#include "Graphics/Render/Pipeline/Converter/Json/Utility/RootSignatureJsonUtility.h"
#include "Graphics/Render/Pipeline/Converter/Json/RootSignatureJsonConverter.h"
#include "Graphics/Render/Pipeline/RootSignature.h"

// パイプラインステート
#include "Graphics/Render/Pipeline/Definition/Enum/PipelineStateEnum.h"
#include "Graphics/Render/Pipeline/Converter/Json/Definition/Macros/PipelineStateJsonConverterMacros.h"
#include "Graphics/Render/Pipeline/Converter/Json/PipelineStateJsonConverter.h"
#include "Graphics/Render/Pipeline/PipelineState.h"

// アップロードバッファークラス
#include "Graphics/Resource/Buffer/Definition/Constant/UploadBufferConstant.h"
#include "Graphics/Resource/Buffer/UploadBuffer.h"

// 定数バッファークラス
#include "Graphics/Render/Frame/Buffer/Definition/Type/Alias/ConstantBufferTypeAlias.h"
#include "Graphics/Render/Frame/Buffer/Definition/Type/Alias/DirectXSimpleMathTypeAlias.h"
#include "Graphics/Render/Frame/Buffer/Definition/Constant/ConstantBufferConstant.h"
#include "Graphics/Render/Frame/Buffer/Definition/Struct/ConstantBufferStruct.h"
#include "Graphics/Render/Frame/Buffer/Utility/Definition/Constant/ConstantBufferUploaderAlignUpUtilityConstant.h"
#include "Graphics/Render/Frame/Buffer/Utility/ConstantBufferUploaderAlignUpUtility.h"
#include "Graphics/Render/Frame/Buffer/Converter/Json/ConstantBufferUploaderJsonConverter.h"
#include "Graphics/Render/Frame/Buffer/ConstantBufferUploaderBase.h"
#include "Graphics/Render/Frame/Buffer/Definition/Type/Alias/Factory/Shared/ConstantBufferSharedFactory.h"
#include "Graphics/Render/Frame/Buffer/ConstantBufferUploader.h"
#include "Graphics/Render/Frame/Buffer/Sprite/Screen/Pass/SpriteScreenPassConstantBufferUploader.h"
#include "Graphics/Render/Frame/Buffer/Sprite/Screen/PerObject/SpriteScreenPerObjectConstantBufferUploader.h"

// コマンドキュー、リスト、アロケータの基底クラスとフェンスクラス
#include "Graphics/Command/Allocator/CommandAllocatorBase.h"
#include "Graphics/Command/List/CommandListBase.h"
#include "Graphics/Command/Queue/Fence/Fence.h"
#include "Graphics/Command/Queue/CommandQueueBase.h"

// ディスクリプタヒープ
#include "Graphics/Resource/Descriptor/Heap/Definition/Constant/DescriptorHeapConstant.h"
#include "Graphics/Resource/Descriptor/Heap/DescriptorHeap.h"

// ディスクリプタヒープアロケーター
#include "Graphics/Resource/Descriptor/Heap/Allocator/Converter/Json/DescriptorHeapIndexAllocatorJsonConverter.h"
#include "Graphics/Resource/Descriptor/Heap/Allocator/DescriptorHeapIndexAllocator.h"

// ディスクリプタプール
#include "Graphics/Resource/Descriptor/Converter/Json/DescriptorPoolJsonConverter.h"
#include "Graphics/Resource/Descriptor/DescriptorPool.h"
#include "Graphics/Resource/Descriptor/Definition/Type/Alias/DescriptorPoolTypeAlias.h"

// 遅延リソースリリースクラス
#include "Graphics/Resource/ReleaseContext/Definition/Constant/ResourceReleaseContextConstant.h"
#include "Graphics/Resource/ReleaseContext/Definition/Struct/ResourceReleaseContextStruct.h"
#include "Graphics/Resource/ReleaseContext/ResourceReleaseContext.h"

// レコードクラス
#include "Graphics/Resource/Record/Definition/Type/Alias/StorageIDTypeAlias.h"
#include "Graphics/Resource/Record/Definition/Constant/RecordBaseConstant.h"
#include "Graphics/Resource/Record/AssetRecordBase.h"
#include "Graphics/Resource/Texture/Record/TextureRecord.h"

// ストレージクラス
#include "Graphics/Resource/Storage/Allocator/Converter/Json/StorageIDAllocatorJsonConverter.h"
#include "Graphics/Resource/Storage/Allocator/StorageIDAllocator.h"

// ダイレクトコマンド用定数
#include "Graphics/Render/Command/List/Definition/Constant/CommandListConstant.h"
#include "Graphics/Render/Frame/Command/Allocator/Definition/Constant/DirectCommandAllocatorConstant.h"

// ダイレクトコマンドアロケータ
#include "Graphics/Render/Frame/Command/Allocator/DirectCommandAllocator.h"

// フレームリソース
#include "Graphics/Render/Frame/Converter/Json/Utility/FrameResourceJsonUtility.h"
#include "Graphics/Render/Frame/Converter/Json/FrameResourceJsonConverter.h"
#include "Graphics/Render/Frame/Definition/Concept/IsDerivedBase/IsDerivedConstantBufferUploaderBaseConcept.h"
#include "Graphics/Render/Frame/FrameResource.h"

// ダイレクトコマンドキュー
#include "Graphics/Render/Command/Queue/DirectCommandQueue.h"

// スワップチェイン
#include "Graphics/Render/SwapChain/Definition/Constant/SwapChainConstant.h"
#include "Graphics/Render/SwapChain/Definition/Struct/SwapChainStruct.h"
#include "Graphics/Render/SwapChain/Converter/Json/SwapChainJsonConverter.h"
#include "Graphics/Render/SwapChain/SwapChain.h"

// レンダーエリア
#include "Graphics/Render/Rasterizer/RenderArea.h"

// ダイレクトコマンドリスト
#include "Graphics/Render/Command/List/DirectCommandList.h"

// Storageクラス
#include "Graphics/Resource/Record/Definition/Concept/IsDerivedBase/IsDerivedAssetRecordBaseConcept.h"
#include "Graphics/Resource/Storage/Converter/Json/AssetStorageJsonConverter.h"
#include "Graphics/Resource/Storage/AssetStorage.h"

// アップロードシステム
#include "Graphics/Resource/Upload/Command/Allocator/Definition/Constant/CopyCommandAllocatorConstant.h"
#include "Graphics/Resource/Upload/Command/Allocator/CopyCommandAllocator.h"
#include "Graphics/Resource/Upload/Command/List/CopyCommandList.h"
#include "Graphics/Resource/Upload/Command/Queue/CopyCommandQueue.h"
#include "Graphics/Resource/Upload/Definition/Struct/UploadSystemStruct.h"
#include "Graphics/Resource/Upload/Converter/Json/UploadSystemJsonConverter.h"
#include "Graphics/Resource/Upload/UploadSystem.h"

// メモリアロケータクラス
#include "Graphics/Resource/Memory/GPUMemoryAllocator.h"

// テクスチャ
#include "Graphics/Resource/Texture/Loader/Definition/Enum/TextureLoaderEnum.h"
#include "Graphics/Resource/Texture/Loader/Converter/Binary/Definition/Constant/TextureBinaryConstant.h"
#include "Graphics/Resource/Texture/Loader/Converter/Binary/TextureBinaryConverter.h"
#include "Graphics/Resource/Texture/Loader/TextureLoader.h"
#include "Graphics/Resource/Texture/Builder/TextureBatchUploadRecordBuilder.h"
#include "Graphics/Resource/Texture/Converter/Json/TextureSystemJsonConverter.h"
#include "Graphics/Resource/Texture/Definition/Struct/TextureSystemStruct.h"
#include "Graphics/Resource/Texture/Definition/Struct/WStringHashStruct.h"
#include "Graphics/Resource/Texture/Default/Definition/Enum/DefaultTextureEnum.h"
#include "Graphics/Resource/Texture/Default/Utility/String/Definition/Constant/DefaultTextureStringUtilityConstant.h"
#include "Graphics/Resource/Texture/Default/Utility/String/Definition/DefaultTextureStringUtility.h"
#include "Graphics/Resource/Texture/Default/Definition/Constant/DefaultTextureConstant.h"
#include "Graphics/Resource/Texture/Default/Converter/Json/DefaultTextureJsonConverter.h"
#include "Graphics/Resource/Texture/Default/DefaultTexture.h"
#include "Graphics/Resource/Texture/RenderTarget/RenderTargetTexture.h"
#include "Graphics/Resource/Texture/TextureSystem.h"
#include "Graphics/Resource/Texture/Texture.h"

// リソースコンテキストの定数
#include "Graphics/Resource/Converter/Json/ResourceContextJsonConverter.h"
#include "Graphics/Resource/ResourceContext.h"

// レンダーグラフ
#include "Graphics/Render/Graph/Definition/Enum/RenderGraphEnum.h"
#include "Graphics/Render/Graph/Definition/Struct/RenderGraphStruct.h"
#include "Graphics/Render/Graph/Request/Pass/DrawRequestPassBase.h"
#include "Graphics/Render/Graph/Request/Pass/CachedPassConstantBufferDrawRequestBase.h"
#include "Graphics/Render/Graph/Request/Pass/Definition/Concept/IsDerivedBase/IsDerivedDrawRequestPassBaseConcept.h"
#include "Graphics/Render/Graph/Request/Pass/Definition/Type/Alias/Factory/Shared/DrawRequestPassSharedFactory.h"
#include "Graphics/Render/Graph/Request/Pass/Sprite/Screen/SpriteScreenPassDrawRequest.h"
#include "Graphics/Render/Graph/Request/Object/DrawRequestPerObjectBase.h"
#include "Graphics/Render/Graph/Request/Object/DrawRequestPerObjectList.h"
#include "Graphics/Render/Graph/Request/Object/Definition/Concept/IsDerivedBase/IsDerivedDrawRequestPerObjectBaseConcept.h"
#include "Graphics/Render/Graph/Request/Object/Definition/Type/Alias/Factory/Shared/DrawRequestPerObjectSharedFactory.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/Definition/Struct/SpriteScreenPerObjectDrawRequestData.h"
#include "Graphics/Render/Graph/Request/Object/Sprite/Screen/SpriteScreenPerObjectDrawRequest.h"
#include "Graphics/Render/Graph/Pass/RenderGraphPassBase.h"
#include "Graphics/Render/Graph/Pass/Definition/Type/Alias/Factory/Unique/RenderGraphPassUniqueFactory.h"
#include "Graphics/Render/Graph/Pass/Sprite/Screen/SpriteScreenPass.h"
#include "Graphics/Render/Graph/Converter/Json/RenderGraphJsonConverter.h"
#include "Graphics/Render/Graph/RenderGraph.h"

// レンダーラー
#include "Graphics/Render/Converter/Json/RendererJsonConverter.h"
#include "Graphics/Render/Renderer.h"

// グラフィックスマネージャー
#include "Graphics/Converter/Json/GraphicsManagerJsonConverter.h" 
#include "Graphics/GraphicsManager.h"

// シーン
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"