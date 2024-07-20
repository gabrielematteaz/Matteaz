#pragma once

#include "error.h"

namespace matteaz
{
	struct allocation_error : public exception
	{
		HANDLE heap_;
		SIZE_T bytes_;

		constexpr allocation_error(HANDLE heap, SIZE_T bytes) noexcept :
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
		using size_type = SIZE_T;
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

		[[nodiscard]] type_ *allocate(SIZE_T count) const
		{
			auto bytes = count * sizeof(type_);
			auto pointer = static_cast < type_* > (HeapAlloc(heap_, 0, bytes));

			return pointer == nullptr ? throw allocation_error(heap_, bytes) : pointer;
		}

		void deallocate(type_ *pointer, SIZE_T) const noexcept
		{
			HeapFree(heap_, 0, pointer);
		}
	};

	template < typename type_, typename other_type_ >
	[[nodiscard]] constexpr bool operator == (const allocator < type_ > &left, const allocator < other_type_ > &right) noexcept
	{
		return left.heap() == right.heap();
	}

	template < typename type_ >
		requires std::is_nothrow_destructible_v < type_ >
	class shared_memory_resource
	{
		allocator < type_ > allocator_;
		std::size_t *references_;
		type_ *pointer_;

	public:
		using element_type = type_;
		using allocator_type = allocator < type_ >;

		template < typename... parameters_type_ >
			requires std::is_constructible_v < type_, parameters_type_... >
		explicit shared_memory_resource(const allocator < type_ > &allocator, parameters_type_&&... parameters) :
			allocator_(allocator), references_(nullptr), pointer_(nullptr)
		{
			matteaz::allocator < std::size_t > allocator_(allocator);
			std::size_t *references = nullptr;
			type_ *pointer = nullptr;

			try {
				references = allocator_.allocate(1);
				pointer = allocator.allocate(1);
				new(pointer) type_(std::forward < parameters_type_ > (parameters)...);
			} catch (...) {
				allocator_.deallocate(references, 1);
				allocator.deallocate(pointer, 1);
				throw;
			}

			*references = 1;
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
			allocator_(other.allocator_), references_(other.references_), pointer_(other.pointer_)
		{
			other.references_ = nullptr;
			other.pointer_ = nullptr;
		}

		constexpr ~shared_memory_resource()
		{
			if (references_ == nullptr) return;

			--*references_;

			if (*references_ == 0) {
				if constexpr (!std::is_trivially_destructible_v < type_ >) pointer_->~type_();

				allocator < std::size_t > (allocator_).deallocate(references_, 1);
				allocator_.deallocate(pointer_, 1);
			}
		}

		constexpr shared_memory_resource &operator = (shared_memory_resource other) noexcept
		{
			swap(*this, other);

			return *this;
		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return pointer_ != nullptr;
		}

		[[nodiscard]] constexpr type_ *operator -> () const
		{
			return pointer_ == nullptr ? throw exception(L"tried to access a member of a null resource") : pointer_;
		}

		[[nodiscard]] constexpr type_ &operator * () const
		{
			return pointer_ == nullptr ? throw exception(L"tried to dereference a null resource") : *pointer_;
		}

		[[nodiscard]] constexpr allocator < type_ > get_allocator() const noexcept
		{
			return allocator_;
		}

		[[nodiscard]] constexpr std::size_t use_count() const noexcept
		{
			return references_ == nullptr ? 0 : *references_;
		}

		[[nodiscard]] constexpr type_ *get() const noexcept
		{
			return pointer_;
		}

		friend constexpr void swap(shared_memory_resource &left, shared_memory_resource &right) noexcept
		{
			auto allocator(left.allocator_);
			auto references = left.references_;
			auto pointer = left.pointer_;

			left.allocator_ = right.allocator_;
			left.references_ = right.references_;
			left.pointer_ = right.pointer_;
			right.allocator_ = allocator;
			right.references_ = references;
			right.pointer_ = pointer;
		}
	};

	template < typename type_ >
	[[nodiscard]] constexpr bool operator == (const shared_memory_resource < type_ > &left, const shared_memory_resource < type_ > &right) noexcept
	{
		return left.get() == right.get();
	}
}