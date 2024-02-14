#ifndef MATTEAZ_SHAREDARRAYPOINTER_H
#define MATTEAZ_SHAREDARRAYPOINTER_H

#include < type_traits >

#include < Windows.h >

namespace Matteaz
{
	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SharedArrayPointer
	{
		std::size_t* references;
		HANDLE heap;
		std::size_t count;
		Type* raw;

	public:
		constexpr explicit SharedArrayPointer(HANDLE heap = NULL, std::size_t count = 0, Type* raw = NULL) :
			references(NULL),
			heap(NULL),
			count(0),
			raw(NULL)
		{
			if (heap != NULL && raw != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(heap, 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				this->heap = heap;
				this->count = count;
				this->raw = raw;
			}
		}

#ifdef MATTEAZ_SHAREDPOINTER_H
		constexpr SharedArrayPointer(const SharedPointer < Type >& sharedPointer) noexcept :
			references(NULL),
			heap(NULL),
			count(0),
			raw(NULL)
		{
			if (sharedPointer.references != NULL)
			{
				references = sharedPointer.references;
				(*references)++;
				heap = sharedPointer.heap;
				count = 1;
				raw = sharedPointer.raw;
			}
		}
#endif

		constexpr SharedArrayPointer(const SharedArrayPointer& sharedArrayPointer) noexcept :
			references(sharedArrayPointer.references),
			heap(sharedArrayPointer.heap),
			count(sharedArrayPointer.count),
			raw(sharedArrayPointer.raw)
		{
			if (references != NULL) (*references)++;
		}

#ifdef MATTEAZ_POINTER_H
		constexpr SharedArrayPointer(Pointer < Type >&& pointer) :
			references(NULL),
			heap(NULL),
			count(0),
			raw(NULL)
		{
			if (pointer.Heap() != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(pointer.Heap(), 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				heap = pointer.Heap();
				count = 1;
				raw = pointer.Release();
			}
		}
#endif

#ifdef MATTEAZ_ARRAYPOINTER_H
		constexpr SharedArrayPointer(ArrayPointer < Type >&& arrayPointer) :
			references(NULL),
			heap(NULL),
			count(0),
			raw(NULL)
		{
			if (arrayPointer.Heap() != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(arrayPointer.Heap(), 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				heap = arrayPointer.Heap();
				count = arrayPointer.Count();
				raw = arrayPointer.Release();
			}
		}
#endif

#ifdef MATTEAZ_SHAREDPOINTER_H
		constexpr SharedArrayPointer(SharedPointer < Type >&& sharedPointer) noexcept :
			references(NULL),
			heap(NULL),
			count(0),
			raw(NULL)
		{
			if (sharedPointer.references != NULL)
			{
				references = sharedPointer.references;
				heap = sharedPointer.heap;
				count = 1;
				raw = sharedPointer.Release();
			}
		}
#endif

		constexpr SharedArrayPointer(SharedArrayPointer&& sharedArrayPointer) noexcept :
			references(sharedArrayPointer.references),
			heap(sharedArrayPointer.heap),
			count(sharedArrayPointer.count),
			raw(sharedArrayPointer.Release())
		{

		}

		constexpr ~SharedArrayPointer() noexcept
		{
			if (references != NULL)
			{
				(*references)--;

				if (*references == 0)
				{
					HeapFree(heap, 0, references);

					if constexpr (std::is_trivially_destructible_v < Type > == false) for (Type* raw = this->raw; count != 0; count--, raw++) raw->~Type();

					HeapFree(heap, 0, static_cast < LPVOID > (raw));
				}

				references = NULL;
				heap = NULL;
				count = 0;
				raw = NULL;
			}
		}

#ifdef MATTEAZ_SHAREDPOINTER_H
		constexpr SharedArrayPointer& operator = (const SharedPointer < Type >& sharedPointer) noexcept
		{
			this->~SharedArrayPointer();

			if (sharedPointer.references != NULL)
			{
				references = sharedPointer.references;
				(*references)++;
				heap = sharedPointer.heap;
				count = 1;
				raw = sharedPointer.raw;
			}

			return *this;
		}
#endif

		constexpr SharedArrayPointer& operator = (const SharedArrayPointer& sharedArrayPointer) noexcept
		{
			if (this != &sharedArrayPointer)
			{
				this->~SharedArrayPointer();
				references = sharedArrayPointer.references;

				if (references != NULL) (*references)++;

				heap = sharedArrayPointer.heap;
				count = sharedArrayPointer.count;
				raw = sharedArrayPointer.raw;
			}

			return *this;
		}

#ifdef MATTEAZ_POINTER_H
		constexpr SharedArrayPointer& operator = (Pointer < Type >&& pointer)
		{
			this->~SharedArrayPointer();

			if (pointer.Heap() != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(pointer.Heap(), 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				heap = pointer.Heap();
				count = 1;
				raw = pointer.Release();
			}

			return *this;
		}
#endif

#ifdef MATTEAZ_ARRAYPOINTER_H
		constexpr SharedArrayPointer& operator = (ArrayPointer < Type >&& arrayPointer)
		{
			this->~SharedArrayPointer();

			if (arrayPointer.Heap() != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(arrayPointer.Heap(), 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				heap = arrayPointer.Heap();
				count = arrayPointer.Count();
				raw = arrayPointer.Release();
			}

			return *this;
		}
#endif

#ifdef MATTEAZ_SHAREDPOINTER_H
		constexpr SharedArrayPointer& operator = (SharedPointer < Type >&& sharedPointer) noexcept
		{
			this->~SharedArrayPointer();

			if (sharedPointer.references != NULL)
			{
				references = sharedPointer.references;
				heap = sharedPointer.heap;
				count = 1;
				raw = sharedPointer.Release();
			}

			return *this;
		}
#endif

		constexpr SharedArrayPointer& operator = (SharedArrayPointer&& sharedArrayPointer) noexcept
		{
			if (this != &sharedArrayPointer)
			{
				this->~SharedArrayPointer();
				references = sharedArrayPointer.references;
				heap = sharedArrayPointer.heap;
				count = sharedArrayPointer.count;
				raw = sharedArrayPointer.Release();
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

			if (references != NULL)
			{
				(*references)--;

				if (*references == 0) HeapFree(heap, 0, references);

				references = NULL;
				heap = NULL;
				count = 0;
				this->raw = NULL;
			}

			return raw;
		}

		constexpr bool Reset(HANDLE heap = NULL, std::size_t count = 0, Type* raw = NULL)
		{
			if (this->raw == raw) return this->references == NULL ? true : false;

			this->~SharedArrayPointer();

			if (heap != NULL && raw != NULL)
			{
				SIZE_T bytes = sizeof(std::size_t);

				references = static_cast < std::size_t* > (HeapAlloc(heap, 0, bytes));

				if (references == NULL) throw AllocationError(heap, bytes);

				*references = 1;
				this->heap = heap;
				this->count = count;
				this->raw = raw;
			}

			return true;
		}
	};
}

#endif