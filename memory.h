#pragma once

#include < memory >
#include "error.h"

namespace matteaz
{
	struct allocation_error : basic_exception
	{
		SIZE_T bytes;

		constexpr allocation_error(SIZE_T bytes) noexcept :
			basic_exception(L"matteaz::allocation_error"), bytes(bytes)
		{

		}
	};

	template < typename type_ >
	class allocator
	{
		template < typename other_type_ >
		friend class allocator;

		HANDLE heap_;
		std::size_t *references_;

	public:
		using native_handle_type = HANDLE;

		using value_type = type_;
		using size_type = SIZE_T;
		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap = std::true_type;

		allocator() :
			heap_(GetProcessHeap()), references_(nullptr)
		{
			if (heap_ == NULL) throw DWORD_error(L"GetProcessHeap failed");
		}

		explicit allocator(SIZE_T initial_size, SIZE_T maximum_size) :
			heap_(HeapCreate(0, initial_size, maximum_size))
		{
			if (heap_ == NULL) throw DWORD_error(L"HeapCreate failed");

			references_ = static_cast < std::size_t * > (HeapAlloc(heap_, 0, sizeof(std::size_t)));

			if (references_ == nullptr) {
				HeapDestroy(heap_);
				throw allocation_error(sizeof(std::size_t));
			}

			*references_ = 1;
		}

		constexpr allocator(const allocator &other) noexcept :
			heap_(other.heap_), references_(other.references_)
		{
			if (references_ == nullptr) return;

			++*references_;
		}

		template < typename other_type_ >
		constexpr allocator(const allocator < other_type_ > &other) noexcept :
			heap_(other.heap_), references_(other.references_)
		{
			if (references_ == nullptr) return;

			++*references_;
		}

		constexpr ~allocator()
		{
			if (references_ == nullptr) return;

			--*references_;

			if (*references_ == 0) {
				HeapFree(heap_, 0, references_);
				HeapDestroy(heap_);
			}
		}

		constexpr allocator &operator = (allocator other) noexcept
		{
			swap(*this, other);

			return *this;
		}

		[[nodiscard]] constexpr HANDLE native_handle() const noexcept
		{
			return heap_;
		}

		[[nodiscard]] constexpr std::size_t use_count() const noexcept
		{
			return references_ == nullptr ? 0 : *references_;
		}

		[[nodiscard]] type_ *allocate(SIZE_T count) const
		{
			auto bytes = count * sizeof(type_);
			auto pointer = static_cast < type_ * > (HeapAlloc(heap_, 0, bytes));

			return pointer == nullptr ? throw allocation_error(bytes) : pointer;
		}

		void deallocate(type_ *pointer, SIZE_T) const noexcept
		{
			HeapFree(heap_, 0, pointer);
		}

		friend constexpr void swap(allocator &left, allocator &right) noexcept
		{
			std::swap(left.heap_, right.heap_);
			std::swap(left.references_, right.references_);
		}
	};

	template < typename type_, typename other_type_ >
	[[nodiscard]] constexpr bool operator == (const allocator < type_ > &left, const allocator < other_type_ > &right) noexcept
	{
		return left.native_handle() == right.native_handle();
	}

	template < typename type_, typename allocator_type_ = allocator < type_ > >
		requires std::is_nothrow_destructible_v < type_ > && std::is_same_v < type_, typename allocator_type_::value_type >
	class shared_memory_resource
	{
		using allocator_traits = std::allocator_traits < allocator_type_ >;
		using size_allocator_traits = std::allocator_traits < typename allocator_traits::template rebind_alloc < std::size_t > >;

		allocator_type_ allocator_;
		size_allocator_traits::pointer references_;
		allocator_traits::pointer resource_;

	public:
		using element_type = type_;
		using allocator_type = allocator_type_;

		template < typename... parameters_type_ >
			requires std::is_constructible_v < type_, parameters_type_... >
		explicit shared_memory_resource(const allocator_type_ &allocator = allocator_type_(), parameters_type_&&... parameters) :
			allocator_(allocator), references_(nullptr), resource_(nullptr)
		{
			typename size_allocator_traits::allocator_type size_allocator(allocator);

			try {
				references_ = size_allocator.allocate(1);
				resource_ = allocator_.allocate(1);
				new(resource_) type_(std::forward < parameters_type_ > (parameters)...);
				*references_ = 1;
			}
			catch (...) {
				size_allocator.deallocate(references_, 1);
				allocator_.deallocate(resource_, 1);
				throw;
			}
		}

		constexpr shared_memory_resource(const shared_memory_resource &other) noexcept :
			allocator_(allocator_traits::select_on_container_copy_construction(other.allocator_)), references_(other.references_), resource_(other.resource_)
		{
			if (references_ == nullptr) return;

			++*references_;
		}

		constexpr shared_memory_resource(shared_memory_resource &&other) noexcept :
			allocator_(std::move(other.allocator_)), references_(other.references_), resource_(other.resource_)
		{
			other.references_ = nullptr;
			other.resource_ = nullptr;
		}

		constexpr ~shared_memory_resource()
		{
			if (references_ == nullptr) return;

			--*references_;

			if (*references_ == 0) {
				if constexpr (!std::is_trivially_destructible_v < type_ >) resource_->~type_();

				typename size_allocator_traits::allocator_type size_allocator(allocator_);

				size_allocator.deallocate(references_, 1);
				allocator_.deallocate(resource_, 1);
			}
		}

		constexpr shared_memory_resource &operator = (shared_memory_resource other) noexcept
		{
			swap(*this, other);

			return *this;
		}

		[[nodiscard]] constexpr allocator_traits::pointer operator -> () const
		{
			return resource_ == nullptr ? throw basic_exception(L"tried to access a member of a null resource") : resource_;
		}

		[[nodiscard]] constexpr type_ &operator * () const
		{
			return resource_ == nullptr ? throw basic_exception(L"tried to dereference a null resource") : *resource_;
		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return resource_ != nullptr;
		}

		[[nodiscard]] constexpr allocator_type_ get_allocator() const noexcept
		{
			return allocator_;
		}

		[[nodiscard]] constexpr std::size_t use_count() const noexcept
		{
			return references_ == nullptr ? 0 : *references_;
		}

		[[nodiscard]] constexpr allocator_traits::pointer get() const noexcept
		{
			return resource_;
		}

		friend constexpr void swap(shared_memory_resource &left, shared_memory_resource &right) noexcept
		{
			using std::swap;

			if constexpr (std::is_same_v < allocator_traits::propagate_on_container_swap, std::true_type >) swap(left.allocator_, right.allocator_);

			swap(left.references_, right.references_);
			swap(left.resource_, right.resource_);
		}
	};

	template < typename type_, typename allocator_type_ >
	[[nodiscard]] constexpr bool operator == (const shared_memory_resource < type_, allocator_type_ > &left, const shared_memory_resource < type_, allocator_type_ > &right) noexcept
	{
		return left.get() == right.get();
	}
}

namespace std
{
	template < typename type_, typename allocator_type_ >
	struct hash < matteaz::shared_memory_resource < type_, allocator_type_ > >
	{
		[[nodiscard]] std::size_t operator () (const matteaz::shared_memory_resource < type_, allocator_type_ > &shared_memory_resource) const noexcept
		{
			return std::hash < decltype(shared_memory_resource.get()) > { } (shared_memory_resource.get());
		}
	};
}