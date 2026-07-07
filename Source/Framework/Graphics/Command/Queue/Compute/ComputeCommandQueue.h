#pragma once

namespace FWK::Graphics
{
	class ComputeCommandQueue final : public CommandQueueBase
	{
	public:

		 ComputeCommandQueue();
		~ComputeCommandQueue() override;
	};
}