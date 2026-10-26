#pragma once

namespace FWK
{
    class GameObject;
}

namespace FWK::Editor
{
    class EditorGameObjectSelectionState;
}

namespace FWK::Editor
{
    // 選択中のGameObjectを、ImGuizmo(マウスで掴んで動かす矢印などの部品)で移動、回転、拡大するクラス
    //
    // 複数選択しているときは、選択した物の位置のXYZそれぞれの最小値と最大値の真ん中にギズモを出し、
    // ギズモの動きを全員に同じだけ掛けることで、お互いの位置関係を保ったまま動かす
    //
    // 仕組み :
    // 掴んだ瞬間のギズモの行列をP0、今のギズモの行列をP1とすると、
    // 「P0から見た位置関係を、そのままP1へ持っていく」行列は P0の逆行列 × P1 になる
    // 各GameObjectの掴んだ瞬間のワールド行列にこれを掛ければ、全員がギズモと一緒に動く
    class ViewportGizmo final
    {
    private:

        // 掴んだ瞬間のGameObject1つ分の状態
        struct ManipulateTarget final
        {
            std::weak_ptr<GameObject> m_gameObject = {};

            TypeAlias::Math::Matrix m_beginWorldMatrix = TypeAlias::Math::Matrix::Identity;

            Struct::Transform m_beginTransform = {};
        };

    public:

         ViewportGizmo() = default;
        ~ViewportGizmo() = default;

        void Draw(const EditorGameObjectSelectionState& a_gameObjectSelectionState,
                  const Struct::CBCameraPass&           a_drawCameraConstantBuffer,
                  const Struct::ViewportScreenRect&     a_viewportRect,
                  const Enum::ViewportGizmoOperation    a_gizmoOperation);

        // マウスがギズモの上にある、又はギズモを掴んでいるか(そのクリックでGameObjectを選び直さないようにするため)
        bool FetchVALIsOverOrUsing() const;

    private:

        void CollectTargetGameObjectList(const EditorGameObjectSelectionState& a_gameObjectSelectionState, std::vector<std::shared_ptr<GameObject>>& a_outGameObjectList) const;

        TypeAlias::Math::Matrix CalculatePivotMatrix(const std::vector<std::shared_ptr<GameObject>>& a_gameObjectList) const;

        void BeginManipulate(const std::vector<std::shared_ptr<GameObject>>& a_gameObjectList, const TypeAlias::Math::Matrix& a_beginPivotMatrix);
        void UpdateManipulate();
        void EndManipulate   ();

        void ApplyWorldMatrix(GameObject& a_gameObject, const TypeAlias::Math::Matrix& a_worldMatrix) const;

        bool IsSelectedAncestor(const GameObject&        a_gameObject, const EditorGameObjectSelectionState& a_gameObjectSelectionState) const;
        bool IsSameTransform   (const Struct::Transform& a_transformA, const Struct::Transform& a_transformB) const;

        static ImGuizmo::OPERATION ConvertImGuizmoOperation(const Enum::ViewportGizmoOperation a_gizmoOperation);

        static constexpr bool k_isOrthographic = false;

        std::vector<ManipulateTarget> m_manipulateTargetList = {};

        TypeAlias::Math::Matrix m_pivotMatrix      = TypeAlias::Math::Matrix::Identity;
        TypeAlias::Math::Matrix m_beginPivotMatrix = TypeAlias::Math::Matrix::Identity;

        bool m_isVisible      = false;
        bool m_isManipulating = false;
    };
}
