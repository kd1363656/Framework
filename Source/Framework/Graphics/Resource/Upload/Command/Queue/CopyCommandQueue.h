#pragma once

namespace FWK::Graphics
{
	class CopyCommandQueue final : public CommandQueueBase
	{
	public:

		 CopyCommandQueue();
		~CopyCommandQueue() override;
	};
}