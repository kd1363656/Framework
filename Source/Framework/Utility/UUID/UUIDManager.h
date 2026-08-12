#pragma once

namespace FWK
{
	class UUIDManager final : public Utility::SingletonBase<UUIDManager>
	{
	private:

		friend class SingletonBase<UUIDManager>;

		 UUIDManager() = default;
		~UUIDManager() = default;

	public:

		TypeAlias::UUID GenerateVALUUID();

	private:

		boost::uuids::random_generator m_randomGenerator = {};
	};
}