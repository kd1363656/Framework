#include "AssetFilePathInspector.h"
#include "../../../Application/Application.h"

bool FWK::AssetFilePathInspector::EditInspector(AssetFilePath& a_assetFilePath) const
{
    // 同じウィンドウにAssetFilePathが複数並んでも(BaseColor / Normalなど)、
    // ボタンのIDが衝突しないように、このAssetFilePathのアドレスでIDを分ける
    ImGui::PushID(&a_assetFilePath);

    // Registryからパスを取得できればそのパス、取得できなければ「不明なパス」をボタンに表示する
    // ボタンの幅はウィンドウの残り幅いっぱいにする(幅が0以下にならないよう最小値を設ける)
    const auto& l_filePath    = a_assetFilePath.FetchVALFilePath();
    const auto& l_buttonLabel = FetchVALButtonText              (l_filePath) + k_assetFilePathDragDropAreaID.data();
    const float l_buttonWidth = std::max                        (ImGui::GetContentRegionAvail().x, k_assetFilePathDropAreaMINWidth);

    ImGui::Button(l_buttonLabel.c_str(), ImVec2{ l_buttonWidth, k_assetFilePathDropAreaHeight });

    // 直前に描いたボタンを、アセットブラウザーからのドロップ先にする
    // アセットブラウザーは選択中のファイルのパスをまとめてstd::vectorで渡してくる
    auto&                              l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();
    std::vector<std::filesystem::path> l_droppedFilePathList         = {};
    bool                               l_isFilePathChanged           = false;

    // 複数のファイルがドロップされた場合は、どれを使えばよいか決められないため受け取らない
    if (l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_imguiAssetBrowserFolderDragAndDropPayloadLabel, l_droppedFilePathList) &&
        l_droppedFilePathList.size() == k_singleDroppedFileCount)
    {
        l_isFilePathChanged = ApplyDroppedFilePath(l_droppedFilePathList.front(), a_assetFilePath);
    }

    ImGui::PopID();

    return l_isFilePathChanged;
}

void FWK::AssetFilePathInspector::RegisterTextureFilePath(const std::filesystem::path& a_filePath, AssetFilePathRegistry& a_assetFilePathRegistry) const
{
    // PNG以外のファイルはテクスチャとして登録しない
    if (!Utility::CanLoadFilePath(a_filePath, Constant::k_lowerPNGExtension)) { return; }

    // 新しいUUIDを発行し、ファイルパスと結び付けてRegistryへ登録する
    // 登録後は、ファイルの名前変更や移動があってもWatcherがRegistry側のパスを書き換えるため、
    // 同じUUIDから同じファイルを辿れる
          auto& l_uuidManager = Utility::UUIDManager::GetInstance();
    const auto& l_assetUUID   = l_uuidManager.GenerateVALUUID    ();

    a_assetFilePathRegistry.Add(a_filePath, l_assetUUID, Enum::AssetFilePathType::Texture);
}

bool FWK::AssetFilePathInspector::ApplyDroppedFilePath(const std::filesystem::path& a_droppedFilePath, AssetFilePath& a_assetFilePath) const
{
    auto& l_application           = Application::GetInstance                        ();
    auto& l_assetFilePathRegistry = l_application.GetMutableREFAssetFilePathRegistry();

    // テクスチャ(PNG)は、ドロップされた時点でRegistryに無ければ登録する
    // Prefab / Sceneは作成したときに登録されているため、ここでは登録しない
    if (a_assetFilePath.GetVALAllowedType() == Enum::AssetFilePathType::Texture &&
        !l_assetFilePathRegistry.FindPTRAssetUUID(a_droppedFilePath))
    {
        RegisterTextureFilePath(a_droppedFilePath, l_assetFilePathRegistry);
    }

    // Registryに登録されていないファイル(フォルダやFBXなど)は受け取らない
    const auto* l_assetUUID = l_assetFilePathRegistry.FindPTRAssetUUID(a_droppedFilePath);

    if (!l_assetUUID) { return false; }

    // 望む種類(AssetFilePathType)でないファイルのUUIDは受け取らない
    if (const auto* l_assetFilePathData = l_assetFilePathRegistry.FindPTRAssetFilePathData(*l_assetUUID);
        !l_assetFilePathData ||
        l_assetFilePathData->m_type != a_assetFilePath.GetVALAllowedType())
    {
        return false;
    }

    // 既に同じファイルを受け取っているなら、変更はない
    if (*l_assetUUID == a_assetFilePath.GetREFAssetFilePathUUID()) { return false; }

    a_assetFilePath.SetAssetFilePathUUID(*l_assetUUID);

    return true;
}

std::string FWK::AssetFilePathInspector::FetchVALButtonText(const std::filesystem::path& a_filePath) const
{
    if (a_filePath.empty()) { return std::string{ k_unknownFilePathText }; }

    // Assetフォルダからの相対パスにして表示する
    // 例 : Asset/Texture/HumanSkin.png → Texture/HumanSkin.png
    const auto& l_relativeFilePath = a_filePath.lexically_relative(Constant::k_assetRootFolderPath);

    // 日本語のファイル名も正しく表示できるよう、UTF-8へ変換してImGuiへ渡す
    // generic_wstringで区切り文字を「/」に揃える(Registryには「\」と「/」が混ざっているため)
    return Utility::WStringToString(l_relativeFilePath.generic_wstring());
}