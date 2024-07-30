#pragma once

#include < Windows.h >

namespace Matteaz
{
	class Mutex
	{
		SRWLOCK slimLock;

	public:
		using native_handle_type = PSRWLOCK;

		Mutex(const Mutex &) = delete;
		Mutex &operator = (const Mutex &) = delete;
		void lock() noexcept;
		void unlock() noexcept;
		[[nodiscard]] bool try_lock() noexcept;
		void lock_shared() noexcept;
		void unlock_shared() noexcept;
		[[nodiscard]] bool try_lock_shared() noexcept;

		constexpr Mutex() noexcept :
			slimLock(SRWLOCK_INIT)
		{

		}

		[[nodiscard]] constexpr native_handle_type native_handle() noexcept
		{
			return &slimLock;
		}
	};
}
