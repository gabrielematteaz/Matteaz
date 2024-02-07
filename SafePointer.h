#pragma once

#include < new >
#include "Exception.h"

namespace Matteaz
{
	class AllocationError : private Exception
	{
	public:
		AllocationError() = delete;
		AllocationError(const AllocationError&) noexcept = default;
		constexpr AllocationError(AllocationError&) noexcept = default;
		virtual ~AllocationError() = default;
		AllocationError& operator = (const AllocationError&) noexcept = default;
		AllocationError& operator = (AllocationError&&) noexcept = default;

		HANDLE heap;
		SIZE_T bytes;

		AllocationError(HANDLE heap, SIZE_T bytes) noexcept :
			Exception(),
			heap(heap),
			bytes(bytes)
		{

		}
	};

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SafePointer
	{
		HANDLE heap;
		Type* pointer;

	public:
		SafePointer(const SafePointer&) = delete;
		SafePointer& operator = (const SafePointer&) = delete;

		constexpr explicit SafePointer(HANDLE heap = NULL, Type* pointer = NULL) noexcept :
			heap(NULL),
			pointer(NULL)
		{
			if (heap != NULL && pointer != NULL)
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
			if (heap != NULL)
			{
				pointer->~Type();
				HeapFree(heap, 0, static_cast < LPVOID > (pointer));
				heap = NULL;
				pointer = NULL;
			}
		}

		SafePointer& operator = (SafePointer&& safePointer)
		{
			if (this != &safePointer)
			{
				if (heap != NULL)
				{
					pointer->~Type();
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

		[[nodiscard]] constexpr Type* Release() noexcept
		{
			Type* pointer = this->pointer;

			heap = NULL;
			this->pointer = NULL;

			return pointer;
		}

		bool Reset(HANDLE heap = NULL, Type* pointer = NULL) noexcept
		{
			if (this->heap == heap) return this->heap == NULL ? true : false;

			if (this->heap != NULL)
			{
				this->pointer->~Type();
				HeapFree(this->heap, 0, static_cast < LPVOID > (this->pointer));
			}

			if (heap == NULL || pointer == NULL)
			{
				this->heap = NULL;
				this->pointer = NULL;
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
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SharedSafePointer
	{
		HANDLE heap;
		Type* pointer;
		std::size_t* references;

	public:
		explicit SharedSafePointer(HANDLE heap = NULL, Type* pointer = NULL) :
			heap(NULL),
			pointer(NULL),
			references(NULL)
		{
			if (heap != NULL && pointer != NULL)
			{
				this->heap = heap;
				this->pointer = pointer;
				references = static_cast < std::size_t* > (HeapAlloc(heap, 0, sizeof(std::size_t)));

				if (references == NULL) throw AllocationError(heap, sizeof(std::size_t));

				*references = 1;
			}
		}

		constexpr SharedSafePointer(const SharedSafePointer& sharedSafePointer) noexcept :
			heap(sharedSafePointer.heap),
			pointer(sharedSafePointer.pointer),
			references(sharedSafePointer.references)
		{
			(*references)++;
		}

		constexpr SharedSafePointer(SharedSafePointer&& sharedSafePointer) noexcept :
			heap(sharedSafePointer.heap),
			pointer(sharedSafePointer.pointer),
			references(sharedSafePointer.references)
		{
			sharedSafePointer.heap = NULL;
			sharedSafePointer.pointer = NULL;
			sharedSafePointer.references = NULL;
		}

		~SharedSafePointer()
		{
			if (heap != NULL)
			{
				(*references)--;

				if (*references == 0)
				{
					pointer->~Type();
					HeapFree(heap, 0, static_cast < LPVOID > (pointer));
					HeapFree(heap, 0, references);
					heap = NULL;
					pointer = NULL;
					references = NULL;
				}
			}
		}

		SharedSafePointer& operator = (const SharedSafePointer& sharedSafePointer) noexcept
		{
			if (this != &sharedSafePointer)
			{
				if (heap != NULL)
				{
					(*references)--;

					if (*references == 0)
					{
						pointer->~Type();
						HeapFree(heap, 0, static_cast < LPVOID > (pointer));
						HeapFree(heap, 0, references);
					}
				}

				heap = sharedSafePointer.heap;
				pointer = sharedSafePointer.pointer;
				references = sharedSafePointer.references;
				(*references)++;
			}

			return *this;
		}

		SharedSafePointer& operator = (SharedSafePointer&& sharedSafePointer) noexcept
		{
			if (this != &sharedSafePointer)
			{
				if (heap != NULL)
				{
					(*references)--;

					if (*references == 0)
					{
						pointer->~Type();
						HeapFree(heap, 0, static_cast < LPVOID > (pointer));
						HeapFree(heap, 0, references);
					}
				}

				heap = sharedSafePointer.heap;
				pointer = sharedSafePointer.pointer;
				references = sharedSafePointer.references;
				sharedSafePointer.heap = NULL;
				sharedSafePointer.pointer = NULL;
				sharedSafePointer.references = NULL;
			}

			return *this;
		}

		[[nodiscard]] constexpr HANDLE Heap() const noexcept
		{
			return heap;
		}

		[[nodiscard]] constexpr std::size_t References() const noexcept
		{
			return *references;
		}

		[[nodiscard]] constexpr Type* Get() const noexcept
		{
			return pointer;
		}

		[[nodiscard]] Type* Release() noexcept
		{
			Type* pointer = this->pointer;

			heap = NULL;
			this->pointer = NULL;
			(*references)--;

			if (*references == 0) HeapFree(heap, 0, references);

			references = NULL;

			return pointer;
		}

		bool Reset(HANDLE heap = NULL, Type* pointer = NULL)
		{
			if (this->pointer == pointer) return this->heap == NULL ? true : false;

			if (heap != NULL)
			{
				(*references)--;

				if (*references == 0)
				{
					this->pointer->~Type();
					HeapFree(this->heap, 0, static_cast < LPVOID > (this->pointer));
					HeapFree(this->heap, 0, references);
				}
			}

			if (heap == NULL || pointer == NULL)
			{
				this->heap = NULL;
				this->pointer = NULL;
				references = NULL;
			}
			else
			{
				references = static_cast < std::size_t* > (HeapAlloc(heap, 0, sizeof(std::size_t)));

				if (references == NULL)
				{
					this->heap = NULL;
					this->pointer = NULL;

					throw AllocationError(heap, sizeof(std::size_t));
				}

				this->heap = heap;
				this->pointer = pointer;
				*references = 1;
			}

			return true;
		}
	};

	template < typename Type, typename... ConstructorArguments >
		requires (std::is_nothrow_constructible_v < Type, ConstructorArguments... > == true || (std::is_constructible_v < Type, ConstructorArguments... > == true && std::is_nothrow_destructible_v < Type > == true))
	SafePointer < Type > MakeSafePointer(HANDLE heap, ConstructorArguments&&... constructorArguments)
	{
		Type* pointer = static_cast < Type* > (HeapAlloc(heap, 0, sizeof(Type)));

		if (pointer == NULL) throw AllocationError(heap, sizeof(Type));

		if constexpr (std::is_nothrow_constructible_v < Type, ConstructorArguments... > == true)
		{
			new(pointer) Type(std::forward < ConstructorArguments > (constructorArguments)...);
		}
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

	template < typename Type, typename... ConstructorArguments >
		requires (std::is_nothrow_constructible_v < Type, ConstructorArguments... > == true || (std::is_constructible_v < Type, ConstructorArguments... > == true && std::is_nothrow_destructible_v < Type > == true))
	SharedSafePointer < Type > MakeSharedSafePointer(HANDLE heap, ConstructorArguments&&... constructorArguments)
	{
		Type* pointer = static_cast < Type* > (HeapAlloc(heap, 0, sizeof(Type)));

		if (pointer == NULL) throw AllocationError(heap, sizeof(Type));

		if constexpr (std::is_nothrow_constructible_v < Type, ConstructorArguments... > == true)
		{
			new(pointer) Type(std::forward < ConstructorArguments > (constructorArguments)...);
		}
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
			HeapFree(heap, 0, static_cast < LPVOID > (pointer));

			throw;
		}
	}
}