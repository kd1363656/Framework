#pragma once

namespace FWK
{
    class GameObjectComponentBase;
}

namespace FWK::Editor
{
    // 詳細ウィンドウの「Add Component」でコンポーネントを追加した操作を、Undo / Redoで戻す / やり直すためのコマンド
    // GameObjectはSceneInstanceUUIDで覚えておき、実行するたびにシーンから探す(再生 / 停止でシーンが作り直されても使えるようにするため)
    class AddGameObjectComponentCommand final : public ICommand
    {
    public:

         AddGameObjectComponentCommand(const boost::uuids::uuid& a_gameObjectUUID, const std::shared_ptr<GameObjectComponentBase>& a_component);
        ~AddGameObjectComponentCommand() override;

        void Undo() override;
        void Redo() override;

    private:

        // UndoでGameObjectから取り外しても実体(設定した値)が消えないよう、コマンド側でも所有する
        std::shared_ptr<GameObjectComponentBase> m_component;

        boost::uuids::uuid m_gameObjectUUID;
    };
}