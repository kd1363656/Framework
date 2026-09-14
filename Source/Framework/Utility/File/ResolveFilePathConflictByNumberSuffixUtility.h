#pragma once

namespace FWK::Utility
{
    // 指定したFilePathが既に存在する場合、
    // 番号を付与した一意なFilePathを返す
    // 例: "Player.png"が既に存在する場合 -> "Player1.png"を返す
    //     "Player1.png"も存在する場合    -> "Player2.png"を返す
    // 存在しない場合はそのまま返す
    inline std::filesystem::path ResolveFilePathConflictByNumberSuffix(const std::filesystem::path& a_desiredPath)
    {
        std::error_code l_errorCode = {};

        // 希望するPathが存在しないならそのまま返す
        if (!std::filesystem::exists(a_desiredPath, l_errorCode)) { return a_desiredPath; }

        // ファイル名のStemと拡張子を取得
        // 例: "Player.png" -> stem = "Player", extension = ".png"
        const auto& l_stem       = a_desiredPath.stem      ().string();
        const auto& l_extension  = a_desiredPath.extension ().string();
        const auto& l_parentPath = a_desiredPath.parent_path();

        // Stemの末尾に数値がついている場合は、その数字を取り出して
        // 次の番号から検索を開始する
        // 例 : "Player1" -> baseName = "Player", StartNumber = 2
        // 例 : "Player"  -> baseName = "Player", StartNumber = 1
        // これにより複数の複製で"Player1"にならず"Player2"になる
        std::string l_baseName    = l_stem;
        auto        l_startNumber = Constant::k_initialNumberSuffixForFilePathConflict;

        // Stemの末尾から数字部分を探す
        // 末尾が数字でない場合はbaseName = stem,startNumber = 1のまま
        if (!l_stem.empty())
        {
            auto l_digitBegin = l_stem.rbegin();
            
            // 末尾から数字である限り進む
            // std::isdigitで数字化を判定
            // unsigned_charへcastして渡す(符号付きcharの負値対策)
            while (l_digitBegin != l_stem.rend() &&
                std::isdigit(static_cast<unsigned char>(*l_digitBegin)))
            {
                ++l_digitBegin;
            }

            // 数字部分が見つかり、かつ数字の前に非数字文字列がある場合
            // (全体が数字(123)の場合は元のstemをbaseNameとして使う)
            // l_digitBeginが数字部分の先頭(gy買う順で見て)を指している
            // l_digitBeginがrendでなければ数字部分あり
            // l_digitBeginがrbeginでなければ数字の前に文字あり
            if (l_digitBegin != l_stem.rbegin() &&
                l_digitBegin != l_stem.rend())
            {
                // baseName = 数字部分を除いた前半
                // 逆順イテレータl_digitBeginは清純で見て数字部分の先頭を指す
                // l_digitBegin.base()で正規イテレータへ変換
                // 例 : "Player1"の"l_digitBegin"は'1'を示す(逆順)
                // .base()は'1'の次を示す(正順)
                // なのでbegin()からbase()の前までが"Player"
                l_baseName = std::string(l_stem.begin(), l_digitBegin.base());

                // 数字部分を数値へ変換して + 1
                // 例 : "1" -> "2", "11" -> "12"
                const auto& l_numberString = std::string(l_digitBegin.base(), l_stem.end());

                l_startNumber = std::stoull(l_numberString) + Constant::k_nextNumberSuffixOffsetForFilePathConflict;
            }
        }

        auto l_number = l_startNumber;

        while (true)
        {
            // baseName + 番号 + 拡張子を結合した新しいPathを作る
            const auto& l_candidatePath = l_parentPath / (std::format("{}{}{}", 
                                                          l_baseName,
                                                          l_number,
                                                          l_extension));

            l_errorCode.clear();

            // ファイルパスが存在しなければ他の番号と被りが発生していないため
            // そのファイルパスを返す
            if (!std::filesystem::exists(l_candidatePath, l_errorCode)) { return l_candidatePath; }

            ++l_number;
        }
    }
}