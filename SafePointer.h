#pragma once

#include "Concepts.h"
#include < Windows.h >

namespace Matteaz
{
	template < NoExtentsDestructible Type >
	class SafePointer
	{
		HANDLE heap;
		Type* pointer;

	public:
		SafePointer(const SafePointer&) = delete;
		SafePointer& operator = (const SafePointer&) = delete;

		constexpr SafePointer(HANDLE heap = NULL, Type* pointer = nullptr) noexcept :
			heap(heap),
			pointer(pointer)
		{
			if (heap != NULL && pointer != nullptr)
			{
				this->heap = heap;
				this->pointer = pointer;
			}
		}

		constexpr SafePointer(SafePointer&& safePointer) noexcept :
			heap(safePointer.heap),
			pointer(safePointer.pointer)
		{
			safePointer.heap = NULL;
			safePointer.pointer = nullptr;
		}

		~SafePointer()
		{
			if (pointer != nullptr)
			{
				pointer->~Type();
				HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				heap = NULL;
				pointer = nullptr;
			}
		}

		SafePointer& operator = (SafePointer&& safePointer) noexcept
		{
			if (pointer != safePointer.pointer)
			{
				if (pointer != nullptr)
				{
					pointer->~Type();
					HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				}

				heap = safePointer.heap;
				pointer = safePointer.pointer;
				safePointer.heap = NULL;
				safePointer.pointer = nullptr;
			}

			return *this;
		}

		[[nodiscard]] constexpr HANDLE Heap() const noexcept
		{
			return heap;
		}

		[[nodiscard]] constexpr Type* Get() const noexcept
		{
			return pointer;
		}

		[[nodiscard]] constexpr Type* Release() noexcept
		{
			Type* pointer = this->pointer;

			heap = NULL;
			this->pointer = nullptr;

			return pointer;
		}

		bool Reset(HANDLE heap = NULL, Type* pointer = nullptr) noexcept
		{
			if (this->pointer == pointer) return this->heap == NULL ? true : false;

			if (this->pointer != nullptr)
			{
				this->pointer->~Type();
				HeapFree(this->heap, 0, static_cast < LPVOID > (this->pointer));
			}

			if (heap == NULL || pointer == nullptr)
			{
				this->heap = NULL;
				this->pointer = nullptr;
			}
			else
			{
				this->heap = heap;
				this->pointer = pointer;
			}

			return true;
		}
	};

	template < typename Type >
	class SafePointer < Type[] >
	{
		HANDLE heap;
		SIZE_T count;
		Type* pointer;

	public:
		SafePointer(const SafePointer&) = delete;
		SafePointer& operator = (const SafePointer&) = delete;

		constexpr SafePointer(HANDLE heap = NULL, SIZE_T count = 0, Type* pointer = nullptr) noexcept :
			heap(NULL),
			count(0),
			pointer(nullptr)
		{
			if (heap != NULL && count != 0 && pointer != nullptr)
			{
				this->heap = heap;
				this->count = count;
				this->pointer = pointer;
			}
		}

		constexpr SafePointer(SafePointer&& safePointer) noexcept :
			heap(safePointer.heap),
			count(safePointer.count),
			pointer(safePointer.pointer)
		{
			safePointer.heap = NULL;
			safePointer.count = 0;
			safePointer.pointer = nullptr;
		}

		~SafePointer()
		{
			if (pointer != nullptr)
			{
				for (Type* pointer = this->pointer; count != 0; count--, pointer++) pointer->~Type();

				HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				heap = NULL;
				pointer = nullptr;
			}
		}

		SafePointer& operator = (SafePointer&& safePointer) noexcept
		{
			if (pointer != safePointer.pointer)
			{
				if (pointer != nullptr)
				{
					for (Type* pointer = this->pointer; count != 0; count--, pointer++) pointer->~Type();

					HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				}

				heap = safePointer.heap;
				pointer = safePointer.pointer;
				safePointer.heap = NULL;
				safePointer.pointer = nullptr;
			}

			return *this;
		}

		[[nodiscard]] constexpr HANDLE Heap() const noexcept
		{
			return heap;
		}

		[[nodiscard]] constexpr SIZE_T Count() const noexcept
		{
			return count;
		}

		[[nodiscard]] constexpr Type* Get() const noexcept
		{
			return pointer;
		}

		[[nodiscard]] constexpr Type* Release() noexcept
		{
			Type* pointer = this->pointer;

			heap = NULL;
			count = 0;
			this->pointer = nullptr;

			return pointer;
		}

		bool Reset(HANDLE heap = NULL, SIZE_T count = 0, Type* pointer = nullptr) noexcept
		{
			if (this->pointer == pointer) return this->heap == NULL ? true : false;

			if (this->pointer != nullptr)
			{
				for (Type* pointerCopy = this->pointer; this->count != 0; this->count--, pointerCopy++) pointerCopy->~Type();

				HeapFree(this->heap, 0, static_cast < LPVOID > (this->pointer));
			}

			if (heap == NULL || count == 0 || pointer == nullptr)
			{
				this->heap = NULL;
				this->count = 0;
				this->pointer = nullptr;
			}
			else
			{
				this->heap = heap;
				this->count = count;
				this->pointer = pointer;
			}

			return true;
		}
	};
}