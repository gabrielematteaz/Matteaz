#pragma once

#include < string >
#include "Memory.h"

namespace Matteaz
{
	struct DirectoryIteratorSentinel
	{
		explicit DirectoryIteratorSentinel() = default;
	};

	template < typename AllocatorType = Allocator < void > >
	class DirectoryIterator
	{
		struct State
		{
			HANDLE findFile;
			WIN32_FIND_DATAW findData;
		};

		SharedMemoryResource < State, typename std::allocator_traits < AllocatorType >::template rebind_alloc < State > > state;

	public:
		using value_type = WIN32_FIND_DATAW;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type *;
		using reference = const value_type &;
		using iterator_category = std::true_type;

		explicit DirectoryIterator(const wchar_t *path, const AllocatorType &allocator = AllocatorType()) :
			state(allocator, INVALID_HANDLE_VALUE)
		{
			std::basic_string < wchar_t, std::char_traits < wchar_t >, typename std::allocator_traits < AllocatorType >::template rebind_alloc < wchar_t > > fileName(path);

			fileName.append(L"\\*");

			if (fileName.length() > MAX_PATH && !fileName.starts_with(L"\\\\?\\")) fileName.insert(0, L"\\\\?\\");

			auto state = this->state.get();

			state->findFile = FindFirstFileW(fileName.c_str(), &state->findData);
		}

		constexpr ~DirectoryIterator()
		{
			if (state.use_count() == 1) FindClose(state.get()->findFile);
		}

		[[deprecated("Unachievable in this implementation")]]
		constexpr void operator ++ (int) const noexcept
		{

		}

		[[nodiscard]] constexpr explicit operator bool() const noexcept
		{
			return state.get()->findFile != INVALID_HANDLE_VALUE;
		}

		[[nodiscard]] constexpr pointer operator -> () const noexcept
		{
			return &state.get()->findData;
		}

		DirectoryIterator &operator ++ () noexcept
		{
			auto state = this->state.get();

			if (FindNextFileW(state->findFile, &state->findData)) return *this;

			FindClose(state->findFile);
			state->findFile = INVALID_HANDLE_VALUE;

			return *this;
		}

		[[nodiscard]] constexpr reference operator * () const noexcept
		{
			return state.get()->findData;
		}

		[[nodiscard]] constexpr bool operator == (const DirectoryIteratorSentinel &) const noexcept
		{
			return state.get()->findFile == INVALID_HANDLE_VALUE;
		}

		[[nodiscard]] constexpr DirectoryIterator begin() const noexcept
		{
			return *this;
		}

		[[nodiscard]] constexpr DirectoryIteratorSentinel end() const noexcept
		{
			return DirectoryIteratorSentinel();
		}

		friend constexpr void swap(DirectoryIterator &left, DirectoryIterator &right) noexcept
		{
			swap(left.state, right.state);
		}
	};
}
