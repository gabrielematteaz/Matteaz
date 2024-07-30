#include "Concurrency.h"

namespace Matteaz
{
	void Mutex::lock() noexcept
	{
		AcquireSRWLockExclusive(&slimLock);
	}

	void Mutex::unlock() noexcept
	{
#pragma warning(suppress : 26110)
		ReleaseSRWLockExclusive(&slimLock);
	}

	[[nodiscard]] bool Mutex::try_lock() noexcept
	{
		return TryAcquireSRWLockExclusive(&slimLock);
	}

	void Mutex::lock_shared() noexcept
	{
		AcquireSRWLockShared(&slimLock);
	}

	void Mutex::unlock_shared() noexcept
	{
#pragma warning(suppress : 26110)
		ReleaseSRWLockShared(&slimLock);
	}

	[[nodiscard]] bool Mutex::try_lock_shared() noexcept
	{
		return TryAcquireSRWLockShared(&slimLock);
	}
}
