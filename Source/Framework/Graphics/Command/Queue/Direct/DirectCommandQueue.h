#pragma once

namespace FWK::Graphics
{
	class DirectCommandQueue final : public CommandQueueBase
	{
	public:

		 DirectCommandQueue();
		~DirectCommandQueue() override;
	};
}