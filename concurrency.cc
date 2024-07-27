#include "concurrency.h"

namespace matteaz
{
	void mutex::lock() noexcept
	{
		AcquireSRWLockExclusive(&lock_);
	}

	void mutex::unlock() noexcept
	{
#pragma warning(suppress : 26110)
		ReleaseSRWLockExclusive(&lock_);
	}

	[[nodiscard]] bool mutex::try_lock() noexcept
	{
		return TryAcquireSRWLockExclusive(&lock_);
	}

	void mutex::lock_shared() noexcept
	{
		AcquireSRWLockShared(&lock_);
	}

	void mutex::unlock_shared() noexcept
	{
#pragma warning(suppress : 26110)
		ReleaseSRWLockShared(&lock_);
	}

	[[nodiscard]] bool mutex::try_lock_shared() noexcept
	{
		return TryAcquireSRWLockShared(&lock_);
	}
}