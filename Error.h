#pragma once

#include < Windows.h >

namespace Matteaz
{
	struct Exception
	{
		~Exception() = default;

		constexpr Exception(const wchar_t *message = nullptr) noexcept :
			message(message == nullptr ? L"Matteaz::Exception" : message)
		{

		}

		[[nodiscard]] constexpr virtual const wchar_t *what() const noexcept
		{
			return message;
		}

	protected:
		const wchar_t *message;
	};

	struct SystemError : Exception
	{
		DWORD code;

		constexpr SystemError(const wchar_t *message = nullptr, DWORD code = GetLastError()) noexcept :
			Exception(message == nullptr ? L"Matteaz::SystemError" : message), code(code)
		{

		}
	};
}
