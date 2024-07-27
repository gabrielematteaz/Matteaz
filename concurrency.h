#pragma once

#include < Windows.h >

namespace matteaz
{
	class mutex
	{
		SRWLOCK lock_;

	public:
		using native_handle_type = PSRWLOCK;

		mutex(const mutex &) = delete;
		mutex &operator = (const mutex &) = delete;
		void lock() noexcept;
		void unlock() noexcept;
		[[nodiscard]] bool try_lock() noexcept;
		void lock_shared() noexcept;
		void unlock_shared() noexcept;
		[[nodiscard]] bool try_lock_shared() noexcept;

		constexpr mutex() noexcept :
			lock_(SRWLOCK_INIT)
		{

		}

		[[nodiscard]] constexpr native_handle_type native_handle() noexcept
		{
			return &lock_;
		}
	};
}