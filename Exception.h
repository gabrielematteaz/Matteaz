#pragma once

#include < Windows.h >

namespace Matteaz
{
	class Exception
	{
		HANDLE heap;
		wchar_t* message;

	protected:
		constexpr wchar_t* _Release() noexcept
		{
			wchar_t* message = this->message;

			heap = NULL;
			this->message = NULL;

			return message;
		}

	public:
		constexpr explicit Exception(HANDLE heap = NULL, const wchar_t* message = nullptr) noexcept :
			heap(NULL),
			message(NULL)
		{
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
		}

		constexpr Exception(const Exception& exception) noexcept :
			Exception(exception.heap, exception.message)
		{

		}

		constexpr Exception(Exception&& exception) noexcept :
			heap(exception.heap),
			message(exception._Release())
		{

		}

		constexpr virtual ~Exception() noexcept
		{
			if (heap != NULL)
			{
				HeapFree(heap, 0, message);
				heap = NULL;
				message = NULL;
			}
		}

		constexpr Exception& operator = (const Exception& exception) noexcept
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

		constexpr Exception& operator = (Exception&& exception) noexcept
		{
			if (this != &exception)
			{
				this->~Exception();
				heap = exception.heap;
				message = exception._Release();
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

		constexpr bool Reset(HANDLE heap = NULL, const wchar_t* message = nullptr) noexcept
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
	};

	class AllocationError : private Exception
	{
	public:
		AllocationError() = delete;
		constexpr AllocationError(const AllocationError&) noexcept = default;
		constexpr AllocationError(AllocationError&&) noexcept = default;
		constexpr virtual ~AllocationError() noexcept = default;
		constexpr AllocationError& operator = (const AllocationError&) noexcept = default;
		constexpr AllocationError& operator = (AllocationError&&) noexcept = default;

		HANDLE heap;
		SIZE_T bytes;

		constexpr AllocationError(HANDLE heap, SIZE_T bytes) noexcept :
			Exception(NULL, NULL),
			heap(heap),
			bytes(bytes)
		{

		}
	};
}