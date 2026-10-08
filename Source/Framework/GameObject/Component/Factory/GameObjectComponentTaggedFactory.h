#pragma once

namespace FWK
{
    class GameObjectComponentTaggedFactory final : public Utility::SingletonBase<GameObjectComponentTaggedFactory>
    {
    private:

        friend class Utility::SingletonBase<GameObjectComponentTaggedFactory>;

         GameObjectComponentTaggedFactory()          = default;
        ~GameObjectComponentTaggedFactory() override = default;

    public:

        template <typename DerivedType>
            requires Concept::IsDerivedBaseConcept<DerivedType, GameObjectComponentBase>
        void Register(const std::string_view& a_tag)
        {
            const auto& l_typeINFO = DerivedType::GetREFTypeINFO();

            // タグに準ずる型名を登録する
            // この型名を使用しコンポーネントをGenericFactoryから生成する
            m_taggedGameObjectComponentMap[a_tag].emplace_back(std::string{ l_typeINFO.k_name });
        }

        const auto& GetREFTaggedGameObjectComponentMap() const { return m_taggedGameObjectComponentMap; }

    private:

        std::unordered_map<std::string_view, std::vector<std::string>> m_taggedGameObjectComponentMap = {};
    };
}