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
		constexpr Exception(Exception&& exception) noexcept;
		virtual ~Exception() noexcept;
		Exception& operator = (const Exception& exception) noexcept;
		Exception& operator = (Exception&& exception) noexcept;
		[[nodiscard]] constexpr HANDLE Heap() const noexcept;
		[[nodiscard]] constexpr virtual const wchar_t* Message() const noexcept;
		bool Reset(HANDLE heap = NULL, const wchar_t* message = nullptr) noexcept;
	};

	Matteaz::Exception::Exception(HANDLE heap, const wchar_t* message) noexcept :
		heap(NULL),
		message(NULL)
	{
		if (heap != NULL && message != nullptr)
		{
			size_t length = wcslen(message) + 1;
			wchar_t* messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, length * sizeof(wchar_t)));

			if (messageCopy != NULL)
			{
				if (wcsncpy_s(messageCopy, length, message, length) == 0)
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

	constexpr Exception::Exception(Exception&& exception) noexcept :
		heap(exception.heap),
		message(exception.message)
	{
		exception.heap = NULL;
		exception.message = NULL;
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

			size_t length;
			wchar_t* messageCopy = NULL;

			if (exception.heap != NULL)
			{
				length = wcslen(exception.message) + 1;
				messageCopy = static_cast < wchar_t* > (HeapAlloc(exception.heap, 0, length * sizeof(wchar_t)));
			}

			if (messageCopy != NULL && wcsncpy_s(messageCopy, length, exception.message, length) != 0)
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

	constexpr HANDLE Exception::Heap() const noexcept
	{
		return heap;
	}

	constexpr const wchar_t* Exception::Message() const noexcept
	{
		return message;
	}

	bool Exception::Reset(HANDLE heap, const wchar_t* message) noexcept
	{
		if (this->message == message) return this->heap == NULL ? true : false;

		this->~Exception();

		size_t length;
		wchar_t* messageCopy = NULL;

		if (heap != NULL && message != nullptr)
		{
			length = wcslen(message) + 1;
			messageCopy = static_cast < wchar_t* > (HeapAlloc(heap, 0, length * sizeof(wchar_t)));
		}

		if (messageCopy != NULL && wcsncpy_s(messageCopy, length, message, length) != 0)
		{
			HeapFree(heap, 0, messageCopy);
			messageCopy = NULL;
		}

		if (messageCopy != NULL)
		{
			this->heap = heap;
			this->message = messageCopy;
		}

		return true;
	}
}