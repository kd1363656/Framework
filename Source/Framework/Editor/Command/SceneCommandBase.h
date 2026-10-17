#pragma once

namespace FWK::Editor
{
    class SceneCommandBase : public ICommand
    {
    public:

         SceneCommandBase()          = default;
        ~SceneCommandBase() override = default;

        FWK_DEFINE_TYPE_INFO(SceneCommandBase, ICommand)
    };
}