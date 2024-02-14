#ifndef MATTEAZ_ARRAYPOINTER_H
#define MATTEAZ_ARRAYPOINTER_H

#include < type_traits >

#include < Windows.h >

namespace Matteaz
{
	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class ArrayPointer
	{
		HANDLE heap;
		std::size_t count;
		Type* raw = NULL;

	public:
		ArrayPointer(const ArrayPointer&) = delete;
		ArrayPointer& operator = (const ArrayPointer) = delete;

		constexpr explicit ArrayPointer(HANDLE heap = NULL, std::size_t count = 0, Type* raw = NULL) noexcept :
			heap(NULL),
			count(0),
			raw(NULL)
		{
			if (heap != NULL && raw != NULL)
			{
				this->heap = heap;
				this->count = count;
				this->raw = raw;
			}
		}

#ifdef MATTEAZ_POINTER_H
		constexpr ArrayPointer(Pointer < Type >&& pointer) noexcept :
			heap(pointer.Heap()),
			count(heap == NULL ? 0 : 1),
			raw(pointer.Release())
		{

		}
#endif

		constexpr ArrayPointer(ArrayPointer&& arrayPointer) noexcept :
			heap(arrayPointer.heap),
			count(arrayPointer.count),
			raw(arrayPointer.Release())
		{

		}

#ifdef MATTEAZ_POINTER_H
		constexpr ArrayPointer& operator = (Pointer < Type >&& pointer) noexcept
		{
			this->~ArrayPointer();
			heap = pointer.Heap();
			count = heap == NULL ? 0 : 1;
			raw = pointer.Release();

			return *this;
		}
#endif

		constexpr ~ArrayPointer() noexcept
		{
			if (heap != NULL)
			{
				if constexpr (std::is_trivially_destructible_v < Type > == false) for (Type* raw = this->raw; count != 0; count--, raw++) raw->~Type();

				HeapFree(heap, 0, static_cast < LPVOID > (raw));
				heap = NULL;
				count = 0;
				raw = NULL;
			}
		}

		constexpr ArrayPointer& operator = (ArrayPointer&& arrayPointer) noexcept
		{
			if (this != &arrayPointer)
			{
				this->~ArrayPointer();
				heap = arrayPointer.heap;
				count = arrayPointer.count;
				raw = arrayPointer.Release();
			}

			return *this;
		}

		[[nodiscard]] constexpr HANDLE Heap() const noexcept
		{
			return heap;
		}

		[[nodiscard]] constexpr std::size_t Count() const noexcept
		{
			return count;
		}

		[[nodiscard]] constexpr Type* Get() const noexcept
		{
			return raw;
		}

		[[nodiscard]] constexpr Type* Release() noexcept
		{
			Type* raw = this->raw;

			heap = NULL;
			count = 0;
			this->raw = NULL;

			return raw;
		}

		constexpr bool Reset(HANDLE heap = NULL, std::size_t count = 0, Type* raw = NULL) noexcept
		{
			if (this->raw == raw) return this->heap == NULL ? true : false;

			this->~ArrayPointer();

			if (heap != NULL && raw != NULL)
			{
				this->heap = heap;
				this->count = count;
				this->raw = raw;
			}

			return true;
		}
	};
}

#endif