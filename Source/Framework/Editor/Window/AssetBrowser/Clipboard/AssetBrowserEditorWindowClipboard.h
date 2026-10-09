#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowClipboard final
    {
    public:

         AssetBrowserEditorWindowClipboard() = default;
        ~AssetBrowserEditorWindowClipboard() = default;

        void Clear();

        bool Has(const std::filesystem::path& a_filePath) const;

        bool IsEmpty() const;

        void Apply(const std::vector<std::filesystem::path>& a_filePathList, const Enum::AssetBrowserFileClipboardOperationType a_operationType);

        const auto& GetREFClipboardFilePathList() const { return m_clipboardFilePathList; }

        auto GetVALOperationType() const { return m_operationType; }

    private:

        std::unordered_set<std::filesystem::path> m_clipboardFilePathSet = {};

        std::vector<std::filesystem::path> m_clipboardFilePathList = {};

        Enum::AssetBrowserFileClipboardOperationType m_operationType = Enum::AssetBrowserFileClipboardOperationType::Invalid;
    };
}