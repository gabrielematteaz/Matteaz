#pragma once

#include < string >
#include "memory.h"

namespace matteaz
{
	struct sentinel
	{
		explicit sentinel() = default;
	};

	template < typename allocator_type_ = allocator < void > >
	class directory_iterator
	{
		struct state
		{
			HANDLE find_file;
			WIN32_FIND_DATAW find_data;
		};

		shared_memory_resource < state, typename std::allocator_traits < allocator_type_ >::template rebind_alloc < state > > state_;

	public:
		using value_type = WIN32_FIND_DATAW;
		using difference_type = std::ptrdiff_t;
		using pointer = const WIN32_FIND_DATAW *;
		using reference = const WIN32_FIND_DATAW &;
		using iterator_category = std::input_iterator_tag;

		explicit directory_iterator(const wchar_t *path, const allocator_type_ &allocator = allocator_type_()) :
			state_(allocator, INVALID_HANDLE_VALUE)
		{
			std::basic_string < wchar_t, std::char_traits < wchar_t >, typename std::allocator_traits < allocator_type_ >::template rebind_alloc < wchar_t > > file_name(path);

			file_name.append(L"\\*");

			if (file_name.length() > MAX_PATH && !file_name.starts_with(L"\\\\?\\")) file_name.insert(0, L"\\\\?\\");

			auto state = state_.get();

			state->find_file = FindFirstFileW(file_name.c_str(), &state->find_data);
		}

		constexpr ~directory_iterator()
		{
			if (state_.use_count() == 1) FindClose(state_.get()->find_file);
		}

		[[deprecated("Unachievable in this implementation")]]
		constexpr void operator ++ (int) const noexcept
		{

		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return state_.get()->find_file != INVALID_HANDLE_VALUE;
		}

		[[nodiscard]] constexpr pointer operator -> () const noexcept
		{
			return &state_.get()->find_data;
		}

		directory_iterator &operator ++ () noexcept
		{
			auto state = state_.get();

			if (FindNextFileW(state->find_file, &state->find_data)) return *this;

			FindClose(state->find_file);
			state->find_file = INVALID_HANDLE_VALUE;

			return *this;
		}

		[[nodiscard]] constexpr reference operator * () const noexcept
		{
			return state_.get()->find_data;
		}

		[[nodiscard]] constexpr bool operator == (const sentinel &) const noexcept
		{
			return state_.get()->find_file == INVALID_HANDLE_VALUE;
		}

		[[nodiscard]] constexpr directory_iterator begin() const noexcept
		{
			return *this;
		}

		[[nodiscard]] constexpr sentinel end() const noexcept
		{
			return sentinel();
		}

		friend constexpr void swap(directory_iterator &left, directory_iterator &right) noexcept
		{
			swap(left.state_, right.state_);
		}
	};
}