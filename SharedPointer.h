#ifndef MATTEAZ_SHAREDPOINTER_H
#define MATTEAZ_SHAREDPOINTER_H

#include < type_traits >

#include "Exception.h"

namespace Matteaz
{
	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SharedPointer
	{
		template < typename Type >
			requires (std::is_nothrow_destructible_v < Type > == true)
		friend class SharedArrayPointer;

		std::size_t* references;
		HANDLE heap;
		Type* raw;

	public:
		constexpr explicit SharedPointer(HANDLE heap = NULL, Type* raw = NULL) :
			references(NULL),
			heap(NULL),
			raw(NULL)
		{
			if (heap != NULL && raw != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(heap, 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				this->heap = heap;
				this->raw = raw;
			}
		}

		constexpr SharedPointer(const SharedPointer& sharedPointer) noexcept :
			references(sharedPointer.references),
			heap(sharedPointer.heap),
			raw(sharedPointer.raw)
		{
			if (references != NULL) (*references)++;
		}

#ifdef MATTEAZ_POINTER_H
		constexpr SharedPointer(Pointer < Type >&& pointer) :
			references(NULL),
			heap(NULL),
			raw(NULL)
		{
			if (pointer.Heap() != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(pointer.Heap(), 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				heap = pointer.Heap();
				raw = pointer.Release();
			}
		}
#endif

		constexpr SharedPointer(SharedPointer&& sharedPointer) noexcept :
			references(sharedPointer.references),
			heap(sharedPointer.heap),
			raw(sharedPointer.Release())
		{

		}

		constexpr ~SharedPointer() noexcept
		{
			if (references != NULL)
			{
				(*references)--;

				if (*references == 0)
				{
					HeapFree(heap, 0, references);

					if constexpr (std::is_trivially_destructible_v < Type > == false) raw->~Type();

					HeapFree(heap, 0, static_cast < LPVOID > (raw));
				}

				references = NULL;
				heap = NULL;
				raw = NULL;
			}
		}

		constexpr SharedPointer& operator = (const SharedPointer& sharedPointer) noexcept
		{
			if (this != &sharedPointer)
			{
				this->~SharedPointer();
				references = sharedPointer.references;

				if (references != NULL) (*references)++;

				heap = sharedPointer.heap;
				raw = sharedPointer.raw;
			}

			return *this;
		}

#ifdef MATTEAZ_POINTER_H
		constexpr SharedPointer& operator = (Pointer < Type >&& pointer)
		{
			this->~SharedPointer();

			if (pointer.Heap() != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(pointer.Heap(), 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				heap = pointer.Heap();
				raw = pointer.Release();
			}

			return *this;
		}
#endif

		constexpr SharedPointer& operator = (SharedPointer&& sharedPointer) noexcept
		{
			if (this != &sharedPointer)
			{
				this->~SharedPointer();
				references = sharedPointer.references;
				heap = sharedPointer.heap;
				raw = sharedPointer.Release();
			}

			return *this;
		}

		[[nodiscard]] constexpr std::size_t References() const noexcept
		{
			return references == NULL ? 0 : *references;
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

			if (references != NULL)
			{
				(*references)--;

				if (*references == 0) HeapFree(heap, 0, references);

				references = NULL;
				heap = NULL;
				this->raw = NULL;
			}

			return raw;
		}

		constexpr bool Reset(HANDLE heap = NULL, Type* raw = NULL)
		{
			if (this->raw == raw) return this->references == NULL ? true : false;

			this->~SharedPointer();

			if (heap != NULL && raw != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(heap, 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				this->heap = heap;
				this->raw = raw;
			}

			return true;
		}
	};
}

#endif