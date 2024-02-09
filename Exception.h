#pragma once

#include < Windows.h >

namespace Matteaz
{
	class Exception
	{
		HANDLE heap;
		wchar_t* message;

	public:
		explicit Exception(HANDLE heap = NULL, const wchar_t* message = nullptr) noexcept;
		Exception(const Exception& exception) noexcept;
		virtual ~Exception() noexcept;
		Exception& operator = (const Exception& exception) noexcept;
		Exception& operator = (Exception&& exception) noexcept;
		bool Reset(HANDLE heap = NULL, const wchar_t* message = nullptr) noexcept;

		constexpr Exception(Exception&& exception) noexcept :
			heap(exception.heap),
			message(exception.message)
		{
			exception.heap = NULL;
			exception.message = NULL;
		}

		[[nodiscard]] constexpr HANDLE Heap() const noexcept
		{
			return heap;
		}

		[[nodiscard]] constexpr virtual const wchar_t* Message() const noexcept
		{
			return message;
		}
	};
}