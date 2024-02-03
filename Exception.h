#pragma once

#include < Windows.h >

namespace Matteaz
{
	class Exception
	{
		HANDLE heap; /* Must be valid when allocating/deallocating 'this->message' */
		wchar_t* message;

	public:
		/* 'this->message' is not guaranteed to be a copy of 'message' */
		explicit Exception(HANDLE heap = NULL, const wchar_t* message = NULL) noexcept :
			heap(NULL),
			message(NULL)
		{
			if (heap != NULL && message != NULL)
			{
				/* 'wcsnlen' is safer but there is no maximum length set ('wcsnlen_s' is useless
				since we already know that 'message' is not NULL) */
				size_t messageLength = wcslen(message) + 1;
				wchar_t* messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, messageLength * sizeof(wchar_t)));

				if (messageCopy != NULL)
				{
					wcsncpy_s(messageCopy, messageLength, message, messageLength);
					this->heap = heap;
					this->message = messageCopy;
				}
			}
		}

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

		Exception& operator = (const Exception& exception) noexcept
		{
			/* Different instances have different messages (if not NULL) */
			if (message != exception.message)
			{
				/* If both heap handles are equal, deallocating first will improve the probability
				of success of the next allocation */
				HeapFree(heap, 0, message);

				size_t messageLength;
				wchar_t* messageCopy = NULL;

				if (exception.message != NULL)
				{
					messageLength = wcslen(exception.message) + 1;
					messageCopy = static_cast < wchar_t* > (HeapAlloc(exception.heap, 0, messageLength * sizeof(wchar_t)));
				}

				if (messageCopy == NULL)
				{
					heap = NULL;
					message = NULL;
				}
				else
				{
					wcsncpy_s(messageCopy, messageLength, exception.message, messageLength);
					heap = exception.heap;
					message = messageCopy;
				}
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
				exception.heap = NULL;
				exception.message = NULL;
			}

			return *this;
		}

		[[nodiscard]] constexpr HANDLE Get() const noexcept
		{
			return heap;
		}

		/* You can safely modify the content of the string returned by this function (without reallocating/deallocating
		it though. If not NULL, the size of its buffer in characters is exactly 'wcslen(Message()) + 1') */
		[[nodiscard]] constexpr virtual const wchar_t* Message() const noexcept
		{
			return message;
		}

		void Reset(HANDLE heap = NULL, const wchar_t* message = NULL) noexcept
		{
			/* messages are more likely to be different */
			if (this->message != message || this->heap != heap)
			{
				HeapFree(this->heap, 0, this->message);

				size_t messageLength;
				wchar_t* messageCopy = NULL;

				if (heap != NULL && message != NULL)
				{
					messageLength = wcslen(message) + 1;
					messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, messageLength * sizeof(wchar_t)));
				}

				if (messageCopy == NULL)
				{
					this->heap = NULL;
					this->message = NULL;
				}
				else
				{
					wcsncpy_s(messageCopy, messageLength, message, messageLength);
					this->heap = heap;
					this->message = messageCopy;
				}
			}
		}
	};
}
