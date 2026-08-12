#pragma once

namespace FWK
{
	class UUIDManager final : public Utility::SingletonBase<UUIDManager>
	{
	private:

		friend class Utility::SingletonBase<UUIDManager>;

		 UUIDManager() = default;
		~UUIDManager() = default;

	public:

		boost::uuids::uuid GenerateVALUUID();

	private:

		boost::uuids::random_generator m_randomGenerator = {};
	};
}