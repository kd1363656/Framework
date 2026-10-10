#include "ModelRenderSystem.h"

// モデルの描画に使うテーブル(GPUElementTable)を、種類ごとにまとめて持つクラス
// 種類 : オブジェクト(行列) / メッシュ(バッファのSRVの番号) / マテリアル(種類ごと)
// 使う側(ModelComponentの描き方・マテリアル)は、FindVALTableでテーブルをweak_ptrで受け取り、
// 番号をもらって、値が変わったときだけ書く
// シェーダーは、ルート定数で受け取った番号でテーブルを引いて値を読む
// ※ 注意 : RendererはModelRenderSystemをコマンドキューより前に宣言している(テーブルの解放をGPUの完了後にするため)
void FWK::Graphics::ModelRenderSystem::Deserialize(const nlohmann::json& a_rootJson)
{}

bool FWK::Graphics::ModelRenderSystem::Create(const Device&                             a_device,
                                              const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                              const std::size_t&                        a_frameCount,
                                                    TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    return false;
}

void FWK::Graphics::ModelRenderSystem::RecordUpload(const DirectCommandList& a_directCommandList, const std::size_t& a_frameIndex) const
{}

nlohmann::json FWK::Graphics::ModelRenderSystem::Serialize() const
{
    return {};
}

FWK::Struct::RCModelTable FWK::Graphics::ModelRenderSystem::FetchVALRCModelTable() const
{
    return {};
}

FWK::TypeAlias::DescriptorIndex FWK::Graphics::ModelRenderSystem::FetchVALTableSRVDescriptorIndex(const Enum::ModelRenderTableType a_type) const
{
    return DescriptorHeap::k_invalidDescriptorIndex;
}

std::weak_ptr<FWK::Graphics::GPUElementTable> FWK::Graphics::ModelRenderSystem::FindVALTable(const Enum::ModelRenderTableType a_type) const
{
    return {};
}

UINT FWK::Graphics::ModelRenderSystem::FetchVALElementByteStride(const Enum::ModelRenderTableType a_type) const
{
    return k_invalidElementByteStride;
}

UINT FWK::Graphics::ModelRenderSystem::FetchVALCapacity(const Enum::ModelRenderTableType a_type) const
{
    return Struct::ModelRenderTableSetting::k_defaultCapacity;
}