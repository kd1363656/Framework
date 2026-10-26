#pragma once

namespace FWK::Utility
{
    // ワールド座標を、カメラのクリップ座標(x, y, z, w)へ変換する
    // wはカメラから見た奥行き(距離)にあたり、0以下ならカメラの後ろにある
    inline TypeAlias::Math::Vector4 TransformViewportWorldToClip(const TypeAlias::Math::Vector3& a_worldPosition, const TypeAlias::Math::Matrix& a_viewProjectionMatrix)
    {
        const TypeAlias::Math::Vector4 l_worldPosition = { a_worldPosition.x, a_worldPosition.y, a_worldPosition.z, Constant::k_viewportProjectionPositionW };

        return TypeAlias::Math::Vector4::Transform(l_worldPosition, a_viewProjectionMatrix);
    }

    // クリップ座標を、ビューポートの画面上の座標(ピクセル)へ変換する
    // クリップ座標をwで割ると、画面の左下が(-1, -1)、右上が(1, 1)の座標(NDC)になる
    // それを左上が(0, 0)、右下が(1, 1)の割合に直し、ビューポートの大きさを掛ける(画面のyは下向きなので上下を反転する)
    inline ImVec2 ConvertViewportClipToScreen(const TypeAlias::Math::Vector4& a_clipPosition, const Struct::ViewportScreenRect& a_viewportRect)
    {
        const float l_ndcX = a_clipPosition.x / a_clipPosition.w;
        const float l_ndcY = a_clipPosition.y / a_clipPosition.w;

        const float l_uvX = l_ndcX * Constant::k_viewportProjectionNDCToUVScale + Constant::k_viewportProjectionNDCToUVOffset;
        const float l_uvY = Constant::k_viewportProjectionNDCToUVOffset - l_ndcY * Constant::k_viewportProjectionNDCToUVScale;

        return
        {
            a_viewportRect.m_min.x + l_uvX * a_viewportRect.m_size.x,
            a_viewportRect.m_min.y + l_uvY * a_viewportRect.m_size.y
        };
    }

    // ワールド座標を、ビューポートの画面上の座標へ変換する(カメラの後ろにある点はfalseを返す)
    // ビューポート上にアイコンを描く、マウスで選ぶ、などで使う
    inline bool ProjectViewportWorldToScreen(const TypeAlias::Math::Vector3&  a_worldPosition,
                                             const TypeAlias::Math::Matrix&   a_viewProjectionMatrix,
                                             const Struct::ViewportScreenRect& a_viewportRect,
                                                   ImVec2&                     a_outScreenPosition)
    {
        const auto& l_clipPosition = TransformViewportWorldToClip(a_worldPosition, a_viewProjectionMatrix);

        if (l_clipPosition.w <= Constant::k_viewportProjectionMinClipW) { return false; }

        a_outScreenPosition = ConvertViewportClipToScreen(l_clipPosition, a_viewportRect);

        return true;
    }
}
