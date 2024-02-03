#pragma once

#include "Concepts.h"
#include < Windows.h >
#include < memory >

namespace Matteaz
{
	template < NoExtentsDestructible Type >
	class SafePointer
	{
		HANDLE heap; /* Must be valid when deallocating 'this->pointer' */
		Type* pointer; /* Must be valid when destroying and deallocating the managed object */

	public:
		SafePointer(const SafePointer&) = delete;
		SafePointer& operator = (const SafePointer&) = delete;

		/* DO NOT directly allocate the 'pointer' inside the constructor, since passing an invalid
		argument will cause the pointer pointing to the newly allocated memory to be lost */
		constexpr explicit SafePointer(HANDLE heap = NULL, Type* pointer = NULL) noexcept :
			heap(NULL),
			pointer(NULL)
		{
			/* pointers are more likely to be NULL */
			if (pointer != NULL && heap != NULL)
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
			safePointer.pointer = NULL;
		}

		~SafePointer()
		{
			if (pointer != NULL)
			{
				std::destroy_at(pointer);
				HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				heap = NULL;
				pointer = NULL;
			}
		}

		SafePointer& operator = (SafePointer&& safePointer) noexcept
		{
			/* If the heap handles are different, the pointers are also different (if not NULL) */
			if (pointer != safePointer.pointer)
			{
				if (pointer != NULL)
				{
					std::destroy_at(pointer);
					HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				}

				heap = safePointer.heap;
				pointer = safePointer.pointer;
				safePointer.heap = NULL;
				safePointer.pointer = NULL;
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

		[[nodiscard("The managed object would never be destroyed and deallocated")]]
		constexpr Type* Release() noexcept
		{
			HANDLE pointer = this->pointer;

			heap = NULL;
			this->pointer = NULL;

			return pointer;
		}

		void Reset(HANDLE heap = NULL, Type* pointer = NULL) noexcept
		{
			if (this->pointer != pointer)
			{
				if (this->pointer != NULL)
				{
					std::destroy_at(this->pointer);
					HeapFree(this->heap, 0, static_cast < LPVOID > (this->pointer));
				}

				if (pointer == NULL || heap == NULL)
				{
					this->heap = NULL;
					this->pointer = NULL;
				}
				else
				{
					this->heap = heap;
					this->pointer = pointer;
				}
			}
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

		constexpr explicit SafePointer(HANDLE heap = NULL, SIZE_T count = 0, Type* pointer = NULL) noexcept :
			heap(NULL),
			count(0),
			pointer(NULL)
		{
			if (heap != NULL && count != 0 && pointer != NULL)
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
			safePointer.pointer = NULL;
		}

		~SafePointer()
		{
			if (pointer != NULL)
			{
				for (Type* pointer = this->pointer; count != 0; count--, pointer++) std::destroy_at(pointer);

				HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				heap = NULL;
				count = 0;
				pointer = NULL;
			}
		}

		SafePointer& operator = (SafePointer&& safePointer) noexcept
		{
			if (pointer != safePointer.pointer)
			{
				if (pointer != NULL)
				{
					for (Type* pointer = this->pointer; count != 0; count--, pointer++) std::destroy_at(pointer);

					HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				}

				heap = safePointer.heap;
				count = safePointer.count;
				pointer = safePointer.pointer;
				safePointer.heap = NULL;
				safePointer.count = 0;
				safePointer.pointer = NULL;
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

		[[nodiscard("The managed objects would never be destroyed and deallocated")]]
		constexpr Type* Release() noexcept
		{
			HANDLE pointer = this->pointer;

			heap = NULL;
			count = 0;
			this->pointer = NULL;

			return pointer;
		}

		bool Reset(HANDLE heap = NULL, SIZE_T count = 0, Type* pointer = NULL) noexcept
		{
			if (this->pointer == pointer) return this->pointer == NULL ? true : false;
			
			if (this->pointer != NULL)
			{
				for (Type* pointer = this->pointer; this->count != 0; this->count--, pointer++) std::destroy_at(pointer);

				HeapFree(this->heap, 0, static_cast < LPVOID > (this->pointer));
			}

			if (pointer == NULL || count == 0 || heap == NULL)
			{
				this->heap = NULL;
				this->count = 0;
				this->pointer = NULL;
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