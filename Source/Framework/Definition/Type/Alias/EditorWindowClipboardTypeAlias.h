#pragma once

namespace FWK::TypeAlias
{
    using AssetBrowserEditorWindowClipboard  = Editor::EditorWindowClipboard<std::filesystem::path>;
    using WorldOutlinerEditorWindowClipboard = Editor::EditorWindowClipboard<boost::uuids::uuid>;
}