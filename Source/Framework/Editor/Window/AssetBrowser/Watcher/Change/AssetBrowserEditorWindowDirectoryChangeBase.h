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

		void SetIsDirectory(const bool a_set) { m_isDirectory = a_set; }

	protected:

		const auto& GetREFFilePath() const { return m_filePath; }

		bool GetVALISDirectory() const { return m_isDirectory; }

		static bool IsChildFilePath(const std::filesystem::path& a_filePath, const std::filesystem::path& a_parentDirectoryPath)
		{
			if (a_filePath.empty() ||
				a_parentDirectoryPath.empty())
			{
				return false;
			}

			auto l_filePathITR            = a_filePath.begin           ();
			auto l_parentDirectoryPathITR = a_parentDirectoryPath.begin();

			while (l_parentDirectoryPathITR != a_parentDirectoryPath.end())
			{
				if (l_filePathITR == a_filePath.end() ||
					*l_filePathITR != *l_parentDirectoryPathITR)
				{
					return false; 
				}

				++l_parentDirectoryPathITR;
				++l_filePathITR;
			}

			// Parentと完全一致でなく
			// Parentより後にもPathComponentが存在すれば子孫
			return l_filePathITR != a_filePath.end();
		}

	private:

		std::filesystem::path m_filePath = {};

		bool m_isDirectory = false;
	};
}