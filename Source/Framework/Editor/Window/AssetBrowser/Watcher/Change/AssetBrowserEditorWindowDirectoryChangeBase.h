#pragma once

namespace FWK::Editor
{
    class AssetBrowserEditorWindowDirectoryChangeBase
    {
    public:

                 AssetBrowserEditorWindowDirectoryChangeBase() = default;
        virtual ~AssetBrowserEditorWindowDirectoryChangeBase() = default;

        virtual void Apply(AssetFilePathRegistry& a_assetFilePathRegistry, SceneManager& a_sceneManager) = 0;

        void SetFilePath(const std::filesystem::path& a_set) { m_filePath = a_set; }

        void SetIsDirectory(const bool a_set) { m_isDirectory     = a_set; }

        bool GetVALIsRequiresRetry() const { return m_isRequiresRetry; }

    protected:

        static bool IsChildFilePath(const std::filesystem::path& a_filePath, const std::filesystem::path& a_parentDirectoryPath);

        void SetIsRequiresRetry(const bool a_set) { m_isRequiresRetry = a_set; }

        const auto& GetREFFilePath() const { return m_filePath; }

        bool GetVALIsDirectory() const { return m_isDirectory; }

    private:

        std::filesystem::path m_filePath = {};

        bool m_isDirectory     = false;
        bool m_isRequiresRetry = false;
    };
}