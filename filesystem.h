#pragma once

#include < Windows.h >
#include < PathCch.h >

#include "memory.h"

#pragma comment(lib, "Pathcch.lib")

namespace matteaz
{
	class directory_iterator
	{
		struct state
		{
			HANDLE find_file_;
			WIN32_FIND_DATAW find_data_;
		};

		shared_memory_resource < state > state_;

	public:
		struct sentinel { };

		using value_type = WIN32_FIND_DATAW;
		using difference_type = std::ptrdiff_t;
		using pointer = WIN32_FIND_DATAW*;
		using reference = WIN32_FIND_DATAW&;
		using iterator_category = std::input_iterator_tag;

		explicit directory_iterator(const wchar_t *path, const allocator < state > &allocator);
		directory_iterator(const directory_iterator&) = default;
		directory_iterator(directory_iterator&&) = default;
		directory_iterator &operator = (const directory_iterator&) = default;
		directory_iterator &operator = (directory_iterator&&) = default;
		directory_iterator &operator ++ () noexcept;
		[[nodiscard]] directory_iterator begin() const noexcept;

		constexpr ~directory_iterator()
		{
			if (state_.use_count() == 1) FindClose(state_.get()->find_file_);
		}

		[[deprecated("Unachievable in this implementation")]]
		constexpr void operator ++ (int) const noexcept
		{

		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return state_.get()->find_file_ != INVALID_HANDLE_VALUE;
		}

		[[nodiscard]] constexpr WIN32_FIND_DATAW *operator -> () const noexcept
		{
			return &state_.get()->find_data_;
		}

		[[nodiscard]] constexpr WIN32_FIND_DATAW &operator * () const noexcept
		{
			return state_.get()->find_data_;
		}

		[[nodiscard]] constexpr bool operator == (const sentinel&) const noexcept
		{
			return state_.get()->find_file_ == INVALID_HANDLE_VALUE;
		}

		template < typename type_ >
		[[nodiscard]] constexpr allocator < type_ > get_allocator() const noexcept
		{
			return state_.get_allocator();
		}

		[[nodiscard]] constexpr std::size_t use_count() const noexcept
		{
			return state_.use_count();
		}

		[[nodiscard]] constexpr sentinel end() const noexcept
		{
			return { };
		}

		friend constexpr void swap(directory_iterator &left, directory_iterator &right) noexcept
		{
			swap(left.state_, right.state_);
		}
	};
}