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

        void Load(NextScene& a_nextScene, nlohmann::json&& a_rootJson) const;

    private:

        static constexpr std::string_view k_nextSceneJsonKey = "NextScene";
    };
}