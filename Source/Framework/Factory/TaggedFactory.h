#pragma once

namespace FWK
{
    template <typename BaseType>
    class TaggedFactory final : public Utility::SingletonBase<TaggedFactory<BaseType>>
    {
    private:

        friend class Utility::SingletonBase<TaggedFactory<BaseType>>;

         TaggedFactory()          = default;
        ~TaggedFactory() override = default;

    public:

        // タグ(カテゴリー)ごとに、BaseTypeを継承したクラスの型の名前を登録する
        // 何を作るか(コンポーネント・ノードなど)はBaseTypeで分かれ、BaseTypeごとに別のシングルトンになる
        // 例 : TaggedFactory<GameObjectComponentBase>とTaggedFactory<NodeBase>は別々の一覧を持つ
        // 実体はGenericFactoryが型の名前から作るため、ここでは名前だけを持つ
        template <typename DerivedType>
            requires Concept::IsDerivedBaseConcept<DerivedType, BaseType>
        void Register(const std::string_view& a_tag)
        {
            // 初めてのタグなら、今の一覧の数(= 次に追加する添字)を覚える
            // 既にあるタグなら、try_emplaceは何もせずに覚えている添字を返す
            // 添字で覚えるのは、std::vectorは要素が増えると引っ越し、要素のアドレスが変わるため
            // 例 : 一覧が空のときに"カメラ"を登録 → { "カメラ" : 0 }
            const auto& [l_tagGroupIndexITR, l_isNewTag] = m_tagGroupIndexMap.try_emplace(a_tag, m_tagGroupList.size());

            if (l_isNewTag)
            {
                auto& l_newTagGroup = m_tagGroupList.emplace_back();

                l_newTagGroup.m_tag = a_tag;
            }

            // 型の名前はTypeINFOの文字列リテラルを指しているため、std::string_viewのまま持っても消えない
            const auto& l_typeINFO = DerivedType::GetREFTypeINFO     ();
                  auto& l_tagGroup = m_tagGroupList[l_tagGroupIndexITR->second];

            l_tagGroup.m_typeNameList.emplace_back(l_typeINFO.k_name);
        }

        const auto& GetREFTagGroupList() const { return m_tagGroupList; }

    private:

        std::vector<Struct::TaggedFactoryTagGroup>        m_tagGroupList     = {};
        std::unordered_map<std::string_view, std::size_t> m_tagGroupIndexMap = {};
    };
}