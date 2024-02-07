#include "Exception.h"

namespace Matteaz
{
	Exception::Exception(HANDLE heap, const wchar_t* message) noexcept :
		heap(NULL),
		message(NULL)
	{
		if (heap != NULL && message != NULL)
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

	Exception& Exception::operator = (const Exception& exception) noexcept
	{
		if (this != &exception)
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
				message = NULL;
			}
			else
			{
				heap = exception.heap;
				message = messageCopy;
			}
		}

		return *this;
	}

	bool Exception::Reset(HANDLE heap, const wchar_t* message) noexcept
	{
		if (this->message == message) return this->heap == NULL ? true : false;

		HeapFree(this->heap, 0, this->message);

		size_t messageLength;
		wchar_t* messageCopy = NULL;

		if (heap != NULL && message != NULL)
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
			this->message = NULL;
		}
		else
		{
			this->heap = heap;
			this->message = messageCopy;
		}

		return true;
	}
}