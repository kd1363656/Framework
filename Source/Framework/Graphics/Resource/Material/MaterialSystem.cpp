#include "MaterialSystem.h"

void FWK::Graphics::MaterialSystem::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}
bool FWK::Graphics::MaterialSystem::Create()
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_materialStorage.Create(), "AssetStorageの作成に失敗したため、MaterialSystemの作成処理に失敗しました。", false);

    return true;
}

FWK::Struct::MaterialLoadResult FWK::Graphics::MaterialSystem::LoadMaterialForBatchUpload(const Device&                             a_device,
                                                                                          const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                                                                          const std::filesystem::path&              a_filePath,
                                                                                                TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    Struct::MaterialLoadResult l_materialLoadResult = {};

    // 同じファイルを別の書き方で指定されても同じMaterialを使うように、パスを整える
    const auto l_filePath = a_filePath.lexically_normal();

    // 成功したらキャッシュ内容が入っているのでreturn
    if (TryResolveCachedMaterialResult(l_filePath, l_materialLoadResult)) { return l_materialLoadResult; }

    auto l_material = CreateMaterialFromBinaryFile(l_filePath);

    // .materialが読み込めない場合でも描画できるように、デフォルトマテリアルを使う
    if (!l_material)
    {
        FWK_ADD_LOG(Constant::k_imguiDebugWarningColor, "[Material]\nFilePath : {}\n.materialファイルを読み込めないため、デフォルトマテリアルを使用します。\n", Utility::WStringToString(l_filePath.wstring()));

        return FetchDefaultMaterialForBatchUpload(a_device, a_gpuMemoryAllocator, a_cbvSRVUAVDescriptorPool);
    }

    const auto l_allocateStorageID = m_materialStorage.AllocateStorageID();

    FWK_ASSERT_RETURN_VALUE_IF(l_allocateStorageID == Constant::k_invalidStorageID, "StorageIDの割り当てに失敗したため、Material読み込み処理に失敗しました。", l_materialLoadResult);

    // テクスチャは.materialファイルの場所を基準に読み込むため、先にファイルパスを設定する
    l_material->SetFilePath      (l_filePath.wstring());
    l_material->SetStorageID     (l_allocateStorageID);
    l_material->SetReferenceCount(Constant::k_initialAssetReferenceCount);

    l_material->BuildRuntimeResources();

    Struct::MaterialBatchUploadRecord l_materialBatchUploadRecord = {};

    l_materialBatchUploadRecord.m_material = l_material;

    if (!l_material->CreateGPUDataBuffer(a_device,
                                         a_gpuMemoryAllocator,
                                         l_materialBatchUploadRecord.m_bufferUploadCommandList,
                                         a_cbvSRVUAVDescriptorPool))
    {
        m_materialStorage.ReleaseStorageID(l_allocateStorageID);

        FWK_ASSERT_RETURN_VALUE("MaterialのGPUDataBufferの作成に失敗したため、Material読み込み処理に失敗しました。", l_materialLoadResult);
    }

    m_materialList.emplace_back(l_material);

    l_materialLoadResult.m_material  = l_material;
    l_materialLoadResult.m_storageID = l_allocateStorageID;

    // バッチアップロード情報をマップに格納
    m_pendingMaterialBatchUploadRecordMap.try_emplace(l_filePath.wstring(), std::move(l_materialBatchUploadRecord));

    return l_materialLoadResult;
}

FWK::Struct::MaterialLoadResult FWK::Graphics::MaterialSystem::FetchDefaultMaterialForBatchUpload(const Device& a_device, const GPUMemoryAllocator& a_gpuMemoryAllocator, TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool)
{
    Struct::MaterialLoadResult l_materialLoadResult = {};

    // 初めて使われる時に1つだけ作り、以降は全てのMeshで共有する
    if (!m_defaultMaterial)
    {
        auto l_defaultMaterial = std::make_shared<StandardLitMaterial>();

        // テクスチャのパスが空のため、全てデフォルトテクスチャになる
        l_defaultMaterial->BuildRuntimeResources();

        FWK_ASSERT_RETURN_VALUE_IF(!l_defaultMaterial->CreateGPUDataBuffer(a_device,
                                                                           a_gpuMemoryAllocator,
                                                                           m_pendingBufferUploadCommandList,
                                                                           a_cbvSRVUAVDescriptorPool),
                                                                           "デフォルトマテリアルのGPUDataBufferの作成に失敗しました。",
                                                                           l_materialLoadResult);

        m_materialList.emplace_back(l_defaultMaterial);

        m_defaultMaterial = l_defaultMaterial;
    }

    // デフォルトマテリアルはMaterialSystemが持ち続けるため、参照数で管理しない(StorageIDは無効値のまま)
    l_materialLoadResult.m_material = m_defaultMaterial;

    return l_materialLoadResult;
}

nlohmann::json FWK::Graphics::MaterialSystem::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

