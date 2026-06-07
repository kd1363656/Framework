#pragma once

//===============================================================================
// 型エイリアス
//===============================================================================
#include "Definition/Type/Alias/TypeAlias.h"

//===============================================================================
// 定数
//===============================================================================
#include "Definition/Constant/Constant.h"

//===============================================================================
// Enum
//===============================================================================
#include "Definition/Enum/Type/Trait/PTR/TypeTraitPTREnum.h"

//===============================================================================
// TypeTrait
//===============================================================================
#include "Definition/Type/Trait/PTR/TypeTraitPTR.h"

//===============================================================================
// マクロ
//===============================================================================
#include "Definition/Macros/Assert/AssertReturnMacros.h"
#include "Definition/Macros/Json/JsonSerializeEnumMacros.h"

//===============================================================================
// 継承しているかどうかを確認するコンセプト
//===============================================================================
#include "Definition/Concept/IsDerivedBase/IsDerivedBaseConcept.h"

//===============================================================================
// スマートポインタかどうかを確認するコンセプト
//===============================================================================
#include "Definition/Concept/IsSmartPTR/IsSmartPTRConcept.h"

//===============================================================================
// 文字列ハッシュ化構造体
//===============================================================================
#include "Definition/Struct/StringHash/StringHashStruct.h"

//===============================================================================
// シングルトンを安全に扱うための基底クラス
//===============================================================================
#include "Utility/Singleton/SingletonUtility.h"

//===============================================================================
// 型情報
//===============================================================================
#include "TypeINFO/Definition/Type/Alias/TypeINFOTypeAlias.h"
#include "TypeINFO/Definition/Constant/TypeINFOConstant.h"
#include "TypeINFO/Definition/Struct/TypeINFOStruct.h"

// 型情報生成
#include "TypeINFO/StaticTypeID/StaticTypeIDGenerator.h"

// 型情報登録
#include "TypeINFO/Registry/TypeINFORegistry.h"

// 型用便利関数
#include "TypeINFO/Utility/TypeINFOUtility.h"

// 型情報登録マクロ(通常版)
#include "TypeINFO/Definition/Macros/TypeINFOMacros.h"

//===============================================================================
// 凡庸ファクトリー
//===============================================================================
#include "Factory/GenericFactory.h"
#include "Factory/Macros/FactoryMacros.h"

//===============================================================================
// バイナリーファイル化基底クラス
//===============================================================================
#include "Converter/Binary/BinaryFileConverterBase.h"

//===============================================================================
// 文字列変換便利関数
//===============================================================================
#include "Utility/String/Constant/StringUtilityConstant.h"
#include "Utility/String/StringUtility.h"

//===============================================================================
// ファイル読み込み便利関数
//===============================================================================
#include "Utility/File/Definition/Constant/FileUtilityConstant.h"
#include "Utility/File/FileUtility.h"

//===============================================================================
// json変換便利関数
//===============================================================================
#include "Utility/Json/JsonUtility.h"

//===============================================================================
// ウィンドウクラス
//===============================================================================
#include "Window/Definition/Enum/WindowEnum.h"
#include "Window/Definition/Constant/WindowConstant.h"
#include "Window/Definition/Struct/WindowStruct.h"
#include "Window/Converter/Json/WindowJsonConverter.h"
#include "Window/Window.h"

//===============================================================================
// FPS管理クラス
//===============================================================================
#include "FPS/Converter/Json/FPSControllerJsonConverter.h"
#include "FPS/FPSController.h"

//===============================================================================
// 描画管理クラス
//===============================================================================
// グラフィックスマネージャー
#include "Graphics/Definition/Type/Alias/GraphicsManagerTypeAlias.h"
#include "Graphics/Definition/Constant/GraphicsManagerConstant.h"

// グラフィックスデバイス管理クラス
#include "Graphics/Hardware/Factory.h"
#include "Graphics/Hardware/Device.h"

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

// コピー用キュー、アロケーター、リスト
#include "Graphics/Resource/Upload/Command/Allocator/Definition/Constant/CopyCommandAllocatorConstant.h"
#include "Graphics/Resource/Upload/Command/Allocator/CopyCommandAllocator.h"
#include "Graphics/Resource/Upload/Command/List/CopyCommandList.h"
#include "Graphics/Resource/Upload/Command/Queue/CopyCommandQueue.h"

// リソースコンテキストの定数
#include "Graphics/Resource/Definition/Constant/ResourceContextConstant.h"

// テクスチャ
#include "Graphics/Resource/Texture/Loader/Definition/Enum/TextureLoaderEnum.h"
#include "Graphics/Resource/Texture/Loader/Converter/Binary/Definition/Constant/TextureBinaryConstant.h"
#include "Graphics/Resource/Texture/Loader/Converter/Binary/TextureBinaryConverter.h"
#include "Graphics/Resource/Texture/Loader/TextureLoader.h"
#include "Graphics/Resource/Texture/TextureSystem.h"

// リソースコンテキスト
#include "Graphics/Resource/Converter/Json/ResourceContextJsonConverter.h"
#include "Graphics/Resource/ResourceContext.h"

// ダイレクトコマンド用定数
#include "Graphics/Render/Frame/Command/Allocator/Definition/Constant/DirectCommandAllocatorConstant.h"

// ダイレクトコマンドアロケータ
#include "Graphics/Render/Frame/Command/Allocator/DirectCommandAllocator.h"

// フレームリソース
#include "Graphics/Render/Frame/FrameResource.h"

// ダイレクトコマンドキュー
#include "Graphics/Render/Command/Queue/DirectCommandQueue.h"

// スワップチェイン
#include "Graphics/Render/SwapChain/Definition/Struct/SwapChainStruct.h"
#include "Graphics/Render/SwapChain/Converter/Json/SwapChainJsonConverter.h"
#include "Graphics/Render/SwapChain/SwapChain.h"

// ダイレクトコマンドリスト
#include "Graphics/Render/Command/List/DirectCommandList.h"

// レンダーグラフ
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