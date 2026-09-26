#pragma once



namespace FWK
{
    class GameObjectComponentContainer final
    {
    public:
    
         GameObjectComponentContainer() = default;
        ~GameObjectComponentContainer() = default;
    
        void INIT();

        void Deserialize(const nlohmann::json& a_rootJson);

        void PostDeserialize();

        void EarlyUpdate   () const;
        void Update        () const;
        void LateUpdate    () const;
        void PostLateUpdate() const;

        void EditInspector() const;

        nlohmann::json Serialize() const;

    private:
   
        Converter::GameObjectComponentContainerJsonConverter m_jsonConverter = {};
    };
}