void FWK::Graphics::MaterialSystem::RebuildDirtyMaterialGPUDataBuffers(const Device&                             a_device,
                                                                       const GPUMemoryAllocator&                 a_gpuMemoryAllocator,
                                                                       const TypeAlias::DirectCommandQueue&      a_directCommandQueue,
                                                                             TypeAlias::CBVSRVUAVDescriptorPool& a_cbvSRVUAVDescriptorPool,
                                                                             ResourceReleaseContext&             a_resourceReleaseContext)
{
    // 解放済みのMaterialを取り除く
    std::erase_if(m_materialList, [](const std::weak_ptr<MaterialBase>& a_material) { return a_material.expired(); });

    const auto& l_lastSignaledFenceValue = a_directCommandQueue.FetchREFLastSignaledFenceValue();

    for (const auto& l_materialElement : m_materialList)
    {
        const auto& l_material = l_materialElement.lock();

        if (!l_material)                   { continue; }
        if (!l_material->IsGPUDataDirty()) { continue; }

        l_material->SetIsGPUDataDirty(false);

        // GPUが前のフレームで使っている可能性があるため、今のバッファは遅延解放へ積んでから作り直す
        // ※ 作り直すとSRVDescriptorIndexが変わるが、描画側は毎フレームMaterialから取得し直すため問題ない
        FWK_ASSERT_CONTINUE_IF(!l_material->ReserveRelease(l_lastSignaledFenceValue, a_resourceReleaseContext), "MaterialのGPUDataBufferの遅延解放登録に失敗しました。");

        FWK_ASSERT_CONTINUE_IF(!l_material->CreateGPUDataBuffer(a_device,
                                                                a_gpuMemoryAllocator,
                                                                m_pendingBufferUploadCommandList,
                                                                a_cbvSRVUAVDescriptorPool),
                                                                "MaterialのGPUDataBufferの作り直しに失敗しました。");
    }
}

void FWK::Graphics::MaterialSystem::RegisterPendingMaterials()
{
    for (const auto& [l_filePath, l_pendingMaterialBatchUploadRecord] : m_pendingMaterialBatchUploadRecordMap)
    {
        const auto& l_material = l_pendingMaterialBatchUploadRecord.m_material;

        FWK_ASSERT_CONTINUE_IF(!l_material,                                               "Materialが無効のため、バッチマテリアル登録に失敗しました。");
        FWK_ASSERT_CONTINUE_IF(!m_materialStorage.RegisterRecord(l_material, l_filePath), "Materialの登録に失敗したため、バッチマテリアル登録に失敗しました。");
    }

    // そのフレーム内でアップロードすべきMaterialを全てアップロードし終えた状態なのでクリア
    m_pendingMaterialBatchUploadRecordMap.clear();
    m_pendingBufferUploadCommandList.clear     ();
}

bool FWK::Graphics::MaterialSystem::AddMaterialReferenceCount(const std::weak_ptr<MaterialBase>& a_material)
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_materialStorage.AddReferenceCount(a_material), "AssetStorageでの参照数加算に失敗したため、Material参照数加算に失敗しました。", false);

    return true;
}
bool FWK::Graphics::MaterialSystem::SubtractMaterialReferenceCount(const std::weak_ptr<MaterialBase>& a_material, const TypeAlias::DirectCommandQueue& a_directCommandQueue, ResourceReleaseContext& a_resourceReleaseContext)
{
    FWK_ASSERT_RETURN_VALUE_IF(!m_materialStorage.SubtractReferenceCount(a_material, a_directCommandQueue, a_resourceReleaseContext), "AssetStorageでの参照数減算に失敗したため、Material参照数減算に失敗しました。", false);

    return true;
}

std::shared_ptr<FWK::Graphics::MaterialBase> FWK::Graphics::MaterialSystem::CreateMaterialFromBinaryFile(const std::filesystem::path& a_filePath) const
{
    Converter::MaterialBinaryConverter l_materialBinaryConverter = {};

    std::string l_typeName = {};

    if (!l_materialBinaryConverter.TryReadTypeName(a_filePath, l_typeName)) { return nullptr; }

    const auto& l_materialFactory = TypeAlias::MaterialSharedFactory::GetInstance();

    // 型名から派生クラスを作る(型を知らなくても、作られたMaterial自身が自分のデータを読み込む)
    auto l_material = l_materialFactory.Create(l_typeName);

    if (!l_material)                            { return nullptr; }
    if (!l_material->LoadBinaryData(a_filePath)) { return nullptr; }

    return l_material;
}

bool FWK::Graphics::MaterialSystem::TryResolveCachedMaterialResult(const std::filesystem::path& a_filePath, Struct::MaterialLoadResult& a_materialLoadResult)
{
    const auto& l_filePath = a_filePath.wstring();

    // 既に登録済みのMaterialなら再度ロード申請する必要がないのでreturn
    if (const auto& l_material = m_materialStorage.FindVALRecord(l_filePath).lock())
    {
        FWK_ASSERT_RETURN_VALUE_IF(!AddMaterialReferenceCount(l_material), "登録済みMaterialの参照数加算に失敗したため、Material読み込み処理に失敗しました。", false);

        a_materialLoadResult.m_material  = l_material;
        a_materialLoadResult.m_storageID = l_material->GetVALStorageID();

        return true;
    }

    // 既にPending中のMaterialなら再度ロード申請する必要がないのでreturn
    if (const auto& l_itr = m_pendingMaterialBatchUploadRecordMap.find(l_filePath);
        l_itr != m_pendingMaterialBatchUploadRecordMap.end())
    {
        const auto& l_material = l_itr->second.m_material;

        FWK_ASSERT_RETURN_VALUE_IF(!l_material, "Pending中のMaterialが無効のため、Material読み込み処理に失敗しました。", false);

        l_material->AddReferenceCount();

        a_materialLoadResult.m_material  = l_material;
        a_materialLoadResult.m_storageID = l_material->GetVALStorageID();

        return true;
    }

    return false;
}