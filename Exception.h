#pragma once

#include <Windows.h>

namespace Matteaz
{
	class Exception
	{
		HANDLE heap;
		wchar_t* message;

	protected:
		Exception(HANDLE heap, const wchar_t* message) noexcept :
			heap(heap),
			message(NULL)
		{
			if (heap != NULL && message != NULL)
			{
				size_t messageLength = wcslen(message) + 1; /* Maybe use "wcsnlen" (No need for "wcsnlen_s") */

				wchar_t* messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, messageLength * sizeof(wchar_t)));
				if (messageCopy != NULL) wcsncpy_s(messageCopy, messageLength, message, messageLength);

				this->message = messageCopy;
			}
		}

	public:
		explicit Exception(const wchar_t* message = NULL) noexcept :
			Exception(GetProcessHeap(), message)
		{

		}

		Exception(const Exception& exception) noexcept :
			Exception(exception.heap, exception.message)
		{

		}

		constexpr Exception(Exception&& exception) noexcept :
			heap(exception.heap),
			message(exception.message)
		{
			exception.message = NULL;
		}

		virtual ~Exception()
		{
			HeapFree(heap, 0, message);
			message = NULL;
		}

		Exception& operator = (const Exception& exception) noexcept
		{
			if (message != exception.message)
			{
				wchar_t* messageCopy = exception.message;

				if (exception.heap != NULL && exception.message != NULL)
				{
					size_t messageLength = wcslen(exception.message) + 1; /* Maybe use "wcsnlen" (No need for "wcsnlen_s") */
				
					messageCopy = static_cast < wchar_t* > (HeapAlloc(exception.heap, 0, messageLength * sizeof(wchar_t)));
					if (messageCopy != NULL) wcsncpy_s(messageCopy, messageLength, exception.message, messageLength);
				}
				
				HeapFree(heap, 0, message);
				heap = exception.heap;
				message = messageCopy;
			}

			return *this;
		}

		Exception& operator = (Exception&& exception) noexcept
		{
			if (message != exception.message)
			{
				HeapFree(heap, 0, message);
				heap = exception.heap;
				message = exception.message;
				exception.message = NULL;
			}

			return *this;
		}

		[[nodiscard]] virtual const wchar_t* Message() const noexcept
		{
			return message;
		}
	};
}