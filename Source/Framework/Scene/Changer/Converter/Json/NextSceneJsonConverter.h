#pragma once

namespace FWK
{
    class NextScene;
}

namespace FWK::Converter
{
    class NextSceneJsonConverter final
    {
    public:

         NextSceneJsonConverter() = default;
        ~NextSceneJsonConverter() = default;

        static bool Rename(const std::filesystem::path& a_oldFilePath, const std::filesystem::path& a_newFilePath, const std::string& a_newName);

        void Load(nlohmann::json&& a_rootJson, NextScene& a_nextScene) const;

    private:

        static constexpr std::string_view k_nextSceneJsonKey = "NextScene";
    };
}