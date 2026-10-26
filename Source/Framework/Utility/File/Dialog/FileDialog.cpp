#include "FileDialog.h"

bool FWK::Utility::FileDialog::RequestOpen(const Struct::FileDialogDesc& a_desc)
{
    // FOS_FILEMUSTEXIST -> 存在するファイルしか選べないようにする
    return Request(CLSID_FileOpenDialog, FOS_FILEMUSTEXIST, a_desc);
}
bool FWK::Utility::FileDialog::RequestSave(const Struct::FileDialogDesc& a_desc)
{
    // FOS_OVERWRITEPROMPT -> 既存ファイルを指定した場合、「上書きしますか？」という確認をWindows側に表示する
    return Request(CLSID_FileSaveDialog, FOS_OVERWRITEPROMPT, a_desc);
}

bool FWK::Utility::FileDialog::TryFetchResultFilePath(std::filesystem::path& a_outFilePath)
{
    // ダイアログを表示していない
    if (!m_resultFilePathFuture.valid()) { return false; }

    // wait_for(0秒) : 待たずに、別スレッドの処理が終わっているか(ダイアログが閉じられたか)だけを調べる
    // まだダイアログが開いているなら、次のフレームでもう一度確認する
    if (m_resultFilePathFuture.wait_for(std::chrono::seconds::zero()) != std::future_status::ready) { return false; }

    // get()で別スレッドの戻り値(選ばれたファイルパス)を受け取る
    // get()を呼ぶとfutureは空(valid() == false)になるため、IsShowing()もfalseに戻る
    a_outFilePath = m_resultFilePathFuture.get();

    // キャンセルされた場合は空のファイルパスが返ってくる
    return !a_outFilePath.empty();
}

bool FWK::Utility::FileDialog::Request(const CLSID& a_dialogCLSID, const FILEOPENDIALOGOPTIONS a_addDialogOptions, const Struct::FileDialogDesc& a_desc)
{
    // 既にダイアログを表示している場合は、二つ目を開かない
    if (m_resultFilePathFuture.valid()) { return false; }

    // std::async(std::launch::async, 関数) : 関数を新しいスレッドで実行し、その戻り値をstd::futureで受け取れるようにする
    // a_descは呼び出し元のローカル変数の可能性があるため、参照ではなくコピーしてスレッドへ渡す
    m_resultFilePathFuture = std::async(std::launch::async,
                                        [a_dialogCLSID, a_addDialogOptions, a_desc]
                                        {
                                            return Show(a_dialogCLSID, a_addDialogOptions, a_desc);
                                        });

    return true;
}

