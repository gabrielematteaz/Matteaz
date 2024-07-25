#pragma once

#include < Windows.h >

namespace matteaz
{
	struct basic_exception
	{
		constexpr ~basic_exception() = default;

		constexpr basic_exception(const wchar_t *message = nullptr) noexcept :
			message_(message == nullptr ? L"matteaz::basic_exception" : message)
		{

		}

		[[nodiscard]] constexpr virtual const wchar_t *what() const noexcept
		{
			return message_;
		}

	protected:
		const wchar_t *message_;
	};

	struct DWORD_error : basic_exception
	{
		DWORD code;

		constexpr DWORD_error(const wchar_t *message = nullptr, DWORD code = GetLastError()) noexcept :
			basic_exception(message == nullptr ? L"matteaz::DWORD_error" : message), code(code)
		{

		}
	};
}