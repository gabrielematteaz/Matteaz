#pragma once

#include < Windows.h >

namespace matteaz
{
	class locker
	{
		SRWLOCK &lock_;

	public:
		locker(SRWLOCK &lock) noexcept;
		locker(const locker&) = delete;
		~locker();
		locker &operator = (const locker&) = delete;
	};
}