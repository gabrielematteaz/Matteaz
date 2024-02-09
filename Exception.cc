#include "Exception.h"

namespace Matteaz
{
	Exception::Exception(HANDLE heap, const wchar_t* message) noexcept :
		heap(NULL),
		message(NULL)
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

	Exception::Exception(const Exception& exception) noexcept :
		Exception(exception.heap, exception.message)
	{

	}

	Exception::~Exception() noexcept
	{
		HeapFree(heap, 0, message);
		heap = NULL;
		message = NULL;
	}

	Exception& Exception::operator = (const Exception& exception) noexcept
	{
		if (this != &exception)
		{
			this->~Exception();

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

			if (messageCopy != NULL)
			{
				heap = exception.heap;
				message = messageCopy;
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
			exception.message = NULL;
		}

		return *this;
	}

	bool Exception::Reset(HANDLE heap, const wchar_t* message) noexcept
	{
		if (this->message == message) return this->heap == NULL ? true : false;

		this->~Exception();

		size_t messageLength;
		wchar_t* messageCopy = NULL;

		if (heap != NULL)
		{
			messageLength = wcslen(message) + 1;
			messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, messageLength * sizeof(wchar_t)));
		}

		if (messageCopy != NULL && wcsncpy_s(messageCopy, messageLength, message, messageLength) != 0)
		{
			HeapFree(heap, 0, messageCopy);
			messageCopy = NULL;
		}

		if (messageCopy != NULL)
		{
			this->heap = heap;
			this->message = messageCopy;
		}

		return false;
	}
}