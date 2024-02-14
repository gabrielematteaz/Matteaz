#ifndef MATTEAZ_POINTER_H
#define MATTEAZ_POINTER_H

#include < type_traits >

#include < Windows.h >

namespace Matteaz
{
	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class Pointer
	{
		HANDLE heap;
		Type* raw;

	public:
		Pointer(const Pointer&) = delete;
		Pointer& operator = (const Pointer&) = delete;

		constexpr explicit Pointer(HANDLE heap = NULL, Type* raw = NULL) noexcept :
			heap(NULL),
			raw(NULL)
		{
			if (heap != NULL && raw != NULL)
			{
				this->heap = heap;
				this->raw = raw;
			}
		}

		constexpr Pointer(Pointer&& pointer) noexcept :
			heap(pointer.heap),
			raw(pointer.Release())
		{

		}

		constexpr ~Pointer() noexcept
		{
			if (heap != NULL)
			{
				if constexpr (std::is_trivially_destructible_v < Type > == false) raw->~Type();

				HeapFree(heap, 0, static_cast < LPVOID > (raw));
				heap = NULL;
				raw = NULL;
			}
		}

		constexpr Pointer& operator = (Pointer&& pointer) noexcept
		{
			if (this != &pointer)
			{
				this->~Pointer();
				heap = pointer.heap;
				raw = pointer.Release();
			}

			return *this;
		}

		[[nodiscard]] constexpr HANDLE Heap() const noexcept
		{
			return heap;
		}

		[[nodiscard]] constexpr Type* Get() const noexcept
		{
			return raw;
		}

		[[nodiscard]] constexpr Type* Release() noexcept
		{
			Type* raw = this->raw;

			heap = NULL;
			this->raw = NULL;

			return raw;
		}

		constexpr bool Reset(HANDLE heap = NULL, Type* raw = NULL) noexcept
		{
			if (this->raw == raw) return this->heap == NULL ? true : false;

			this->~Pointer();

			if (heap != NULL && raw != NULL)
			{
				this->heap = heap;
				this->raw = raw;
			}

			return true;
		}
	};
}

#endif