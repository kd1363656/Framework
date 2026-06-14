#pragma once

namespace FWK::Graphics
{
	template <typename ConstantBufferType>
	class CachedPassConstantBufferDrawRequestBase : public DrawRequestPassBase
	{
	public:

		 CachedPassConstantBufferDrawRequestBase()		    = default;
		~CachedPassConstantBufferDrawRequestBase() override = default;

		void SetSourceConstantBuffer(const std::shared_ptr<ConstantBufferType>& a_set) { m_sourceConstantBuffer = a_set; }

	protected:

		void UpdateConstantBuffer()
		{
			const auto& l_source = m_sourceConstantBuffer.lock();

			if (!l_source) { return; }

			m_constantBuffer = *l_source;
		}

		const auto& GetREFConstantBuffer() const { return m_constantBuffer; }

	private:

		std::weak_ptr<ConstantBufferType> m_sourceConstantBuffer = {};

		ConstantBufferType m_constantBuffer = {};
	};
}