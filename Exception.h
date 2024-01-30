#pragma once

#include <Windows.h>

namespace Matteaz
{
	class Exception
	{
		HANDLE heap;
		wchar_t* message;

	public:
		/* It is not recommended to pass a handle to a private heap, as private heaps can be be destroyed
		using "HeapDestroy" (it would cause "this->message" to be invalidated prematurely) */
		explicit Exception(const wchar_t* message = NULL, HANDLE heap = GetProcessHeap()) noexcept :
			heap(heap),
			message(NULL)
		{
			/* If "message" is NULL, there is no message to copy. If "heap" is NULL, no allocation can be made */
			if (message != NULL && heap != NULL)
			{
				/* "wcsnlen" is more secure but no maximum length is set ("wcsnlen_s" is useless
				since we already know that "message" is not NULL) */
				size_t messageLength = wcslen(message) + 1;

				wchar_t* messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, messageLength * sizeof(wchar_t)));
				if (messageCopy != NULL) wcsncpy_s(messageCopy, messageLength, message, messageLength);

				this->message = messageCopy;
			}
		}

		Exception(const Exception& exception) noexcept :
			Exception(exception.message, exception.heap)
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
			if (this != &exception)
			{
				wchar_t* messageCopy = NULL;

				if (exception.message != NULL && exception.heap != NULL)
				{
					size_t messageLength = wcslen(exception.message) + 1;

					messageCopy = static_cast < wchar_t* > (HeapAlloc(exception.heap, 0, messageLength * sizeof(wchar_t)));
					if (messageCopy != NULL) wcsncpy_s(messageCopy, messageLength, exception.message, messageLength);
				}

				HeapFree(heap, 0, message);
				message = messageCopy;

				heap = exception.heap;
			}

			return *this;
		}

		Exception& operator = (Exception&& exception) noexcept
		{
			if (this != &exception)
			{
				HeapFree(heap, 0, message);
				message = exception.message;

				heap = exception.heap;

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

		void Reset(const wchar_t* message = NULL, HANDLE heap = GetProcessHeap()) noexcept
		{
			if (this->message != message || this->heap != heap)
			{
				wchar_t* messageCopy = NULL;

				if (message != NULL && heap != NULL)
				{
					size_t messageLength = wcslen(message) + 1;

					messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, messageLength * sizeof(wchar_t)));
					if (messageCopy != NULL) wcsncpy_s(messageCopy, messageLength, message, messageLength);
				}

				HeapFree(this->heap, 0, this->message);
				this->message = messageCopy;

				this->heap = heap;
			}
		}
	};
}