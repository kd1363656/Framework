#pragma once

namespace FWK
{
    class TaggedGameObjectComponentFactory final : public Utility::SingletonBase<TaggedGameObjectComponentFactory>
    {
    private:

        friend class Utility::SingletonBase<TaggedGameObjectComponentFactory>;

         TaggedGameObjectComponentFactory()          = default;
        ~TaggedGameObjectComponentFactory() override = default;

    public:

        template <typename DerivedType>
            requires Concept::IsDerivedBaseConcept<DerivedType, GameObjectComponentBase>
        void Register(const Enum::GameObjectComponentFactoryTag a_tag)
        {
            const auto& l_typeINFO = DerivedType::GetREFTypeINFO();

            // タグに準ずる型名を登録する
            // この型名を使用しコンポーネントをGenericFactoryから生成する
            m_taggedGameObjectComponentMap[a_tag].emplace_back(std::string{ l_typeINFO.k_name });
        }

        const auto& GetREFTaggedGameObjectComponentMap() const { return m_taggedGameObjectComponentMap; }

    private:
    
        std::unordered_map<Enum::GameObjectComponentFactoryTag, std::vector<std::string>> m_taggedGameObjectComponentMap = {};
    };
}