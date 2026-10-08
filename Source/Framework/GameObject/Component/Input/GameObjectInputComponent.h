#pragma once

namespace FWK
{
    class GameObjectInputComponent final : public GameObjectComponentBase
    {
    public:

         GameObjectInputComponent() = default;
        ~GameObjectInputComponent() = default;

        void Deserialize(const nlohmann::json& a_rootJson) override;

        nlohmann::json Serialize() const override;

        std::shared_ptr<GameObjectComponentBase> Clone() const override;

    private:

    };
}