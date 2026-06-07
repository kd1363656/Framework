#pragma once

namespace FWK::Graphics
{
	class CopyCommandList final : public CommandListBase
	{
	public:

		 CopyCommandList();
		~CopyCommandList() override;
	};
}