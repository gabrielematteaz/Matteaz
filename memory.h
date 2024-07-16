#pragma once

#include "exception.h"

namespace matteaz
{
	struct allocation_error : public exception
	{
		HANDLE heap_;
		SIZE_T bytes_;

		constexpr explicit allocation_error(HANDLE heap, SIZE_T bytes) noexcept :
			exception(L"matteaz::allocation_error"), heap_(heap), bytes_(bytes)
		{

		}
	};

	template < typename type_ >
	class allocator
	{
		HANDLE heap_;

	public:
		using value_type = type_;
		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap = std::true_type;

		constexpr allocator(HANDLE heap = GetProcessHeap()) :
			heap_(heap == NULL ? throw exception(L"passed a null heap handle") : heap)
		{

		}

		template < typename other_type_ >
		constexpr allocator(const allocator < other_type_ > &other) noexcept :
			heap_(other.heap())
		{

		}

		[[nodiscard]] constexpr HANDLE heap() const noexcept
		{
			return heap_;
		}

		[[nodiscard]] value_type *allocate(std::size_t count) const
		{
			auto bytes = count * sizeof(value_type);
			auto pointer = static_cast < value_type* > (HeapAlloc(heap_, 0, bytes));

			return pointer == nullptr ? throw allocation_error(heap_, bytes) : pointer;
		}

		void deallocate(value_type *pointer, std::size_t) const noexcept
		{
			HeapFree(heap_, 0, pointer);
		}
	};

	template < typename type_, typename other_type_ >
	[[nodiscard]] constexpr bool operator == (const allocator < type_ > &left, const allocator < other_type_ > &right) noexcept
	{
		return left.heap() == right.heap();
	}

	template < typename allocator_type_ >
	requires std::is_nothrow_destructible_v < typename allocator_type_::value_type >
	class shared_memory_resource
	{
		using allocator_traits = std::allocator_traits < allocator_type_ >;
		using size_allocator_type = allocator_traits::template rebind_alloc < std::size_t >;
		using size_allocator_traits = std::allocator_traits < size_allocator_type >;

		allocator_type_ allocator_;
		size_allocator_traits::pointer references_;
		allocator_traits::pointer pointer_;

	public:
		using element_type = allocator_type_::value_type;
		using allocator_type = allocator_type_;

		template < typename... parameters_type_ >
		requires std::is_constructible_v < element_type, parameters_type_... >
		explicit shared_memory_resource(const allocator_type &allocator = allocator_type(), parameters_type_&&... parameters) :
			allocator_(allocator), references_(nullptr), pointer_(nullptr)
		{
			size_allocator_type size_allocator(allocator);
			typename size_allocator_traits::pointer references = nullptr;
			typename allocator_traits::pointer pointer = nullptr;

			try {
				references = size_allocator_traits::allocate(size_allocator, 1);
				*references = 1;
				pointer = allocator_traits::allocate(allocator_, 1);
				new(pointer) element_type(std::forward < parameters_type_ > (parameters)...);
			} catch (...) {
				size_allocator_traits::deallocate(size_allocator, references, 1);
				allocator_traits::deallocate(allocator_, pointer, 1);
				throw;
			}

			references_ = references;
			pointer_ = pointer;
		}

		constexpr shared_memory_resource(const shared_memory_resource &other) noexcept :
			allocator_(other.allocator_), references_(other.references_), pointer_(other.pointer_)
		{
			if (references_ == nullptr) return;

			++*references_;
		}

		constexpr shared_memory_resource(shared_memory_resource &&other) noexcept :
			allocator_(std::move(other.allocator_)), references_(other.references_), pointer_(other.pointer_)
		{
			other.references_ = nullptr;
			other.pointer_ = nullptr;
		}

		constexpr ~shared_memory_resource()
		{
			if (references_ == nullptr) return;

			--*references_;

			if (*references_ == 0) {
				size_allocator_type allocator(allocator_);

				if constexpr (!std::is_trivially_destructible_v < element_type >) pointer_->~element_type();

				size_allocator_traits::deallocate(allocator, references_, 1);
				allocator_traits::deallocate(allocator_, pointer_, 1);
			}
		}

		constexpr shared_memory_resource &operator = (const shared_memory_resource &other) noexcept
		{
			if (this == &other) return *this;

			this->~shared_memory_resource();
			allocator_ = other.allocator_;
			references_ = other.references_;
			pointer_ = other.pointer_;

			if (references_ == nullptr) return *this;

			++*references_;

			return *this;
		}

		constexpr shared_memory_resource &operator = (shared_memory_resource &&other) noexcept
		{
			if (this == &other) return *this;

			this->~shared_memory_resource();
			allocator_ = std::move(other.allocator_);
			references_ = other.references_;
			pointer_ = other.pointer_;
			other.references_ = nullptr;
			other.pointer_ = nullptr;

			return *this;
		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return pointer_ != nullptr;
		}

		[[nodiscard]] constexpr allocator_traits::pointer operator -> () const
		{
			return pointer_ == nullptr ? throw exception(L"tried to access a member of a null shared memory resource") : pointer_;
		}

		[[nodiscard]] constexpr decltype(*pointer_) operator * () const
		{
			return pointer_ == nullptr ? throw exception(L"tried to dereference a null shared memory resource") : *pointer_;
		}

		[[nodiscard]] constexpr allocator_type allocator() const noexcept
		{
			return allocator_;
		}

		[[nodiscard]] constexpr std::size_t references() const noexcept
		{
			return references_ == nullptr ? 0 : *references_;
		}

		[[nodiscard]] constexpr allocator_traits::pointer get() const noexcept
		{
			return pointer_;
		}

		friend constexpr void swap(shared_memory_resource < allocator_type_ > &left, shared_memory_resource < allocator_type_ > &right) noexcept
		{
			using std::swap;

			auto references = left.references_;
			auto pointer = left.pointer_;

			swap(left.allocator_, right.allocator_);
			left.references_ = right.references_;
			left.pointer_ = right.pointer_;
			right.references_ = references;
			right.pointer_ = pointer;
		}
	};

	template < typename allocator_type_ >
	[[nodiscard]] constexpr bool operator == (const shared_memory_resource < allocator_type_ > &left, const shared_memory_resource < allocator_type_ > &right) noexcept
	{
		return left.get() == right.get();
	}
}

namespace std
{
	template < typename allocator_type_ >
	struct hash < matteaz::shared_memory_resource < allocator_type_ > >
	{
		[[nodiscard]] std::size_t operator () (const matteaz::shared_memory_resource < allocator_type_ > &shared_memory_resource) const noexcept
		{
			return std::hash < decltype(shared_memory_resource.get()) > { } (shared_memory_resource.get());
		}
	};
}