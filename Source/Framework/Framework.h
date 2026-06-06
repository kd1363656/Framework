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
#include "TypeINFO/StaticTypeID/StaticTypeIDGenerator.h"
#include "TypeINFO/Registry/TypeINFORegistry.h"

// 型情報登録マクロ(通常版)
#include "TypeINFO/Definition/Macros/TypeINFOMacros.h"

//===============================================================================
// 凡庸ファクトリー
//===============================================================================
#include "Factory/GenericFactory.h"
#include "Factory/Macros/FactoryMacros.h"

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

// ディスクリプタヒープ
#include "Graphics/Resource/Descriptor/Heap/Definition/Constant/DescriptorHeapConstant.h"
#include "Graphics/Resource/Descriptor/Heap/Definition/Struct/DescriptorHeapStruct.h"
#include "Graphics/Resource/Descriptor/Heap/DescriptorHeap.h"

// ディスクリプタヒープアロケーター
#include "Graphics/Resource/Descriptor/Heap/Allocator/Converter/Json/DescriptorHeapIndexAllocatorJsonConverter.h"
#include "Graphics/Resource/Descriptor/Heap/Allocator/DescriptorHeapIndexAllocator.h"

// ディスクリプタプール
#include "Graphics/Resource/Descriptor/Converter/Json/DescriptorPoolJsonConverter.h"
#include "Graphics/Resource/Descriptor/DescriptorPool.h"

// リソースコンテキスト
#include "Graphics/Resource/Definition/Type/Alias/ResourceContextTypeAlias.h"
#include "Graphics/Resource/Converter/Json/ResourceContextJsonConverter.h"
#include "Graphics/Resource/ResourceContext.h"

// コマンド用定数
#include "Graphics/Command/Definition/Constant/CommandConstant.h"

// コマンドアロケータ
#include "Graphics/Command/Allocator/CommandAllocatorBase.h"
#include "Graphics/Command/Allocator/Direct/DirectCommandAllocator.h"

// コマンドリスト
#include "Graphics/Command/List/CommandListBase.h"
#include "Graphics/Command/List/Direct/DirectCommandList.h"

// フェンス
#include "Graphics/Command/Queue/Fence/Fence.h"

// コマンドキュー
#include "Graphics/Command/Queue/CommandQueueBase.h"
#include "Graphics/Command/Queue/Direct/DirectCommandQueue.h"

// フレームリソース
#include "Graphics/Render/Frame/FrameResource.h"

// スワップチェイン
#include "Graphics/Render/SwapChain/Definition/Struct/SwapChainStruct.h"
#include "Graphics/Render/SwapChain/Converter/Json/SwapChainJsonConverter.h"
#include "Graphics/Render/SwapChain/SwapChain.h"

// レンダーグラフ
#include "Graphics/Render/Graph/Definition/Enum/RenderGraphEnum.h"
#include "Graphics/Render/Graph/Definition/Struct/RenderGraphStruct.h"
#include "Graphics/Render/Graph/Pass/RenderGraphPassBase.h"
#include "Graphics/Render/Graph/Pass/BackBuffer/Clear/RenderGraphBackBufferClearPass.h"
#include "Graphics/Render/Graph/Pass/BackBuffer/Present/RenderGraphBackBufferPresentPass.h"
#include "Graphics/Render/Graph/Converter/Json/RenderGraphJsonConverter.h"
#include "Graphics/Render/Graph/RenderGraph.h"

// レンダーラー
#include "Graphics/Render/Converter/Json/RendererJsonConverter.h"
#include "Graphics/Render/Renderer.h"

// グラフィックスマネージャー
#include "Graphics/Converter/Json/GraphicsManagerJsonConverter.h" 
#include "Graphics/GraphicsManager.h"