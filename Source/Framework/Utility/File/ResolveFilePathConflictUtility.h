#pragma once

namespace FWK::Utility
{
    // 指定したFilePathが既に存在する場合、
    // 番号を付与した一意なFilePathを返す
    // 例: "Player.png"が既に存在する場合 -> "Player1.png"を返す
    //     "Player1.png"も存在する場合    -> "Player2.png"を返す
    // 存在しない場合はそのまま返す
    inline std::filesystem::path ResolveFilePathConflict(const std::filesystem::path& a_desiredPath)
    {
        std::error_code l_errorCode = {};

        // 希望するPathが存在しないならそのまま返す
        if (!std::filesystem::exists(a_desiredPath, l_errorCode)) { return a_desiredPath; }

        // ファイル名のStemと拡張子を取得
        // 例: "Player.png" -> stem = "Player", extension = ".png"
        const auto& l_stem       = a_desiredPath.stem      ().string();
        const auto& l_extension  = a_desiredPath.extension ().string();
        const auto& l_parentPath = a_desiredPath.parent_path();

        // Player1, Player2... と番号を増やしながら存在をチェックする
        auto l_number = Constant::k_initialNameConflictResolveNumber;

        while (true)
        {
            // stem + 番号 + 拡張子を結合した新しいPathを作る
            const auto& l_candidatePath = l_parentPath / (std::format("{}{}{}", l_stem, l_number, l_extension));

            l_errorCode.clear();

            // ファイルパスが存在しなければ他の番号と被りが発生していないため
            // そのファイルパスを返す
            if (!std::filesystem::exists(l_candidatePath, l_errorCode)) { return l_candidatePath; }

            ++l_number;
        }
    }
}