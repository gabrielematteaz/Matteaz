#pragma once

#include < Windows.h >

namespace Matteaz
{
	class Exception
	{
		HANDLE heap;
		wchar_t* message;

	public:
		explicit Exception(HANDLE heap = NULL, const wchar_t* message = NULL) noexcept;
		Exception& operator = (const Exception& exception) noexcept;
		bool Reset(HANDLE heap = NULL, const wchar_t* message = NULL) noexcept;

		Exception(const Exception& exception) noexcept :
			Exception(exception.heap, exception.message)
		{

		}

		constexpr Exception(Exception&& exception) noexcept :
			heap(exception.heap),
			message(exception.message)
		{
			exception.heap = NULL;
			exception.message = NULL;
		}

		virtual ~Exception()
		{
			HeapFree(heap, 0, message);
			heap = NULL;
			message = NULL;
		}

		Exception& operator = (Exception&& exception) noexcept
		{
			if (this != &exception)
			{
				HeapFree(heap, 0, message);
				heap = exception.heap;
				message = exception.message;
				exception.heap = NULL;
				exception.message = NULL;
			}

			return *this;
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