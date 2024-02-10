#pragma once

#include < new >
#include "Exception.h"

namespace Matteaz
{
	class AllocationError : private Exception
	{
	public:
		HANDLE heap;
		SIZE_T bytes;

		AllocationError() = delete;
		AllocationError(HANDLE heap, SIZE_T bytes) noexcept;
		AllocationError(const AllocationError&) noexcept = default;
		constexpr AllocationError(AllocationError&&) noexcept = default;
		~AllocationError() noexcept = default;
		AllocationError& operator = (const AllocationError&) noexcept = default;
		AllocationError& operator = (AllocationError&&) noexcept = default;
	};

	AllocationError::AllocationError(HANDLE heap, SIZE_T bytes) noexcept :
		Exception(),
		heap(heap),
		bytes(bytes)
	{

	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SafePointer
	{
		HANDLE heap;
		Type* pointer;

	public:
		constexpr SafePointer(HANDLE heap = NULL, Type* pointer = NULL) noexcept;
		SafePointer(const SafePointer&) = delete;
		constexpr SafePointer(SafePointer&& safePointer) noexcept;
		~SafePointer() noexcept;
		SafePointer& operator = (const SafePointer&) = delete;
		SafePointer& operator = (SafePointer&& safePointer) noexcept;
		[[nodiscard]] constexpr HANDLE Heap() const noexcept;
		[[nodiscard]] constexpr Type* Get() const noexcept;
		[[nodiscard]] constexpr Type* Release() noexcept;
		bool Reset(HANDLE heap = NULL, Type* pointer = NULL) noexcept;
	};

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr SafePointer < Type >::SafePointer(HANDLE heap, Type* pointer) noexcept :
		heap(NULL),
		pointer(NULL)
	{
		if (heap != NULL && pointer != NULL)
		{
			this->heap = heap;
			this->pointer = pointer;
		}
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr SafePointer < Type >::SafePointer(SafePointer&& safePointer) noexcept :
		heap(safePointer.heap),
		pointer(safePointer.Release())
	{

	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SafePointer < Type >::~SafePointer() noexcept
	{
		if (heap != NULL)
		{
			pointer->~Type();
			HeapFree(heap, 0, static_cast < LPVOID > (pointer));
			heap = NULL;
			pointer = NULL;
		}
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SafePointer < Type >& SafePointer < Type >::operator = (SafePointer&& safePointer) noexcept
	{
		if (this != &safePointer)
		{
			this->~SafePointer();
			heap = safePointer.heap;
			pointer = safePointer.Release();
		}

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr HANDLE SafePointer < Type >::Heap() const noexcept
	{
		return heap;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr Type* SafePointer < Type >::Get() const noexcept
	{
		return pointer;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr Type* SafePointer < Type >::Release() noexcept
	{
		Type* pointer = this->pointer;

		heap = NULL;
		this->pointer = NULL;

		return pointer;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	bool SafePointer < Type >::Reset(HANDLE heap, Type* pointer) noexcept
	{
		if (this->pointer == pointer) return this->heap == NULL ? true : false;

		this->~SafePointer();

		if (heap != NULL && pointer != NULL)
		{
			this->heap = heap;
			this->pointer = pointer;
		}

		return true;
	}

	template < typename Type, typename... ConstructorArguments >
		requires (std::is_constructible_v < Type, ConstructorArguments... > == true && std::is_nothrow_destructible_v < Type > == true)
	SafePointer < Type > MakeSafePointer(HANDLE heap, ConstructorArguments&&... constructorArguments)
	{
		constexpr SIZE_T bytes = sizeof(Type);

		if (heap != NULL)
		{
			Type* pointer = static_cast < Type* > (HeapAlloc(heap, 0, bytes));

			if (pointer != NULL)
			{
				if constexpr (std::is_nothrow_constructible_v < Type, ConstructorArguments... > == true) new(pointer) Type(std::forward < ConstructorArguments > (constructorArguments)...);
				else
				{
					try
					{
						new(pointer) Type(std::forward < ConstructorArguments > (constructorArguments)...);
					}
					catch (...)
					{
						HeapFree(heap, 0, static_cast < LPVOID > (pointer));

						throw;
					}
				}

				return SafePointer(heap, pointer);
			}
		}

		throw AllocationError(heap, bytes);
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SafeArrayPointer
	{
		HANDLE heap;
		std::size_t count;
		Type* pointer;

	public:
		constexpr SafeArrayPointer(HANDLE heap = NULL, std::size_t count = 0, Type* pointer = NULL) noexcept;
		SafeArrayPointer(const SafeArrayPointer&) = delete;
		constexpr SafeArrayPointer(SafeArrayPointer&& safeArrayPointer) noexcept;
		constexpr SafeArrayPointer(SafePointer < Type >&& safePointer) noexcept;
		~SafeArrayPointer() noexcept;
		SafeArrayPointer& operator = (const SafeArrayPointer&) = delete;
		SafeArrayPointer& operator = (SafeArrayPointer&& safeArrayPointer) noexcept;
		SafeArrayPointer& operator = (SafePointer < Type >&& safePointer) noexcept;
		[[nodiscard]] constexpr HANDLE Heap() const noexcept;
		[[nodiscard]] constexpr std::size_t Count() const noexcept;
		[[nodiscard]] constexpr Type* Get() const noexcept;
		[[nodiscard]] constexpr Type* Release() noexcept;
		bool Reset(HANDLE heap = NULL, std::size_t count = 0, Type* pointer = NULL) noexcept;
	};

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr SafeArrayPointer < Type >::SafeArrayPointer(HANDLE heap, std::size_t count, Type* pointer) noexcept :
		heap(NULL),
		count(0),
		pointer(NULL)
	{
		if (heap != NULL && pointer != NULL)
		{
			this->heap = heap;
			this->count = count;
			this->pointer = pointer;
		}
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr SafeArrayPointer < Type >::SafeArrayPointer(SafeArrayPointer&& safeArrayPointer) noexcept :
		heap(safeArrayPointer.heap),
		count(safeArrayPointer.count),
		pointer(safeArrayPointer.Release())
	{

	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr SafeArrayPointer < Type >::SafeArrayPointer(SafePointer < Type >&& safePointer) noexcept :
		heap(safePointer.Heap()),
		count(heap == NULL ? 0 : 1),
		pointer(safePointer.Release())
	{

	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SafeArrayPointer < Type >::~SafeArrayPointer() noexcept
	{
		if (heap != NULL)
		{
			for (Type* pointer = this->pointer; count != 0; count--, pointer++) pointer->~Type();

			HeapFree(heap, 0, static_cast < LPVOID > (pointer));
			heap = NULL;
			pointer = NULL;
		}
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SafeArrayPointer < Type >& SafeArrayPointer < Type >::operator = (SafeArrayPointer&& safeArrayPointer) noexcept
	{
		if (this != &safeArrayPointer)
		{
			this->~SafeArrayPointer();
			heap = safeArrayPointer.heap;
			count = safeArrayPointer.count;
			pointer = safeArrayPointer.Release();
		}

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SafeArrayPointer < Type >& SafeArrayPointer < Type >::operator = (SafePointer < Type >&& safePointer) noexcept
	{
		this->~SafeArrayPointer();
		heap = safePointer.Heap();
		count = heap == NULL ? 0 : 1;
		pointer = safePointer.Release();

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr HANDLE SafeArrayPointer < Type >::Heap() const noexcept
	{
		return heap;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr std::size_t SafeArrayPointer < Type >::Count() const noexcept
	{
		return count;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr Type* SafeArrayPointer < Type >::Get() const noexcept
	{
		return pointer;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr Type* SafeArrayPointer < Type >::Release() noexcept
	{
		Type* pointer = this->pointer;

		heap = NULL;
		count = 0;
		this->pointer = NULL;

		return pointer;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	bool SafeArrayPointer < Type >::Reset(HANDLE heap, std::size_t count, Type* pointer) noexcept
	{
		if (this->pointer == pointer) return this->heap == NULL ? true : false;

		this->~SafeArrayPointer();

		if (heap != NULL && pointer != NULL)
		{
			this->heap = heap;
			this->count = count;
			this->pointer = pointer;
		}

		return true;
	}

	template < typename Type >
		requires (std::is_default_constructible_v < Type > == true && std::is_nothrow_destructible_v < Type > == true)
	SafeArrayPointer < Type > MakeSafeArrayPointer(HANDLE heap, std::size_t count)
	{
		SIZE_T bytes = count * sizeof(Type);

		if (heap != NULL)
		{
			Type* pointer = static_cast < Type* > (HeapAlloc(heap, 0, bytes));

			if (pointer != NULL)
			{
				std::size_t countCopy = count;

				if constexpr (std::is_nothrow_default_constructible_v < Type > == true) for (Type* pointerCopy = pointer; countCopy != 0; countCopy--, pointerCopy++) new(pointerCopy) Type();
				else
				{
					try
					{
						for (Type* pointerCopy = pointer; countCopy != 0; countCopy--, pointerCopy++) new(pointerCopy) Type();
					}
					catch (...)
					{
						countCopy = count - countCopy;

						for (Type* pointerCopy = pointer; countCopy != 0; countCopy--, pointerCopy++) pointerCopy->~Type();

						HeapFree(heap, 0, static_cast < LPVOID > (pointer));

						throw;
					}
				}

				return SafeArrayPointer(heap, count, pointer);
			}
		}

		throw AllocationError(heap, bytes);
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SharedSafeArrayPointer;

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SharedSafePointer
	{
		template < typename Type >
			requires (std::is_nothrow_destructible_v < Type > == true)
		friend class SharedSafeArrayPointer;

		std::size_t* references;
		HANDLE heap;
		Type* pointer;

	public:
		SharedSafePointer(HANDLE heap = NULL, Type* pointer = NULL);
		constexpr SharedSafePointer(const SharedSafePointer& sharedSafePointer) noexcept;
		SharedSafePointer(SharedSafePointer&& sharedSafePointer) noexcept;
		~SharedSafePointer() noexcept;
		SharedSafePointer& operator = (const SharedSafePointer& sharedSafePointer) noexcept;
		SharedSafePointer& operator = (SharedSafePointer&& sharedSafePointer) noexcept;
		[[nodiscard]] constexpr std::size_t References() const noexcept;
		[[nodiscard]] constexpr HANDLE Heap() const noexcept;
		[[nodiscard]] constexpr Type* Get() const noexcept;
		[[nodiscard]] Type* Release() noexcept;
		bool Reset(HANDLE heap = NULL, Type* pointer = NULL);
	};

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafePointer < Type >::SharedSafePointer(HANDLE heap, Type* pointer) :
		references(NULL),
		heap(NULL),
		pointer(NULL)
	{
		if (heap != NULL && pointer != NULL)
		{
			constexpr SIZE_T bytes = sizeof(std::size_t);
			references = static_cast < std::size_t* > (HeapAlloc(heap, 0, bytes));

			if (references == NULL) throw AllocationError(heap, bytes);

			*references = 1;
			this->heap = heap;
			this->pointer = pointer;
		}		
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr SharedSafePointer < Type >::SharedSafePointer(const SharedSafePointer& sharedSafePointer) noexcept :
		references(sharedSafePointer.references),
		heap(sharedSafePointer.heap),
		pointer(sharedSafePointer.pointer)
	{
		if (references != NULL) (*references)++;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafePointer < Type >::SharedSafePointer(SharedSafePointer&& sharedSafePointer) noexcept :
		references(sharedSafePointer.references),
		heap(sharedSafePointer.heap),
		pointer(sharedSafePointer.Release())
	{

	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafePointer < Type >::~SharedSafePointer() noexcept
	{
		if (references != NULL)
		{
			(*references)--;

			if (*references == 0)
			{
				HeapFree(heap, 0, references);
				pointer->~Type();
				HeapFree(heap, 0, static_cast < LPVOID > (pointer));
			}

			references = NULL;
			heap = NULL;
			pointer = NULL;
		}
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafePointer < Type >& SharedSafePointer < Type >::operator = (const SharedSafePointer& sharedSafePointer) noexcept
	{
		if (this != &sharedSafePointer)
		{
			this->~SharedSafePointer();

			references = sharedSafePointer.references;
			heap = sharedSafePointer.heap;
			pointer = sharedSafePointer.pointer;
			if (references != NULL) (*references)++;
		}

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafePointer < Type >& SharedSafePointer < Type >::operator = (SharedSafePointer&& sharedSafePointer) noexcept
	{
		if (this != &sharedSafePointer)
		{
			this->~SharedSafePointer();

			references = sharedSafePointer.references;
			heap = sharedSafePointer.heap;
			pointer = sharedSafePointer.Release();
		}

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr std::size_t SharedSafePointer < Type >::References() const noexcept
	{
		return references == NULL ? 0 : *references;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr HANDLE SharedSafePointer < Type >::Heap() const noexcept
	{
		return heap;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr Type* SharedSafePointer < Type >::Get() const noexcept
	{
		return pointer;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	Type* SharedSafePointer < Type >::Release() noexcept
	{
		Type* pointer = this->pointer;

		if (references != NULL)
		{
			(*references)--;

			if (*references == 0) HeapFree(heap, 0, references);
		}

		references = NULL;
		heap = NULL;
		this->pointer = NULL;

		return pointer;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	bool SharedSafePointer < Type >::Reset(HANDLE heap, Type* pointer)
	{
		if (this->pointer == pointer) this->references == NULL ? true : false;

		this->~SharedSafePointer();

		if (heap != NULL && pointer != NULL)
		{
			constexpr SIZE_T bytes = sizeof(std::size_t);
			references = static_cast < std::size_t* > (HeapAlloc(heap, 0, bytes));

			if (references == NULL) throw AllocationError(heap, bytes);

			*references = 1;
			this->heap = heap;
			this->pointer = pointer;
		}

		return true;
	}

	template < typename Type, typename... ConstructorArguments >
		requires (std::is_constructible_v < Type, ConstructorArguments... > == true && std::is_nothrow_destructible_v < Type > == true)
	SharedSafePointer < Type > MakeSharedSafePointer(HANDLE heap, ConstructorArguments&&... constructorArguments)
	{
		constexpr SIZE_T bytes = sizeof(Type);

		if (heap != NULL)
		{
			Type* pointer = static_cast < Type* > (HeapAlloc(heap, 0, bytes));

			if (pointer != NULL)
			{
				if constexpr (std::is_nothrow_constructible_v < Type, ConstructorArguments... > == true) new(pointer) Type(std::forward < ConstructorArguments > (constructorArguments)...);
				else
				{
					try
					{
						new(pointer) Type(std::forward < ConstructorArguments > (constructorArguments)...);
					}
					catch (...)
					{
						HeapFree(heap, 0, static_cast < LPVOID > (pointer));

						throw;
					}
				}

				try
				{
					return SharedSafePointer(heap, pointer);
				}
				catch (const AllocationError&)
				{
					pointer->~Type();
					HeapFree(heap, 0, static_cast < LPVOID > (pointer));

					throw;
				}
			}
		}

		throw AllocationError(heap, bytes);
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SharedSafeArrayPointer
	{
		std::size_t* references;
		HANDLE heap;
		std::size_t count;
		Type* pointer;

	public:
		SharedSafeArrayPointer(HANDLE heap = NULL, std::size_t count = 0, Type* pointer = NULL);
		constexpr SharedSafeArrayPointer(const SharedSafeArrayPointer& sharedSafeArrayPointer) noexcept;
		constexpr SharedSafeArrayPointer(const SharedSafePointer < Type >& sharedSafePointer) noexcept;
		SharedSafeArrayPointer(SharedSafeArrayPointer&& sharedSafeArrayPointer) noexcept;
		SharedSafeArrayPointer(SharedSafePointer < Type >&& sharedSafePointer) noexcept;
		~SharedSafeArrayPointer() noexcept;
		SharedSafeArrayPointer& operator = (const SharedSafeArrayPointer& sharedSafeArrayPointer) noexcept;
		SharedSafeArrayPointer& operator = (const SharedSafePointer < Type >& sharedSafePointer) noexcept;
		SharedSafeArrayPointer& operator = (SharedSafeArrayPointer&& sharedSafeArrayPointer) noexcept;
		SharedSafeArrayPointer& operator = (SharedSafePointer < Type >&& sharedSafePointer) noexcept;
		[[nodiscard]] constexpr std::size_t References() const noexcept;
		[[nodiscard]] constexpr HANDLE Heap() const noexcept;
		[[nodiscard]] constexpr std::size_t Count() const noexcept;
		[[nodiscard]] constexpr Type* Get() const noexcept;
		[[nodiscard]] Type* Release() noexcept;
		bool Reset(HANDLE heap = NULL, std::size_t count = 0, Type* pointer = NULL);
	};

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type >::SharedSafeArrayPointer(HANDLE heap, std::size_t count, Type* pointer) :
		references(NULL),
		heap(NULL),
		count(0),
		pointer(NULL)
	{
		if (heap != NULL && pointer != NULL)
		{
			constexpr SIZE_T bytes = sizeof(std::size_t);
			references = static_cast < std::size_t* > (HeapAlloc(heap, 0, bytes));

			if (references == NULL) throw AllocationError(heap, bytes);

			*references = 1;
			this->heap = heap;
			this->count = count;
			this->pointer = pointer;
		}
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr SharedSafeArrayPointer < Type >::SharedSafeArrayPointer(const SharedSafeArrayPointer& sharedSafeArrayPointer) noexcept :
		references(sharedSafeArrayPointer.references),
		heap(sharedSafeArrayPointer.heap),
		count(sharedSafeArrayPointer.count),
		pointer(sharedSafeArrayPointer.pointer)
	{
		if (references != NULL) (*references)++;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr SharedSafeArrayPointer < Type >::SharedSafeArrayPointer(const SharedSafePointer < Type >& sharedSafePointer) noexcept :
		references(sharedSafePointer.references),
		heap(sharedSafePointer.Heap()),
		count(references == NULL ? 0 : 1),
		pointer(sharedSafePointer.Get())
	{
		if (references != NULL) (*references)++;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type >::SharedSafeArrayPointer(SharedSafeArrayPointer&& sharedSafeArrayPointer) noexcept :
		references(sharedSafeArrayPointer.references),
		heap(sharedSafeArrayPointer.heap),
		count(sharedSafeArrayPointer.count),
		pointer(sharedSafeArrayPointer.Release())
	{

	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type >::SharedSafeArrayPointer(SharedSafePointer < Type >&& sharedSafePointer) noexcept :
		references(sharedSafePointer.references),
		heap(sharedSafePointer.Heap()),
		count(references == NULL ? 0 : 1),
		pointer(sharedSafePointer.Release())
	{

	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type >::~SharedSafeArrayPointer() noexcept
	{
		if (references != NULL)
		{
			(*references)--;

			if (*references == 0)
			{
				HeapFree(heap, 0, references);
			
				for (Type* pointer = this->pointer; count != 0; count--, pointer++) pointer->~Type();

				HeapFree(heap, 0, static_cast < LPVOID > (pointer));
			}

			references = NULL;
			heap = NULL;
			count = 0;
			pointer = NULL;
		}
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type >& SharedSafeArrayPointer < Type >::operator = (const SharedSafeArrayPointer& sharedSafeArrayPointer) noexcept
	{
		if (this != &sharedSafeArrayPointer)
		{
			this->~SharedSafeArrayPointer();
			references = sharedSafeArrayPointer.references;
			heap = sharedSafeArrayPointer.heap;
			count = sharedSafeArrayPointer.count;
			pointer = sharedSafeArrayPointer.pointer;

			if (references != NULL) (*references)++;
		}

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type >& SharedSafeArrayPointer < Type >::operator = (const SharedSafePointer < Type >& sharedSafePointer) noexcept
	{
		this->~SharedSafeArrayPointer();
		references = sharedSafePointer.References();
		heap = sharedSafePointer.Heap();
		count = references == NULL ? 0 : 1;
		pointer = sharedSafePointer.Get();

		if (references != NULL) (*references)++;

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type >& SharedSafeArrayPointer < Type >::operator = (SharedSafeArrayPointer&& sharedSafeArrayPointer) noexcept
	{
		if (this != &sharedSafeArrayPointer)
		{
			this->~SharedSafeArrayPointer();
			references = sharedSafeArrayPointer.references;
			heap = sharedSafeArrayPointer.heap;
			count = sharedSafeArrayPointer.count;
			pointer = sharedSafeArrayPointer.Release();
		}

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type >& SharedSafeArrayPointer < Type >::operator = (SharedSafePointer < Type >&& sharedSafePointer) noexcept
	{
		if (this != &sharedSafePointer)
		{
			this->~SharedSafeArrayPointer();
			references = sharedSafePointer.References();
			heap = sharedSafePointer.Heap();
			count = references == NULL ? 0 : 1;
			pointer = sharedSafePointer.Release();
		}

		return *this;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr std::size_t SharedSafeArrayPointer < Type >::References() const noexcept
	{
		return references == NULL ? 0 : *references;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr HANDLE SharedSafeArrayPointer < Type >::Heap() const noexcept
	{
		return heap;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr std::size_t SharedSafeArrayPointer < Type >::Count() const noexcept
	{
		return count;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	constexpr Type* SharedSafeArrayPointer < Type >::Get() const noexcept
	{
		return pointer;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	Type* SharedSafeArrayPointer < Type >::Release() noexcept
	{
		Type* pointer = this->pointer;

		if (references != NULL)
		{
			(*references)--;

			if (*references == 0) HeapFree(heap, 0, references);
		}

		references = NULL;
		heap = NULL;
		count = 0;
		this->pointer = NULL;

		return pointer;
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	bool SharedSafeArrayPointer < Type >::Reset(HANDLE heap, std::size_t count, Type* pointer)
	{
		if (this->pointer == pointer) this->references == NULL ? true : false;

		this->~SharedSafeArrayPointer();

		if (heap != NULL && pointer != NULL)
		{
			constexpr SIZE_T bytes = sizeof(std::size_t);
			references = static_cast < std::size_t* > (HeapAlloc(heap, 0, bytes));

			if (references == NULL) throw AllocationError(heap, bytes);

			*references = 1;
			this->heap = heap;
			this->count = count;
			this->pointer = pointer;
		}

		return true;
	}

	template < typename Type, typename... ConstructorArguments >
		requires (std::is_default_constructible_v < Type > == true && std::is_nothrow_destructible_v < Type > == true)
	SharedSafeArrayPointer < Type > MakeSharedSafeArrayPointer(HANDLE heap, std::size_t count)
	{
		SIZE_T bytes = count * sizeof(Type);

		if (heap != NULL)
		{
			Type* pointer = static_cast < Type* > (HeapAlloc(heap, 0, bytes));

			if (pointer != NULL)
			{
				std::size_t countCopy = count;

				if constexpr (std::is_nothrow_default_constructible_v < Type > == true) for (Type* pointerCopy = pointer; countCopy != 0; countCopy--, pointerCopy++) new(pointerCopy) Type();
				else
				{
					try
					{
						for (Type* pointerCopy = pointer; countCopy != 0; countCopy--, pointerCopy++) new(pointerCopy) Type();
					}
					catch (...)
					{
						countCopy = count - countCopy;

						for (Type* pointerCopy = pointer; countCopy != 0; countCopy--, pointerCopy++) pointerCopy->~Type();

						HeapFree(heap, 0, static_cast < LPVOID > (pointer));

						throw;
					}
				}

				try
				{
					return SharedSafeArrayPointer(heap, count, pointer);
				}
				catch (const AllocationError&)
				{
					countCopy = count - countCopy;

					for (Type* pointerCopy = pointer; countCopy != 0; countCopy--, pointerCopy++) pointerCopy->~Type();

					HeapFree(heap, 0, static_cast < LPVOID > (pointer));

					throw;
				}
			}
		}

		throw AllocationError(heap, bytes);
	}
}