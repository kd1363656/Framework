#pragma once

namespace FWK::Graphics
{
    template <typename ConstantBufferType>
    class CachedPassConstantBufferDrawRequestBase : public DrawRequestPassBase
    {
    public:

         CachedPassConstantBufferDrawRequestBase()          = default;
        ~CachedPassConstantBufferDrawRequestBase() override = default;

        void SetSourceConstantBuffer(const std::shared_ptr<ConstantBufferType>& a_set) { m_sourceConstantBuffer = a_set; }

        // 登録元の定数バッファ(カメラなど)の参照
        // LOD選択などでカメラ情報を読み取るために公開している(読み取り専用)
        const auto& GetREFSourceConstantBuffer() const { return m_sourceConstantBuffer; }

    protected:

        void UpdateConstantBuffer()
        {
            const auto& l_source = m_sourceConstantBuffer.lock();

            if (!l_source) { return; }

            m_constantBuffer = *l_source;
        }

        const auto& GetREFConstantBuffer() const { return m_constantBuffer; }

        auto& GetMutableREFConstantBuffer() { return m_constantBuffer; }

    private:

        std::weak_ptr<ConstantBufferType> m_sourceConstantBuffer = {};

        ConstantBufferType m_constantBuffer = {};
    };
}