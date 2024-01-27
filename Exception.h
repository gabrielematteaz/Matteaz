#pragma once

#include <Windows.h>

namespace Matteaz
{
	class Exception
	{
		wchar_t* message;
		HANDLE heap;

	public:
		/* Since this constructor is designed not to throw any axceptions and the copying
		process of "message" might fail, "this->message" is not guaranteed to be
		a valid copy (It might be NULL) */
		explicit Exception(const wchar_t* message = NULL, HANDLE heap = NULL) noexcept :
			message(NULL),
			heap(heap)
		{
			if (message != NULL && heap != NULL)
			{
				/* "wcsnlen" is safer but I can't come up with a meaningful maximum
				length (There is no need to use "wcsnlen_s" since I already checked if "message" is NULL) */
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
			message(exception.message),
			heap(exception.heap)
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
				wchar_t* messageCopy = NULL;

				if (exception.message != NULL && exception.heap != NULL)
				{
					/* Go to the constructor definition to know why I
					used "wcslen" instead of "wcsnlen" */
					size_t messageLength = wcslen(exception.message) + 1;

					messageCopy = static_cast < wchar_t* > (HeapAlloc(exception.heap, 0, messageLength * sizeof(wchar_t)));
					if (messageCopy != NULL) wcsncpy_s(messageCopy, messageLength, exception.message, messageLength);
				}

				/* If the next line is commented then you commit to the fact
				that you could lose the previous message in case the copying process fails */
				if (messageCopy != NULL)
				{
					HeapFree(heap, 0, message);
					message = messageCopy;

					heap = exception.heap;
				}
			}

			return *this;
		}

		Exception& operator = (Exception&& exception) noexcept
		{
			if (message != exception.message)
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

		/* This class is not designed to let "message" be modified but if you really wanted to do
		so just cast away the const-ness of the pointer returned by this
		function (It is safe since the original message was copied in the constructor) */
		[[nodiscard]] constexpr virtual const wchar_t* Message() const noexcept
		{
			return message;
		}
	};
}