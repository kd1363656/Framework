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

	inline void SaveJsonFile(const nlohmann::json& a_json, const std::filesystem::path& a_filePath)
	{
		// 拡張子が".json"でなければreturn
		if (a_filePath.extension() != Constant::k_lowerJsonExtension) { return; }

		std::ofstream l_ofs(a_filePath, std::ios::out);

		// ファイルパスにあるjsonにjsonデータを保存
		l_ofs << a_json.dump(Constant::k_jsonIndentCount);
		l_ofs.close         ();
	}
}