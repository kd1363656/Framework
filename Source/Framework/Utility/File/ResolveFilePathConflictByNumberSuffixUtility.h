#pragma once

namespace FWK::Utility
{
    // 指定したFilePathが既に存在する場合、
    // Unityと同じ「名前 (番号)」の形で番号を付与した一意なFilePathを返す
    // 例: "Player.png"が既に存在する場合     -> "Player (1).png"を返す
    //     "Player (1).png"も存在する場合     -> "Player (2).png"を返す
    //     "K02.png"が既に存在する場合        -> "K02 (1).png"を返す(名前の末尾の数字は番号として扱わない)
    // 存在しない場合はそのまま返す
    inline std::filesystem::path ResolveFilePathConflictByNumberSuffix(const std::filesystem::path& a_desiredPath)
    {
        std::error_code l_errorCode = {};

        // 希望するPathが存在しないならそのまま返す
        if (!std::filesystem::exists(a_desiredPath, l_errorCode)) { return a_desiredPath; }

        // ファイル名のStemと拡張子を取得
        // 例: "Player.png" -> stem = "Player", extension = ".png"
        const auto& l_stem       = a_desiredPath.stem       ().string();
        const auto& l_extension  = a_desiredPath.extension  ().string();
        const auto& l_parentPath = a_desiredPath.parent_path();

        // Stemの末尾が" (数字)"の形になっている場合だけ、その数字を番号として取り出して
        // 次の番号から検索を開始する
        // 例 : "Player (1)" -> baseName = "Player", StartNumber = 2
        // 例 : "Player"     -> baseName = "Player", StartNumber = 1
        // 例 : "K02"        -> baseName = "K02",    StartNumber = 1
        // これにより複数の複製で"Player (1)"にならず"Player (2)"になり、
        // 名前の一部として付けた数字("K02"の"02")が番号として書き換えられることもない
        std::string l_baseName    = l_stem;
        auto        l_startNumber = Constant::k_initialNumberSuffixForFilePathConflict;

        // 最後に出てくる" ("の位置を探す
        // 末尾が")"で終わっていない場合は番号付きの名前ではない
        if (const auto& l_openPosition = l_stem.rfind(Constant::k_numberSuffixOpenStringForFilePathConflict);
            l_openPosition != std::string::npos &&
            l_stem.ends_with(Constant::k_numberSuffixCloseStringForFilePathConflict))
        {
            // " ("と")"に挟まれた部分を取り出す
            // 例 : "Player (12)" -> "12"
            const auto& l_numberBegin  = l_openPosition + Constant::k_numberSuffixOpenStringForFilePathConflict.size();
            const auto& l_numberLength = l_stem.size                                                                () - l_numberBegin - Constant::k_numberSuffixCloseStringForFilePathConflict.size();
            const auto& l_numberString = l_stem.substr                                                              (l_numberBegin, l_numberLength);

            // 括弧の中がすべて数字の場合だけ番号として扱う
            // "Player (abc)"のような名前は、名前全体をbaseNameとして扱う
            // std::isdigitで数字かを判定し、unsigned_charへcastして渡す(符号付きcharの負値対策)
            const bool l_isNumberSuffix = !l_numberString.empty() &&
                                          std::ranges::all_of(l_numberString,
                                                              [](const char a_character)
                                                              {
                                                                  return static_cast<bool>(std::isdigit(static_cast<unsigned char>(a_character)));
                                                              });

            if (l_isNumberSuffix)
            {
                // baseName = " (数字)"を除いた前半
                // 例 : "Player (1)" -> "Player"
                l_baseName.resize(l_openPosition);

                // 括弧の中の数字を数値へ変換して + 1
                // 例 : "1" -> "2", "11" -> "12"
                l_startNumber = std::stoull(l_numberString) + Constant::k_nextNumberSuffixOffsetForFilePathConflict;
            }
        }

        auto l_number = l_startNumber;

        while (true)
        {
            // baseName + " (" + 番号 + ")" + 拡張子を結合した新しいPathを作る
            const auto& l_candidatePath = l_parentPath / (std::format("{}{}{}{}{}",
                                                          l_baseName,
                                                          Constant::k_numberSuffixOpenStringForFilePathConflict,
                                                          l_number,
                                                          Constant::k_numberSuffixCloseStringForFilePathConflict,
                                                          l_extension));

            l_errorCode.clear();

            // ファイルパスが存在しなければ他の番号と被りが発生していないため
            // そのファイルパスを返す
            if (!std::filesystem::exists(l_candidatePath, l_errorCode)) { return l_candidatePath; }

            ++l_number;
        }
    }
}