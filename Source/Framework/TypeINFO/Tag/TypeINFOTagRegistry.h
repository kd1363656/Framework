#pragma once

namespace FWK
{
    // タグ名と型情報を紐づけて保存するレジストリークラス
    // 例 : エディターで"Camera"カテゴリーを選んだ際に"Camera"タグで登録された型だけを一覧表示する
    class TypeINFOTagRegistry final : public Utility::SingletonBase<TypeINFOTagRegistry>
    {
    private:

        // タグ名は文字列リテラル(静的寿命)であることを前提にstd::string_viewをキーとして使用
        using TypeINFOList       = std::vector<const TypeINFORegistry::TypeINFO*>;
        using TagTypeINFOListMap = std::unordered_map<std::string_view, TypeINFOList, Struct::StringHash, std::equal_to<>>;

        friend class Utility::SingletonBase<TypeINFOTagRegistry>;

         TypeINFOTagRegistry()          = default;
        ~TypeINFOTagRegistry() override = default;

    public:

        void Register(const std::string_view& a_tagName, const TypeINFORegistry::TypeINFO& a_typeINFO);

        const TypeINFOList* FindPTRTypeINFOListByTag(const std::string_view& a_tagName) const;

        const auto& GetREFTagTypeINFOListMap() const { return m_tagTypeINFOListMap; }

    private:

        TagTypeINFOListMap m_tagTypeINFOListMap = {};
    };
}
