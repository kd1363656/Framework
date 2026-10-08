#pragma once

namespace FWK
{
    class NextScene final
    {
    public:

         NextScene() = default;
        ~NextScene() = default;

        void Load(const std::filesystem::path& a_filePath);

        void SetJson(nlohmann::json&& a_set) { m_json = std::move(a_set); }

        void SetName(const std::string& a_set) { m_name = a_set; }

        const auto& GetREFJson() const { return m_json; }

        const auto& GetREFName() const { return m_name; }

    private:

        nlohmann::json m_json = {};

        std::string m_name = {};

        Converter::NextSceneJsonConverter m_jsonConverter = {};
    };
}