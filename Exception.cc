#include "Exception.h"

namespace Matteaz
{
	Exception::Exception(HANDLE heap, const wchar_t* message) noexcept :
		heap(NULL),
		message(defaultMessages[1])
	{
		if (heap != NULL && message != nullptr)
		{
			size_t length = wcslen(message) + 1;
			wchar_t* messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, length * sizeof(wchar_t)));

			if (messageCopy == NULL) this->message = defaultMessages[2];
			else
			{
				wcscpy_s(messageCopy, length, message);
				this->heap = heap;
				this->message = messageCopy;
			}
		}
	}

	Exception::Exception(const Exception& exception) noexcept :
		heap(NULL),
		message(exception.message)
	{
		if (exception.heap != NULL)
		{
			size_t length = wcslen(exception.message) + 1;
			wchar_t* message = static_cast < wchar_t* > (HeapAlloc(exception.heap, 0, length * sizeof(wchar_t)));

			if (message == NULL) this->message = defaultMessages[2];
			else
			{
				wcscpy_s(message, length, exception.message);
				heap = exception.heap;
				this->message = message;
			}
		}
	}

	Exception::~Exception()
	{
		if (heap != NULL)
		{
			HeapFree(heap, 0, const_cast < wchar_t* > (message));
			heap = NULL;
		}

		message = defaultMessages[0];
	}

	Exception& Exception::operator = (const Exception& exception) noexcept
	{
		if (this != &exception)
		{
			this->~Exception();
			message = exception.message;

			if (exception.heap != NULL)
			{
				size_t length = wcslen(exception.message) + 1;
				wchar_t* message = static_cast < wchar_t* > (HeapAlloc(exception.heap, 0, length * sizeof(wchar_t)));

				if (message == NULL) this->message = defaultMessages[2];
				else
				{
					wcscpy_s(message, length, exception.message);
					heap = exception.heap;
					this->message = message;
				}
			}
		}

		return *this;
	}

	Exception& Exception::operator = (Exception&& exception) noexcept
	{
		if (this != &exception)
		{
			this->~Exception();
			heap = exception.heap;
			message = exception.message;
			exception.heap = NULL;
			exception.message = defaultMessages[0];
		}

		return *this;
	}
}