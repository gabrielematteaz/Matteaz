#include "concurrency.h"

namespace matteaz
{
	locker::locker(SRWLOCK &lock) noexcept :
		lock_(lock)
	{
		AcquireSRWLockExclusive(&lock);
	}

	locker::~locker()
	{
		ReleaseSRWLockExclusive(&lock_);
	}
}