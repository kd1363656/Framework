#include "Material.h"

FWK::Graphics::Material::Material() :
    m_material (),
    m_storageID(Constant::k_invalidStorageID)
{}
FWK::Graphics::Material::Material(const Material& a_other) :
    m_material (a_other.m_material),
    m_storageID(a_other.m_storageID)
{
    AddReferenceCount();
}
FWK::Graphics::Material::Material(Material&& a_other) noexcept :
    m_material (std::move(a_other.m_material)),
    m_storageID(a_other.m_storageID)
{
    a_other.m_storageID = Constant::k_invalidStorageID;
    a_other.m_material.reset();
}
FWK::Graphics::Material::~Material()
{
    SubtractReferenceCount();
}
FWK::Graphics::Material& FWK::Graphics::Material::operator=(const Material& a_other)
{
    if (this == &a_other) { return *this; }

    // 所持しているMaterialの参照数を減らす
    SubtractReferenceCount();

    // コピー元と同じMaterialを参照
    m_storageID = a_other.m_storageID;
    m_material  = a_other.m_material;

    // 新しいMaterialの参照数の加算
    AddReferenceCount();

    return *this;
}
FWK::Graphics::Material& FWK::Graphics::Material::operator=(Material&& a_other) noexcept
{
    if (this == &a_other) { return *this; }

    // 所持しているMaterialの参照数を減らす
    SubtractReferenceCount();

    // ムーブでは参照数を増やさず、参照先だけ移す
    m_storageID = a_other.m_storageID;
    m_material  = std::move(a_other.m_material);

    // 参照元のStorageIDを無効化
    a_other.m_storageID = Constant::k_invalidStorageID;
    a_other.m_material.reset();

    return *this;
}

bool FWK::Graphics::Material::Load(const std::filesystem::path& a_filePath)
{
    // 既に別のStorageIDを持っている場合は先に参照を外す
    SubtractReferenceCount();

          auto& l_graphicsManager         = GraphicsManager::GetInstance                          ();
          auto& l_resourceContext         = l_graphicsManager.GetMutableREFResourceContext        ();
          auto& l_materialSystem          = l_resourceContext.GetMutableREFMaterialSystem         ();
    const auto& l_device                  = l_graphicsManager.GetREFDevice                        ();
    const auto& l_gpuMemoryAllocator      = l_resourceContext.GetREFGPUMemoryAllocator            ();
          auto& l_cbvSRVUAVDescriptorPool = l_resourceContext.GetMutableREFCBVSRVUAVDescriptorPool();

    Struct::MaterialLoadResult l_materialLoadResult = {};

    // .materialファイルを持たないMeshはデフォルトマテリアルを使う
    if (a_filePath.empty())
    {
        l_materialLoadResult = l_materialSystem.FetchDefaultMaterialForBatchUpload(l_device, l_gpuMemoryAllocator, l_cbvSRVUAVDescriptorPool);
    }
    else
    {
        l_materialLoadResult = l_materialSystem.LoadMaterialForBatchUpload(l_device,
                                                                           l_gpuMemoryAllocator,
                                                                           a_filePath,
                                                                           l_cbvSRVUAVDescriptorPool);
    }

    FWK_ASSERT_RETURN_VALUE_IF(l_materialLoadResult.m_material.expired(), "Materialの読み込みに失敗しました。", false);

    m_storageID = l_materialLoadResult.m_storageID;
    m_material  = l_materialLoadResult.m_material;

    return true;
}

bool FWK::Graphics::Material::IsValid() const
{
    return !m_material.expired();
}

void FWK::Graphics::Material::AddReferenceCount() const
{
    // デフォルトマテリアルは参照数で管理しない
    if (m_storageID == Constant::k_invalidStorageID) { return; }

    auto& l_graphicsManager = GraphicsManager::GetInstance                  ();
    auto& l_resourceContext = l_graphicsManager.GetMutableREFResourceContext();
    auto& l_materialSystem  = l_resourceContext.GetMutableREFMaterialSystem ();

    FWK_ASSERT_RETURN_IF(!l_materialSystem.AddMaterialReferenceCount(m_material), "Materialの参照数加算に失敗しました。");
}

void FWK::Graphics::Material::SubtractReferenceCount()
{
    if (m_storageID == Constant::k_invalidStorageID)
    {
        m_material.reset();

        return;
    }

          auto& l_graphicsManager    = GraphicsManager::GetInstance        ();
    const auto& l_renderer           = l_graphicsManager.GetREFRenderer    ();
    const auto& l_directCommandQueue = l_renderer.GetREFDirectCommandQueue();

    auto& l_resourceContext        = l_graphicsManager.GetMutableREFResourceContext       ();
    auto& l_materialSystem         = l_resourceContext.GetMutableREFMaterialSystem        ();
    auto& l_resourceReleaseContext = l_resourceContext.GetMutableREFResourceReleaseContext();

    FWK_ASSERT_RETURN_IF(!l_materialSystem.SubtractMaterialReferenceCount(m_material, l_directCommandQueue, l_resourceReleaseContext), "Materialの参照数減算に失敗しました。");

    m_storageID = Constant::k_invalidStorageID;
    m_material.reset();
}