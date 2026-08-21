#include "AssetFilePathHelperInspector.h"

void FWK::Utility::AssetFilePathHelperInspector::EditInspector(AssetFilePathHelper& a_assetFilePathHelper) const
{
	std::filesystem::path l_droppedFilePath = {};

    // 毎回PrefabInspector処理に入ったらダーティーフラグを下げておく
    a_assetFilePathHelper.SetIsFilePathChangedDirty(false);

    // Assetがまだ設定されていなければ案内文字列を表示する
    const auto&       l_currentAssetFilePath = a_assetFilePathHelper.GetREFAssetFilePath();
          std::string l_dropAreaLabel        = k_assetFilePathDropAreaEmptyText.data    ();

    if (!l_currentAssetFilePath.empty())
    {
        l_dropAreaLabel = l_currentAssetFilePath.generic_string();
    }

    l_dropAreaLabel.append(k_assetFilePathDragDropAreaID.data(), k_assetFilePathDragDropAreaID.size());

    // DetailsWindowの横幅が変更された場合にも追従できるように、
    // 現在利用可能な横幅をそのままDrop領域として使用する
    float l_dropAreaWidth = ImGui::GetContentRegionAvail().x;

    if (l_dropAreaWidth < k_assetFilePathDropAreaMINWidth)
    {
        l_dropAreaWidth = k_assetFilePathDropAreaMINWidth;
    }

    const ImVec2& l_dropAreaSize = { l_dropAreaWidth, k_assetFilePathDropAreaHeight };

    // DragDropTargetは「直前に描画されたImGuiItem」を
    // Drop対象として使用する
    // そのため最初にButtonを描画してこのButton全体をAssetのDrop領域として登録する
    ImGui::Button(l_dropAreaLabel.c_str(), l_dropAreaSize);
       
    // 直前に描画されたImGuiItemへFilePathがDropされた場合だけ、
    // DropされたFilePathを受け取る
    if (auto& l_imguiDragDropPayloadStorage = Utility::IMGUIDragDropPayloadStorage::GetInstance();
        !l_imguiDragDropPayloadStorage.DragDropTarget(Constant::k_assetFilePathDragAndDropPayloadLabel, l_droppedFilePath)) 
    {
        return; 
    }

    if (l_droppedFilePath == l_currentAssetFilePath) { return; }

    // FilePathの最終的な妥当性確認はApplyAssetFilePathへ集約する
    if (!a_assetFilePathHelper.ApplyAssetFilePath(l_droppedFilePath)) 
    {
        FWK_ADD_LOG(Constant::k_debugWarningColor, "ファイルパスがFBX形式のファイルを示しておらず読み込めませんでした。\nFilePath : {}", a_assetFilePathHelper.GetREFAssetFilePath().string());

        return;
    }

    // アセットファイルパスの適用が完了したらダーティーフラグを上げる
    a_assetFilePathHelper.SetIsFilePathChangedDirty(true);
}