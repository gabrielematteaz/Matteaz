#pragma once

#include <Windows.h>

namespace Matteaz
{
	class SafeHandle
	{
		HANDLE handle;

	public:
		SafeHandle(const SafeHandle&) = delete;
		SafeHandle& operator = (const SafeHandle&&) = delete;

		constexpr explicit SafeHandle(HANDLE handle = INVALID_HANDLE_VALUE) noexcept :
			handle(handle)
		{

		}

		constexpr SafeHandle(SafeHandle&& safeHandle) noexcept :
			handle(safeHandle.handle)
		{
			safeHandle.handle = INVALID_HANDLE_VALUE;
		}

		~SafeHandle()
		{
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
		}

		SafeHandle& operator = (SafeHandle&& safeHandle) noexcept
		{
			if (handle != safeHandle.handle)
			{
				CloseHandle(handle);
				handle = safeHandle.handle;
				safeHandle.handle = INVALID_HANDLE_VALUE;
			}

			return *this;
		}

		[[nodiscard]] constexpr HANDLE Get() const noexcept
		{
			return handle;
		}

		[[nodiscard]] constexpr HANDLE Release() noexcept
		{
			HANDLE handle = this->handle;

			this->handle = INVALID_HANDLE_VALUE;

			return handle;
		}

		bool Reset(HANDLE handle = INVALID_HANDLE_VALUE) noexcept
		{
			if (this->handle == handle) return !handle;

			CloseHandle(this->handle);
			this->handle = handle;

			return true;
		}
	};
}