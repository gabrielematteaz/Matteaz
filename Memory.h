#ifndef MATTEAZ_MEMORY_H
#define MATTEAZ_MEMORY_H

#include < new >

#include "Pointer.h"
#include "ArrayPointer.h"
#include "SharedPointer.h"
#include "SharedArrayPointer.h"

namespace Matteaz
{
	template < typename Type, typename... ConstructorArguments >
		requires (std::is_constructible_v < Type, ConstructorArguments... > == true)
	Pointer < Type > MakePointer(HANDLE heap, ConstructorArguments&&... constructorArguments)
	{
		SIZE_T bytes = sizeof(Type);

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

				return Pointer(heap, pointer);
			}
		}

		throw AllocationError(heap, bytes);
	}

	template < typename Type >
		requires (std::is_default_constructible_v < Type > == true)
	ArrayPointer < Type > MakeArrayPointer(HANDLE heap, std::size_t count)
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
						count = count - countCopy;

						if constexpr (std::is_trivially_destructible_v < Type > == false) for (Type* pointerCopy = pointer; count != 0; count--, pointerCopy++) pointerCopy->~Type();

						HeapFree(heap, 0, static_cast < LPVOID > (pointer));

						throw;
					}
				}

				return ArrayPointer(heap, count, pointer);
			}
		}

		throw AllocationError(heap, bytes);
	}

	template < typename Type, typename... ConstructorArguments >
		requires (std::is_constructible_v < Type, ConstructorArguments... > == true)
	SharedPointer < Type > MakeSharedPointer(HANDLE heap, ConstructorArguments&&... constructorArguments)
	{
		SIZE_T bytes = sizeof(Type);

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
					return SharedPointer(heap, pointer);
				}
				catch (const AllocationError&)
				{
					if constexpr (std::is_trivially_destructible_v < Type > == false) pointer->~Type();

					HeapFree(heap, 0, static_cast < LPVOID > (pointer));

					throw;
				}
			}
		}

		throw AllocationError(heap, bytes);
	}

	template < typename Type >
		requires (std::is_default_constructible_v < Type > == true)
	SharedArrayPointer < Type > MakeSharedArrayPointer(HANDLE heap, std::size_t count)
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
						count = count - countCopy;

						if constexpr (std::is_trivially_destructible_v < Type > == false) for (Type* pointerCopy = pointer; count != 0; count--, pointerCopy++) pointerCopy->~Type();

						HeapFree(heap, 0, static_cast < LPVOID > (pointer));

						throw;
					}
				}

				try
				{
					return SharedArrayPointer(heap, count, pointer);
				}
				catch (const AllocationError&)
				{
					if constexpr (std::is_trivially_destructible_v < Type > == false) for (Type* pointerCopy = pointer; count != 0; count--, pointerCopy++) pointerCopy->~Type();

					HeapFree(heap, 0, static_cast < LPVOID > (pointer));

					throw;
				}
			}
		}

		throw AllocationError(heap, bytes);
	}
}

#endif