#pragma once

#include < Windows.h >

namespace Matteaz
{
	class Exception
	{
		HANDLE heap;
		wchar_t* message;

	public:
		explicit Exception(HANDLE heap = NULL, const wchar_t* message = nullptr) noexcept :
			heap(NULL),
			message(nullptr)
		{
			if (heap != NULL && message != nullptr)
			{
				size_t messageLength = wcslen(message) + 1;
				wchar_t* messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, messageLength * sizeof(wchar_t)));

				if (messageCopy != NULL)
				{
					if (wcsncpy_s(messageCopy, messageLength, message, messageLength) == 0)
					{
						this->heap = heap;
						this->message = messageCopy;
					}
					else HeapFree(heap, 0, messageCopy);
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
			message = nullptr;
		}

		Exception& operator = (const Exception& exception) noexcept
		{
			if (message != exception.message)
			{
				HeapFree(heap, 0, message);

				size_t messageLength;
				wchar_t* messageCopy = NULL;

				if (exception.heap != NULL)
				{
					messageLength = wcslen(exception.message) + 1;
					messageCopy = static_cast < wchar_t* > (HeapAlloc(exception.heap, 0, messageLength * sizeof(wchar_t)));
				}

				if (messageCopy != NULL && wcsncpy_s(messageCopy, messageLength, exception.message, messageLength) != 0)
				{
					HeapFree(exception.heap, 0, messageCopy);
					messageCopy = NULL;
				}

				if (messageCopy == NULL)
				{
					heap = NULL;
					message = nullptr;
				}
				else
				{
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
				exception.message = nullptr;
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

		bool Reset(HANDLE heap = NULL, const wchar_t* message = nullptr) noexcept
		{
			if (this->heap == heap && this->message == message) return this->heap == NULL ? true : false;

			HeapFree(this->heap, 0, this->message);

			size_t messageLength;
			wchar_t* messageCopy = NULL;

			if (heap != NULL && message != nullptr)
			{
				messageLength = wcslen(message) + 1;
				messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, messageLength * sizeof(wchar_t)));
			}

			if (messageCopy != NULL && wcsncpy_s(messageCopy, messageLength, message, messageLength) != 0)
			{
				HeapFree(heap, 0, messageCopy);
				messageCopy = NULL;
			}

			if (messageCopy == NULL)
			{
				this->heap = NULL;
				this->message = nullptr;
			}
			else
			{
				this->heap = heap;
				this->message = messageCopy;
			}

			return true;
		}
	};
}