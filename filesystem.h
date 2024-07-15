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
		struct state
		{
			HANDLE find_file_;
			WIN32_FIND_DATAW find_data_;
		};

		struct sentinel
		{
			[[nodiscard]] constexpr bool operator == (const directory_iterator &directory_iterator) const noexcept
			{
				return INVALID_HANDLE_VALUE == directory_iterator.state_.get()->find_file_;
			}
		};

		using state_allocator_type = std::allocator_traits < allocator_type_ >::template rebind_alloc < state >;

		shared_memory_resource < state_allocator_type > state_;

	public:
		using value_type = WIN32_FIND_DATAW;
		using difference_type = std::ptrdiff_t;

		directory_iterator(const directory_iterator&) = default;
		directory_iterator(directory_iterator&&) = default;
		directory_iterator &operator = (const directory_iterator&) = default;
		directory_iterator &operator = (directory_iterator&&) = default;

		constexpr directory_iterator(const wchar_t *path = nullptr, const allocator_type_ &allocator = allocator_type_()) :
			state_(allocator, INVALID_HANDLE_VALUE)
		{
			if (path == nullptr) return;

			//std::basic_string < wchar_t, std::char_traits < wchar_t >, matteaz::allocator < wchar_t > > file_name(path, allocator);

			//file_name.append(L"\\*");

			//if (file_name.length() > MAX_PATH && !file_name.starts_with(L"\\\\?\\")) file_name.insert(0, L"\\\\?\\");

			//auto state = state_.get();

			//state->find_file_ = FindFirstFileW(file_name.c_str(), &state->find_data_);

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

		[[nodiscard]] constexpr WIN32_FIND_DATAW *operator -> () const noexcept
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

		[[nodiscard]] constexpr WIN32_FIND_DATAW &operator * () const noexcept
		{
			return state_.get()->find_data_;
		}

		[[nodiscard]] constexpr bool operator == (const sentinel&) const noexcept
		{
			return state_.get()->find_file_ == INVALID_HANDLE_VALUE;
		}

		constexpr directory_iterator begin() const noexcept
		{
			return *this;
		}

		constexpr sentinel end() const noexcept
		{
			return { };
		}

		friend constexpr void swap(directory_iterator &left, directory_iterator &right) noexcept
		{
			swap(left.state_, right.state_);
		}
	};
}