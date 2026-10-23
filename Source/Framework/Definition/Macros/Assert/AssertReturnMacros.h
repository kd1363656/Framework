#pragma once

// do-whileを使う理由はマクロにセミコロンを使用したときにセミコロンが二重定義
// されることを防ぎ警告が出ないようにするため、できるだけコードの一貫性を守るため

// 失敗を報告してからassertする
// assert()はReleaseビルド(NDEBUG定義時)では中身ごと消えるため、
// assertだけだとReleaseでは「何も言わずにreturnする」だけになってしまう
// そこでassertの前にFWK::AssertReporter::Report()でログを残す
// ・「同じ場所では1回だけ報告する」判定はAssertReporterが行う
//   (毎フレーム呼ばれる処理で失敗しても、ログが毎フレーム増え続けないようにするため)
// ・std::source_location::current()はこのマクロを書いた場所(ファイル名・行番号・関数名)を表す
#define FWK_ASSERT_REPORT(Message)                                                        \
do                                                                                        \
{                                                                                         \
    FWK::AssertReporter::GetInstance().Report(Message, std::source_location::current());  \
                                                                                          \
    assert(false && Message);                                                             \
}                                                                                         \
while (false)

#define FWK_ASSERT_RETURN(Message) \
do                                 \
{                                  \
    FWK_ASSERT_REPORT(Message);    \
                                   \
    return;                        \
}                                  \
while (false)

#define FWK_ASSERT_RETURN_VALUE(Message, ReturnValue) \
do                                                    \
{                                                     \
    FWK_ASSERT_REPORT(Message);                       \
                                                      \
    return ReturnValue;                               \
}                                                     \
while (false)

#define FWK_ASSERT_RETURN_IF(Conditions, Message) \
do                                                \
{                                                 \
    if (Conditions)                               \
    {                                             \
        FWK_ASSERT_REPORT(Message);               \
                                                  \
        return;                                   \
    }                                             \
}                                                 \
while (false)

#define FWK_ASSERT_RETURN_VALUE_IF(Conditions, Message, ReturnValue) \
do                                                                   \
{                                                                    \
    if (Conditions)                                                  \
    {                                                                \
        FWK_ASSERT_REPORT(Message);                                  \
                                                                     \
        return ReturnValue;                                          \
    }                                                                \
}                                                                    \
while (false)

// ループの中で「この要素だけ飛ばして次の要素へ進む」ためのマクロ
// FWK_ASSERT_RETURN_IFをループ内で使うと、1つの要素の失敗で
// 残りの要素がすべて処理されなくなるため、要素ごとに独立した処理ではこちらを使う
// ※他のマクロと違いdo-while(false)で囲っていない理由
//   continueはdo-while(false)の中で使うと「do-while自体を抜ける」だけになり、
//   外側のループの次の要素へ進まないため
// ※最後のelse {}は、後ろに書いたelseがマクロ内のifと結びついてしまう
//   (ぶら下がりelse)のを防ぐため
// ※使い方の注意 : if文の中で使うときは必ず{}で囲むこと
//   if (a) { FWK_ASSERT_CONTINUE_IF(...); } else { ... }   ← OK
//   if (a) FWK_ASSERT_CONTINUE_IF(...); else { ... }       ← コンパイルエラー(;でifが終わるため)
//   (間違えてもコンパイルエラーになるだけで、黙って違う動きになることはない)
#define FWK_ASSERT_CONTINUE_IF(Conditions, Message) \
if (Conditions)                                     \
{                                                   \
    FWK_ASSERT_REPORT(Message);                     \
                                                    \
    continue;                                       \
}                                                   \
else                                                \
{                                                   \
}