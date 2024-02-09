#pragma once

#include "Exception.h"
#include < new >

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
		virtual ~AllocationError() noexcept = default;
		AllocationError& operator = (const AllocationError&) noexcept = default;
		AllocationError& operator = (AllocationError&&) noexcept = default;
	};

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	class SafePointer
	{
		HANDLE heap;
		Type* pointer;

	public:
		SafePointer(const SafePointer&) = delete;
		~SafePointer() noexcept;
		SafePointer& operator = (const SafePointer&) = delete;
		SafePointer& operator = (SafePointer&& safePointer) noexcept;
		template < typename TypeCopy = Type >
			requires (std::is_copy_constructible_v < TypeCopy > == true)
		SafePointer Copy();
		template < typename TypeCopy = Type >
			requires (std::is_copy_constructible_v < TypeCopy > == true)
		SafePointer Copy(const std::nothrow_t&) noexcept;
		bool Reset(HANDLE heap = NULL, Type* pointer = NULL) noexcept;

		constexpr SafePointer(HANDLE heap = NULL, Type* pointer = NULL) noexcept :
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
			pointer(safePointer.Release())
		{

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
	};

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	inline SafePointer < Type >::~SafePointer() noexcept
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
	inline SafePointer < Type >& SafePointer < Type >::operator = (SafePointer < Type >&& safePointer) noexcept
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
	template < typename TypeCopy >
		requires (std::is_copy_constructible_v < TypeCopy > == true)
	inline SafePointer < Type > SafePointer < Type >::Copy()
	{
		constexpr SIZE_T bytes = sizeof(Type);

		if (heap == NULL) throw AllocationError(NULL, bytes);

		Type* pointer = static_cast < Type* > (HeapAlloc(heap, 0, bytes));

		if (pointer == NULL) throw AllocationError(heap, bytes);

		if constexpr (std::is_nothrow_copy_constructible_v < Type > == true) new(pointer) Type(*this->pointer);
		else
		{
			try
			{
				new(pointer) Type(*this->pointer);
			}
			catch (...)
			{
				HeapFree(heap, 0, static_cast < LPVOID > (pointer));

				throw;
			}
		}

		return SafePointer(heap, pointer);
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	template < typename TypeCopy >
		requires (std::is_copy_constructible_v < TypeCopy > == true)
	inline SafePointer < Type > SafePointer < Type >::Copy(const std::nothrow_t&) noexcept
	{
		Type* pointer = NULL;

		if (heap != NULL)
		{
			pointer = static_cast < Type* > (HeapAlloc(heap, 0, sizeof(Type)));

			if (pointer != NULL)
			{
				if constexpr (std::is_nothrow_copy_constructible_v < Type > == true) new(pointer) Type(*this->pointer);
				else
				{
					try
					{
						new(pointer) Type(*this->pointer);
					}
					catch (...)
					{
						HeapFree(heap, 0, static_cast < LPVOID > (pointer));
						pointer = NULL;
					}
				}
			}
		}

		return SafePointer(heap, pointer);
	}

	template < typename Type >
		requires (std::is_nothrow_destructible_v < Type > == true)
	inline bool SafePointer < Type >::Reset(HANDLE heap, Type* pointer) noexcept
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

		if (heap == NULL) throw AllocationError(NULL, bytes);

		Type* pointer = static_cast < Type* > (HeapAlloc(heap, 0, bytes));

		if (pointer == NULL) throw AllocationError(heap, bytes);

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