std::filesystem::path FWK::Utility::FileDialog::Show(const CLSID& a_dialogCLSID, const FILEOPENDIALOGOPTIONS a_addDialogOptions, const Struct::FileDialogDesc& a_desc)
{
    // このスレッドのCOMをSTAで初期化する(ファイル選択ダイアログの決まり)
    // COINIT_DISABLE_OLE1DDE -> 古いOLE1(DDE)の仕組みを使わないようにする(Microsoftがダイアログ用に推奨している指定)
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) { return {}; }

    // ShowDialog内のComPtrはShowDialogを抜けるときに解放される
    // COMのオブジェクトはCoUninitializeより前に解放しなければならないため、関数を分けている
    const auto& l_filePath = ShowDialog(a_dialogCLSID, a_addDialogOptions, a_desc);

    CoUninitialize();

    return l_filePath;
}
std::filesystem::path FWK::Utility::FileDialog::ShowDialog(const CLSID& a_dialogCLSID, const FILEOPENDIALOGOPTIONS a_addDialogOptions, const Struct::FileDialogDesc& a_desc)
{
    // Windowsの新しいファイル選択ダイアログを生成する
    // CLSID_FileOpenDialog -> 既存ファイルを開くためのダイアログ
    // CLSID_FileSaveDialog -> 新しいファイル名を入力して保存するためのダイアログ
    // どちらもIFileDialogを継承しているので、共通の設定はIFileDialogとして行える
    TypeAlias::ComPtr<IFileDialog> l_fileDialog = nullptr;

    if (FAILED(CoCreateInstance(a_dialogCLSID,
                                nullptr,
                                CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&l_fileDialog))))
    {
        return {};
    }

    // ダイアログ上部に表示するタイトルを設定する
    if (FAILED(l_fileDialog->SetTitle(a_desc.m_title.c_str()))) { return {}; }

    // 選択可能なファイル形式を設定する
    // 例えば
    // m_filterName    = L"JSON File(*.json)"
    // m_filterPattern = L"*.json"とした場合、
    // ダイアログにはJSONファイルだけが表示される
    const std::array<COMDLG_FILTERSPEC, Constant::k_fileDialogFilterCount> l_fileTypeList = { { { a_desc.m_filterName.c_str(), a_desc.m_filterPattern.c_str() } } };

    if (FAILED(l_fileDialog->SetFileTypes(static_cast<UINT>(l_fileTypeList.size()), l_fileTypeList.data()))) { return {}; }

    // ユーザーがTestSceneとだけ入力した場合に、
    // TestScene.jsonのように拡張子を自動的につける
    if (FAILED(l_fileDialog->SetDefaultExtension(a_desc.m_defaultExtension.c_str()))) { return {}; }

    // 現在設定されているダイアログOptionを取得する
    FILEOPENDIALOGOPTIONS l_dialogOptions = {};

    if (FAILED(l_fileDialog->GetOptions(&l_dialogOptions))) { return {}; }

    // FOS_FORCEFILESYSTEM -> 実際のWindowsファイルシステム上のPathだけを対象にする
    // FOS_PATHMUSTEXIST   -> 保存先・読み込み元のFolderが存在している必要がある
    // FOS_NOCHANGEDIR     -> ダイアログでフォルダを移動しても、アプリのカレントディレクトリを変えない
    //                        (このアプリは"Asset/..."のようにカレントディレクトリからの相対パスでファイルを読むため、
    //                         変わってしまうとすべてのファイル読み込みが失敗する)
    l_dialogOptions |= FOS_FORCEFILESYSTEM |
                       FOS_PATHMUSTEXIST   |
                       FOS_NOCHANGEDIR     |
                       a_addDialogOptions;

    if (FAILED(l_fileDialog->SetOptions(l_dialogOptions))) { return {}; }

    // ダイアログを最初に表示するFolderを設定する
    // 設定に失敗してもダイアログ自体は表示できるため、ここでは失敗してもreturnしない
    if (std::error_code l_errorCode = {};
        !a_desc.m_initialDirectoryPath.empty() &&
        std::filesystem::is_directory(a_desc.m_initialDirectoryPath, l_errorCode))
    {
        const auto& l_initialDirectoryPath = std::filesystem::absolute(a_desc.m_initialDirectoryPath, l_errorCode);

        // std::filesystem::pathを
        // WindowsShellが扱うIShellItemへ変換してからダイアログへ渡す
        if (TypeAlias::ComPtr<IShellItem> l_initialDirectoryItem = nullptr;
            !l_errorCode &&
            SUCCEEDED(SHCreateItemFromParsingName(l_initialDirectoryPath.c_str(), nullptr, IID_PPV_ARGS(&l_initialDirectoryItem))))
        {
            l_fileDialog->SetFolder(l_initialDirectoryItem.Get());
        }
    }

    // 実際にダイアログを表示する
    // ユーザーが閉じるまでこのスレッドはここで止まる(メインスレッドは止まらない)
    // ユーザーがキャンセルした場合もFAILEDとなるため、その場合は空のパスを返す
    if (FAILED(l_fileDialog->Show(a_desc.m_ownerHWND))) { return {}; }

    // ユーザーがダイアログで選択した結果を取得する
    // IShellItemは、WindowsShell上のファイルやFolderを表すオブジェクト
    TypeAlias::ComPtr<IShellItem> l_selectedItem = nullptr;

    if (FAILED(l_fileDialog->GetResult(&l_selectedItem))) { return {}; }

    // 選択されたIShellItemから、
    // 実際のWindowsファイルシステム上のPathを取得する
    PWSTR l_selectedFilePath = nullptr;

    if (FAILED(l_selectedItem->GetDisplayName(SIGDN_FILESYSPATH, &l_selectedFilePath))) { return {}; }

    // GetDisplayName()で取得したPWSTRは
    // Windows側が確保したメモリなので、
    // 最後にCoTaskMemFree()で解放する必要がある
    // ただし解放する前にstd::filesystem::pathへコピーする
    const std::filesystem::path l_filePath = l_selectedFilePath;

    CoTaskMemFree(l_selectedFilePath);

          std::error_code l_errorCode   = {};
    const auto&           l_currentPath = std::filesystem::current_path(l_errorCode);

    if (l_errorCode) { return {}; }

    // AssetFilePathRegistryなどと同じ形にそろえるため、カレントディレクトリからの相対パスにする
    // 例 : C:/GitHub/Framework/Framework/Asset/Data/Scene/Title.json -> Asset/Data/Scene/Title.json
    const auto& l_relativeFilePath = std::filesystem::relative(l_filePath, l_currentPath, l_errorCode);

    if (l_errorCode) { return {}; }

    // generic_wstringで区切り文字を'/'にそろえる(Registryに保存されているパスと同じ書き方にする)
    // stringではなくwstringを使うのは、日本語などのファイル名でも文字化けさせないため
    return std::filesystem::path{ l_relativeFilePath.generic_wstring() };
}