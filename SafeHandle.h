#pragma once

#include < Windows.h >

namespace Matteaz
{
	class SafeHandle
	{
		HANDLE handle;

	public:
		constexpr explicit SafeHandle(HANDLE handle = INVALID_HANDLE_VALUE) noexcept;
		SafeHandle(const SafeHandle&) = delete;
		constexpr SafeHandle(SafeHandle&& safeHandle) noexcept;
		~SafeHandle() noexcept;
		SafeHandle& operator = (const SafeHandle&) = delete;
		SafeHandle& operator = (SafeHandle&& safeHandle) noexcept;
		[[nodiscard]] constexpr HANDLE Get() const noexcept;
		[[nodiscard]] constexpr HANDLE Release() noexcept;
		bool Reset(HANDLE handle = INVALID_HANDLE_VALUE) noexcept;
	};

	constexpr SafeHandle::SafeHandle(HANDLE handle) noexcept :
		handle(handle)
	{

	}

	constexpr SafeHandle::SafeHandle(SafeHandle&& safeHandle) noexcept :
		handle(safeHandle.handle)
	{

	}

	SafeHandle::~SafeHandle() noexcept
	{
		CloseHandle(handle);
		handle = INVALID_HANDLE_VALUE;
	}

	SafeHandle& SafeHandle::operator = (SafeHandle&& safeHandle) noexcept
	{
		if (this != &safeHandle)
		{
			this->~SafeHandle();
			handle = safeHandle.handle;
			safeHandle.handle = INVALID_HANDLE_VALUE;
		}

		return *this;
	}

	constexpr HANDLE SafeHandle::Get() const noexcept
	{
		return handle;
	}

	constexpr HANDLE SafeHandle::Release() noexcept
	{
		HANDLE handle = this->handle;

		this->handle = INVALID_HANDLE_VALUE;

		return handle;
	}

	bool SafeHandle::Reset(HANDLE handle) noexcept
	{
		if (this->handle == handle) return this->handle == INVALID_HANDLE_VALUE ? true : false;

		this->~SafeHandle();
		this->handle = handle;

		return false;
	}
}