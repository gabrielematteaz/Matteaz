#include "concurrency.h"

namespace matteaz
{
	struct _shared shared;

	void mutex::lock() noexcept
	{
		AcquireSRWLockExclusive(&lock_);
	}

	void mutex::unlock() noexcept
	{
#pragma warning(suppress : 26110)
		ReleaseSRWLockExclusive(&lock_);
	}

	bool mutex::try_lock() noexcept
	{
		return TryAcquireSRWLockExclusive(&lock_);
	}

	void mutex::lock(const _shared &) noexcept
	{
		AcquireSRWLockShared(&lock_);
	}

	void mutex::unlock(const _shared &) noexcept
	{
#pragma warning(suppress : 26110)
		ReleaseSRWLockShared(&lock_);
	}

	bool mutex::try_lock(const _shared &) noexcept
	{
		return TryAcquireSRWLockShared(&lock_);
	}

	exclusive_lock::exclusive_lock(mutex &mutex) noexcept :
		mutex_(mutex)
	{
		mutex.lock();
	}

	exclusive_lock::~exclusive_lock()
	{
		mutex_.unlock();
	}

	shared_lock::shared_lock(mutex &mutex) noexcept :
		mutex_(mutex)
	{
		mutex.lock(shared);
	}

	shared_lock::~shared_lock()
	{
		mutex_.unlock(shared);
	}
}