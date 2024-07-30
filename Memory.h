#pragma once

#include < memory >
#include "Error.h"

namespace Matteaz
{
	struct AllocationError : Exception
	{
		SIZE_T bytes;

		constexpr AllocationError(SIZE_T bytes) noexcept :
			Exception(L"Matteaz::AllocationError"), bytes(bytes)
		{

		}
	};

	template < typename Type >
	class Allocator
	{
		template < typename OtherType >
		friend class Allocator;

		HANDLE heap;
		std::size_t *references;

	public:
		using native_handle_type = HANDLE;
		using value_type = Type;
		using size_type = SIZE_T;
		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap = std::true_type;

		Allocator() :
			heap(GetProcessHeap()), references(nullptr)
		{
			if (heap == NULL) throw SystemError(L"GetProcessHeap failed");
		}

		explicit Allocator(SIZE_T initialSize, SIZE_T maximumSize) :
			heap(HeapCreate(0, initialSize, maximumSize)), references(heap == NULL ? throw SystemError(L"HeapCreate failed") : static_cast < std::size_t* > (HeapAlloc(heap, 0, sizeof(std::size_t))))
		{
			if (references == nullptr) {
				HeapDestroy(heap);
				throw AllocationError(sizeof(std::size_t));
			}

			*references = 1;
		}

		constexpr Allocator(const Allocator &other) noexcept :
			heap(other.heap), references(other.references)
		{
			if (references == nullptr) return;

			++*references;
		}

		template < typename OtherType >
		constexpr Allocator(const Allocator < OtherType > &other) noexcept :
			heap(other.heap), references(other.references)
		{
			if (references == nullptr) return;

			++*references;
		}

		constexpr ~Allocator()
		{
			if (references == nullptr) return;

			--*references;

			if (*references == 0) {
				HeapFree(heap, 0, references);
				HeapDestroy(heap);
			}
		}

		constexpr Allocator &operator = (Allocator other) noexcept
		{
			swap(*this, other);

			return *this;
		}

		[[nodiscard]] constexpr native_handle_type native_handle() const noexcept
		{
			return heap;
		}

		[[nodiscard]] constexpr std::size_t use_count() const noexcept
		{
			return references == nullptr ? 0 : *references;
		}

		[[nodiscard]] value_type *allocate(SIZE_T count) const
		{
			auto bytes = count * sizeof(value_type);
			auto pointer = static_cast < value_type* > (HeapAlloc(heap, 0, bytes));

			return pointer == nullptr ? throw AllocationError(bytes) : pointer;
		}

		void deallocate(value_type *pointer, SIZE_T) const noexcept
		{
			HeapFree(heap, 0, pointer);
		}

		friend constexpr void swap(Allocator &left, Allocator &right) noexcept
		{
			std::swap(left.heap, right.heap);
			std::swap(left.references, right.references);
		}
	};

	template < >
	void *Allocator < void >::allocate(SIZE_T count) const
	{
		auto pointer = HeapAlloc(heap, 0, count);

		return pointer == nullptr ? throw AllocationError(count) : pointer;
	}

	template < typename Type, typename OtherType >
	[[nodiscard]] constexpr bool operator == (const Allocator < Type > &left, const Allocator < OtherType > &right) noexcept
	{
		return left.native_handle() == right.native_handle();
	}

	template < typename Type, typename AllocatorType = Allocator < Type > >
		requires std::is_nothrow_destructible_v < Type > && std::is_same_v < Type, typename AllocatorType::value_type >
	class SharedMemoryResource
	{
		using AllocatorTraits = std::allocator_traits < AllocatorType >;
		using SizeAllocatorTraits = std::allocator_traits < typename AllocatorTraits::template rebind_alloc < std::size_t > >;

		AllocatorType allocator;
		SizeAllocatorTraits::pointer references;
		AllocatorTraits::pointer resource;

	public:
		using element_type = Type;
		using allocator_type = AllocatorType;
		using pointer = AllocatorTraits::pointer;

		template < typename... ConstructorParametersType >
			requires std::is_constructible_v < element_type, ConstructorParametersType... >
		explicit SharedMemoryResource(const allocator_type &allocator = allocator_type(), ConstructorParametersType&&... constructorParameters) :
			allocator(allocator), references(nullptr), resource(nullptr)
		{
			typename SizeAllocatorTraits::allocator_type sizeAllocator(allocator);

			try {
				references = SizeAllocatorTraits::allocate(sizeAllocator, 1);
				resource = AllocatorTraits::allocate(this->allocator, 1);
				new(resource) element_type(std::forward < ConstructorParametersType > (constructorParameters)...);
			}
			catch (...) {
				SizeAllocatorTraits::deallocate(sizeAllocator, references, 1);
				AllocatorTraits::deallocate(this->allocator, resource, 1);
				throw;
			}

			*references = 1;
		}

		constexpr SharedMemoryResource(const SharedMemoryResource &other) noexcept :
			allocator(AllocatorTraits::select_on_container_copy_construction(other.allocator)), references(other.references), resource(other.resource)
		{
			if (references == nullptr) return;

			++*references;
		}

		constexpr SharedMemoryResource(SharedMemoryResource &&other) noexcept :
			allocator(std::move(other.allocator)), references(other.references), resource(other.resource)
		{
			other.references = nullptr;
			other.resource = nullptr;
		}

		constexpr ~SharedMemoryResource()
		{
			if (references == nullptr) return;

			--*references;

			if (*references == 0) {
				if constexpr (!std::is_trivially_destructible_v < element_type >) resource->~element_type();

				typename SizeAllocatorTraits::allocator_type sizeAllocator(allocator);

				SizeAllocatorTraits::deallocate(sizeAllocator, references, 1);
				AllocatorTraits::deallocate(allocator, resource, 1);
			}
		}

		constexpr SharedMemoryResource &operator = (SharedMemoryResource other) noexcept
		{
			swap(*this, other);

			return *this;
		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return resource != nullptr;
		}

		[[nodiscard]] constexpr pointer operator -> () const
		{
			return resource == nullptr ? throw Exception(L"tried to access a member of a null resource") : resource;
		}

		[[nodiscard]] constexpr element_type &operator * () const
		{
			return resource == nullptr ? throw Exception(L"tried to dereference a null resource") : *resource;
		}

		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept
		{
			return allocator;
		}

		[[nodiscard]] constexpr std::size_t use_count() const noexcept
		{
			return references == nullptr ? 0 : *references;
		}

		[[nodiscard]] constexpr pointer get() const noexcept
		{
			return resource;
		}

		friend constexpr void swap(SharedMemoryResource &left, SharedMemoryResource &right) noexcept
		{
			using std::swap;

			if constexpr (std::is_same_v < AllocatorTraits::propagate_on_container_swap, std::true_type >) swap(left.allocator, right.allocator);

			swap(left.references, right.references);
			swap(left.resource, right.resource);
		}
	};

	template < typename Type, typename AllocatorType >
	[[nodiscard]] constexpr bool operator == (const SharedMemoryResource < Type, AllocatorType > &left, const SharedMemoryResource < Type, AllocatorType > &right) noexcept
	{
		return left.get() == right.get();
	}
}

namespace std
{
	template < typename Type, typename AllocatorType >
	struct hash < Matteaz::SharedMemoryResource < Type, AllocatorType > >
	{
		[[nodiscard]] std::size_t operator () (const Matteaz::SharedMemoryResource < Type, AllocatorType > &sharedMemoryResource) const noexcept
		{
			return std::hash < decltype(sharedMemoryResource.get()) > { } (sharedMemoryResource.get());
		}
	};
}
