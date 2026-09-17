#include "AssetBrowserEditorWindowAssetPaneBreadcrumb.h"

void FWK::Editor::AssetBrowserEditorWindowAssetPaneBreadcrumb::Draw(AssetBrowserEditorWindow& a_editorWindow)
{
    // 現在参照中のフォルダパスを取得
    // Window側のm_currentSelectFolderPathを参照
    // 空の場合はAssetルートデフォルトとする
    const auto& l_currentSelectFolderPath = a_editorWindow.GetREFCurrentSelectFolderPath();
    const auto& l_breadcrumbPath          = l_currentSelectFolderPath.empty() ? Constant::k_assetRootFolderPath : l_currentSelectFolderPath;

    // 絶対パスで書くのされている場合に備えてカレントディレクトリからの相対パスへ変換
    // std::filesystem::pathのイテレータは絶対パスの場合
    // ルート名(C:)の次にルートディレクトリ(/)をから文字列の要素として返す
    // proximateで"C:/.../Asset/" -> "Asset/"に変換する
          std::error_code       l_errorCode   = {};
    const std::filesystem::path l_displayPath = std::filesystem::proximate(l_breadcrumbPath, std::filesystem::current_path(), l_errorCode);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Constant::k_imguiStrongDarkColor);

    // パンくずリストを格納する子領域を作成
    // ImGui::BeginChild(ラベル(),
    //                   領域サイズ(枠0 = 最大サイズ、高さ0 = AutoResizeYで決定))、
    //                   ボーダー一括描画有無、
    //                   ImGuiChildFlags_AutoResizeYで高さを内容に自動合わせ
    //                   ImGuiWindowFlags_HorizontalScrollbarで幅超過時に水平スクロール
    // 戻り値がfalseの場合は霊異記が描画されていないためEndChildしてreturn
    if (const ImVec2& l_size = { Constant::k_imguiRemainingSize.x, ImGui::GetFrameHeight() };
        !ImGui::BeginChild(k_childLabel.data(), 
                           Constant::k_imguiRemainingSize, 
                           false,
                           ImGuiChildFlags_AutoResizeY |
                           ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::EndChild     ();
        ImGui::PopStyleColor();

        return;
    }

    // パスを改造ごとに分割して描画
    // std::filesystem::pathのイテレータで改造の要素を順に取得
    // 例 : Asset / Data / Soundの場合
    // 1回目 : l_pathElement = "Asset"
    // 2回目 : l_pathElement = "Data"
    // 3回目 : l_pathElement = "Sound"
    // 累積パスを構築しながら(表示名、絶対パス)を扱う
    // l_accumulatedPath : Asset -> Asset / Data -> Asset/Data/Sound)
    std::filesystem::path l_accumulatedPath = {};
    bool                  l_isFirstSegment  = true;

    for (const auto& l_pathElement : l_displayPath)
    {
        // 蓄積パスに要素を追加
        l_accumulatedPath /= l_pathElement;

        // 2件目以降は区切り文字を同じ行に描画
        // ImGui::SameLine : 次のアイテムを現在の行に続けて配置
        // 1件目は区切り文字不要なのでスキップ
        if (!l_isFirstSegment)
        {
            // 区切り文字" / "をテキストとして描画
            // ImGui::TextUnformatted : フォーマットなしテキスト描画
            ImGui::SameLine       ();
            ImGui::TextUnformatted(k_separatorLabel.data());
            ImGui::SameLine       ();
        }

        l_isFirstSegment = false;

        // セグメント名を取得
        // l_pathElementはpathの要素一つ分(フォルダ名)
        // generic_string()でstd::stringへ変換
        const auto& l_segmentName     = l_pathElement.generic_string();
        const auto& l_selectableLabel = l_segmentName + "##" + l_accumulatedPath.generic_string();

        // テキストサイズを取得
        const ImVec2& l_selectableSize = { ImGui::CalcTextSize(l_segmentName.c_str()).x, Constant::k_imguiRemainingSize.y };

        // クリック可能なセグメントを描画
        // ImGui::Selectable(ラベル、
        //                   選択状態ハイライト有無、
        //                   特別なフラグなし);
        // ホバー時(カーソルが当たっている部分)に明るいグレーでハイライトするため
        // ImGuiCol_HeaderHovered色をPushしてから描画
        // ImGui::PushStyleColor  : 色をスタックへ積む
        // ImGuiCol_HeaderHovered : ホバー時の背景色
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Constant::k_imguiLightGrayColor);

        // Selectable描画
        ImGui::Selectable(l_selectableLabel.c_str(),
                          false, 
                          ImGuiSelectableFlags_None,
                          l_selectableSize);

        // Pushした色をPop(一つ分)
        ImGui::PopStyleColor();

        // セグメントが左クリックされ場合
        // その階層の絶対パス(l_accumulatedPath)を現在参照中のフォルダパスに設定
        // ImGui::IsItemClicked  : 直前に描画したアイテムがクリックされたか
        // ImGuiMouseButton_Left : 左クリック
        // 現在参照中のフォルダパスがAsset / Dataになる
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            a_editorWindow.SetCurrentSelectFolderPath(l_accumulatedPath);
        }
    }

    ImGui::EndChild     ();
    ImGui::PopStyleColor();
}