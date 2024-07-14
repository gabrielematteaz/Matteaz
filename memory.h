#pragma once

#include "exception.h"

namespace matteaz
{
	struct allocation_error : public exception
	{
		HANDLE heap_;
		SIZE_T bytes_;

		constexpr explicit allocation_error(HANDLE heap, SIZE_T bytes) noexcept :
			exception(L"matteaz::allocation_error"),
			heap_(heap),
			bytes_(bytes)
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

		[[nodiscard]] type_ *allocate(std::size_t count) const
		{
			auto bytes = count * sizeof(type_);
			auto pointer = static_cast < type_* > (HeapAlloc(heap_, 0, bytes));

			return pointer == nullptr ? throw allocation_error(heap_, bytes) : pointer;
		}

		void deallocate(type_ *pointer, std::size_t) const noexcept
		{
			HeapFree(heap_, 0, pointer);
		}
	};

	template < typename type_, typename other_type_ >
	[[nodiscard]] constexpr bool operator == (const allocator < type_ > &left, const allocator < other_type_ > &right) noexcept
	{
		return left.heap() == right.heap();
	}

	template < typename type_, template < typename type_ > typename allocator_type_ = allocator >
	class shared_memory_resource
	{
		using type_allocator_traits = std::allocator_traits < allocator_type_ < type_ > >;
		using size_allocator_traits = std::allocator_traits < allocator_type_ < std::size_t > >;

		allocator_type_ < type_ > allocator_;
		size_allocator_traits::pointer references_;
		type_allocator_traits::pointer pointer_;

	public:
		template < typename... parameters_type_ >
		constexpr explicit shared_memory_resource(const allocator_type_ < type_ > &allocator = allocator_type_ < type_ > (), parameters_type_&&... parameters) :
			allocator_(allocator), references_(nullptr), pointer_(nullptr)
		{
			allocator_type_ < std::size_t > size_allocator(allocator);

			try {
				references_ = size_allocator_traits::allocate(size_allocator, 1);
				pointer_ = type_allocator_traits::allocate(allocator_, 1);
				new(pointer_) type_(std::forward < parameters_type_ > (parameters)...);
				*references_ = 1;
			} catch (...) {
				size_allocator_traits::deallocate(size_allocator, references_, 1);
				type_allocator_traits::deallocate(allocator_, pointer_, 1);
				throw;
			}
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
				allocator_type_ < std::size_t > size_allocator(allocator_);

				type_allocator_traits::deallocate(allocator_, pointer_, 1);
				size_allocator_traits::deallocate(size_allocator, references_, 1);
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

		[[nodiscard]] constexpr type_allocator_traits::pointer operator -> () const
		{
			return pointer_ == nullptr ? throw exception(L"tried to access a member of a null shared memory resource") : pointer_;
		}

		[[nodiscard]] constexpr type_ &operator * () const
		{
			return pointer_ == nullptr ? throw exception(L"tried to dereference a null shared memory resource") : *pointer_;
		}

		[[nodiscard]] constexpr allocator_type_ < type_ > allocator() const noexcept
		{
			return allocator_;
		}

		[[nodiscard]] constexpr std::size_t references() const noexcept
		{
			return references_ == nullptr ? 0 : *references_;
		}

		[[nodiscard]] constexpr type_allocator_traits::pointer get() const noexcept
		{
			return pointer_;
		}

		friend constexpr void swap(shared_memory_resource &left, shared_memory_resource &right) noexcept
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

	template < typename type_, template < typename type_ > typename allocator_type_ >
	[[nodiscard]] constexpr bool operator == (const shared_memory_resource < type_, allocator_type_ > &left, const shared_memory_resource < type_, allocator_type_ > &right) noexcept
	{
		return left.get() == right.get();
	}
}

namespace std
{
	template < typename type_, template < typename type_ > typename allocator_type_ >
	struct hash < matteaz::shared_memory_resource < type_, allocator_type_ > >
	{
		[[nodiscard]] std::size_t operator () (const matteaz::shared_memory_resource < type_, allocator_type_ > &shared_memory_resource) const noexcept
		{
			return std::hash < type_* > {} (shared_memory_resource.get());
		}
	};
}