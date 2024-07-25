#pragma once

#include < Windows.h >

namespace matteaz
{
	struct _shared { };

	extern _shared shared;

	class mutex
	{
		SRWLOCK lock_;

	public:
		mutex(const mutex &) = delete;
		mutex &operator = (const mutex &) = delete;
		void lock() noexcept;
		void unlock() noexcept;
		[[nodiscard]] bool try_lock() noexcept;
		void lock(const _shared &) noexcept;
		void unlock(const _shared &) noexcept;
		[[nodiscard]] bool try_lock(const _shared &) noexcept;

		constexpr mutex() noexcept :
			lock_(SRWLOCK_INIT)
		{

		}
	};

	class exclusive_lock
	{
		mutex &mutex_;

	public:
		using mutex_type = mutex;

		exclusive_lock(mutex &mutex) noexcept;
		exclusive_lock(const exclusive_lock &) = delete;
		~exclusive_lock();
		exclusive_lock &operator = (const exclusive_lock &) = delete;
	};

	class shared_lock
	{
		mutex &mutex_;

	public:
		using mutex_type = mutex;

		shared_lock(mutex &mutex) noexcept;
		shared_lock(const shared_lock &) = delete;
		~shared_lock();
		shared_lock &operator = (const shared_lock &) = delete;
	};
}