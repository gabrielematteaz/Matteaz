#pragma once

#include < Windows.h >
#include < PathCch.h >

#include "memory.h"

#pragma comment(lib, "Pathcch.lib")

namespace matteaz
{
	template < typename allocator_type_ >
	class directory_iterator
	{
		struct _state
		{
			HANDLE find_file_;
			WIN32_FIND_DATAW find_data_;
		};

		shared_memory_resource < typename std::allocator_traits < allocator_type_ >::template rebind_alloc < _state > > state_;

	public:
		struct sentinel { };

		using value_type = WIN32_FIND_DATAW;
		using difference_type = std::ptrdiff_t;
		using reference = WIN32_FIND_DATAW&;
		using pointer = WIN32_FIND_DATA*;
		using iterator_category = std::input_iterator_tag;

		directory_iterator(const directory_iterator&) = default;
		directory_iterator(directory_iterator&&) = default;
		directory_iterator &operator = (const directory_iterator&) = default;
		directory_iterator &operator = (directory_iterator&&) = default;

		explicit directory_iterator(const wchar_t *path, const allocator_type_ &allocator = allocator_type_()) :
			state_(allocator, INVALID_HANDLE_VALUE)
		{
			wchar_t *file_name;
			auto result = PathAllocCombine(path, L"*", PATHCCH_ALLOW_LONG_PATHS, &file_name);

			if (result != S_OK) throw HRESULT_error(result, L"PathAllocCombine failed");

			auto state = state_.get();

			state->find_file_ = FindFirstFileW(file_name, &state->find_data_);
			LocalFree(file_name);
		}

		constexpr ~directory_iterator()
		{
			if (state_.references() == 1) FindClose(state_.get()->find_file_);
		}

		[[deprecated("Unachievable in this implementation")]]
		constexpr void operator ++ (int) const noexcept
		{

		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return state_.get()->find_file_ != INVALID_HANDLE_VALUE;
		}

		[[nodiscard]] constexpr pointer operator -> () const noexcept
		{
			return &state_.get()->find_data_;
		}

		directory_iterator &operator ++ () noexcept
		{
			auto state = state_.get();

			if (FindNextFileW(state->find_file_, &state->find_data_) != 0) return *this;

			FindClose(state->find_file_);
			state->find_file_ = INVALID_HANDLE_VALUE;

			return *this;
		}

		[[nodiscard]] constexpr reference operator * () const noexcept
		{
			return state_.get()->find_data_;
		}

		[[nodiscard]] constexpr bool operator == (const sentinel&) const noexcept
		{
			return state_.get()->find_file_ == INVALID_HANDLE_VALUE;
		}

		[[nodiscard]] constexpr directory_iterator begin() const noexcept
		{
			return *this;
		}

		[[nodiscard]] constexpr sentinel end() const noexcept
		{
			return { };
		}

		friend constexpr void swap(directory_iterator < allocator_type_ > &left, directory_iterator < allocator_type_ > &right) noexcept
		{
			swap(left.state_, right.state_);
		}
	};
}