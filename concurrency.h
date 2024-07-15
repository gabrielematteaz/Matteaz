#pragma once

#include < Windows.h >

namespace matteaz
{
	class mutex
	{
		SRWLOCK lock_;

	public:
		mutex(const mutex&) = delete;
		mutex &operator = (const mutex&) = delete;
		void lock() noexcept;
		void unlock() noexcept;
		bool try_lock() noexcept;

		constexpr mutex() noexcept :
			lock_ SRWLOCK_INIT
		{

		}
	};
}