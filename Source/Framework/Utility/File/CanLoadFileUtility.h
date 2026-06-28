#pragma once

namespace FWK::Utility
{
	// ファイルが読み込める形式かどうかを確認する。
	inline bool CanLoadFilePath(const std::filesystem::path& a_filePath)
	{
		if (a_filePath.empty())							   { return false; }
		if (!std::filesystem::exists(a_filePath))		   { return false; }
		if (!std::filesystem::is_regular_file(a_filePath)) { return false; }

		return true;
	}

	// ファイルが読み込める形式かどうか、拡張子が一致しているかどうかを確認する
	inline bool CanLoadFilePath(const std::filesystem::path& a_filePath, const std::filesystem::path& a_extension)
	{
		if (!CanLoadFilePath(a_filePath))		   { return false; }
		if (a_filePath.extension() != a_extension) { return false; }

		return true;
	}

	// ファイルパスの拡張子を指定された拡張子に変える
	inline std::filesystem::path CreateFilePathByReplaceExtension(const std::filesystem::path& a_filePath, const std::filesystem::path& a_extension)
	{
		auto l_filePath = a_filePath;

		// 指定された拡張子に置き換える
		l_filePath.replace_extension(a_extension);

		return l_filePath;
	}
}