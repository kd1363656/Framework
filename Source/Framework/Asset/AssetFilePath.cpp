#include "AssetFilePath.h"
#include "../../Application/Application.h"

void FWK::AssetFilePath::Deserialize(const nlohmann::json& a_rootJson)
{
    if (a_rootJson.is_null()) { return; }

    m_jsonConverter.Deserialize(a_rootJson, *this);
}

bool FWK::AssetFilePath::EditInspector()
{
    return m_inspector.EditInspector(*this);
}

nlohmann::json FWK::AssetFilePath::Serialize() const
{
    return m_jsonConverter.Serialize(*this);
}

std::filesystem::path FWK::AssetFilePath::FetchVALFilePath() const
{
    // UUIDが未設定なら、まだ何もファイルを受け取っていない
    if (m_assetFilePathUUID.is_nil()) { return {}; }

    // ApplicationのRegistryから、UUIDに結び付いたファイルの情報を探す
    // パスそのものではなくUUIDを持つことで、ファイルの名前変更や移動があっても
    // Watcherが書き換えたRegistry側の最新のパスを辿れる
    const auto& l_application           = Application::GetInstance                        ();
    const auto& l_assetFilePathRegistry = l_application.GetREFAssetFilePathRegistry       ();
    const auto* l_assetFilePathData     = l_assetFilePathRegistry.FindPTRAssetFilePathData(m_assetFilePathUUID);

    // ファイルが削除されてRegistryから外れている場合は、空のパスを返す
    if (!l_assetFilePathData) { return {}; }

    // 登録されている種類が、このAssetFilePathで受け取る種類と違うなら使わない
    if (l_assetFilePathData->m_type != m_allowedType) { return {}; }

    return l_assetFilePathData->m_assetFilePath;
}