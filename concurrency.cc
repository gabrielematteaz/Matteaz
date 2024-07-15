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

	bool mutex::try_lock() noexcept
	{
		return TryAcquireSRWLockExclusive(&lock_);
	}
}