#pragma once

namespace FWK::Utility
{
	inline nlohmann::json LoadJsonFile(const std::filesystem::path& a_filePath)
	{
		// 読み込めないファイルならreturn
		if (!CanLoadFilePath(a_filePath, Constant::k_lowerJsonExtension)) { return {}; }

		// ifstreamからjsonを読み込む
		std::ifstream l_ifs{ a_filePath };

		// 読み込みに失敗したらreturn
		if (l_ifs.fail()) { return {}; }

		const auto& l_loadedJson = nlohmann::json::parse(l_ifs, nullptr, false);

		// jsonオブジェクトがパース失敗などで無効状態になっているかを確認し
		// 無効状態なら空のjsonを返す
		if (l_loadedJson.is_discarded()) { return {}; }

		return l_loadedJson;
	}

	inline bool SaveJsonFile(const nlohmann::json& a_json, const std::filesystem::path& a_filePath)
	{
		// 拡張子が".json"でなければreturn
		if (a_filePath.extension() != Constant::k_lowerJsonExtension) { return false; }

		// ファイルパの親パスを取得
		const auto& l_parentDirectoryPath = a_filePath.parent_path();

		// 保存先フォルダーが存在しない場合は、
		// 親フォルダーを含めて作成する
		if (!l_parentDirectoryPath.empty())
		{
			std::error_code l_errorCode = {};

			std::filesystem::create_directories(l_parentDirectoryPath, l_errorCode);
			 
			if (l_errorCode) { return false; }
		}

		std::ofstream l_ofs{ a_filePath, std::ios::out };

		if (!l_ofs.is_open()) { return false; }
		
		// ファイルパスにあるjsonにjsonデータを保存
		l_ofs << a_json.dump(Constant::k_jsonIndentCount);
		l_ofs.close         ();

		return !l_ofs.fail();
	}
}