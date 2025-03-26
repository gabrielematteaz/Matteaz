#ifndef MATTEAZ_COMMANDLINE_H
#define MATTEAZ_COMMANDLINE_H

#include <optional>
#include <string>
#include <string_view>
#include <cstddef>
#include <iterator>
#include <algorithm>
#include <utility>

namespace Matteaz
{
	class ArgumentsIterator;
	[[nodiscard]] constexpr std::optional < std::string > Normalize(std::string_view string);

	class ArgumentsIterator
	{
	public:
		using value_type = std::string_view;
		using difference_type = std::ptrdiff_t;
		using reference = const value_type &;
		using pointer = const value_type *;
		using iterator_category = std::forward_iterator_tag;

	private:
		std::string_view thisCommandLine;
		std::string_view::size_type thisCurrentOffset = 0;
		value_type thisCurrentArgument; // guaranteed to be normalizable

	public:
		ArgumentsIterator() = default;
		constexpr ArgumentsIterator(std::string_view commandLine) noexcept;
		constexpr ArgumentsIterator operator ++ (int) noexcept;
		[[nodiscard]] constexpr pointer operator -> () const noexcept;
		constexpr ArgumentsIterator &operator ++ () noexcept;
		[[nodiscard]] constexpr reference operator * () const noexcept;
		[[nodiscard]] bool operator == (const ArgumentsIterator &) const = default;
		[[nodiscard]] constexpr ArgumentsIterator begin() const noexcept;
		[[nodiscard]] constexpr ArgumentsIterator end() const noexcept;
		constexpr bool TryIncrement() noexcept;
	};

	constexpr ArgumentsIterator::ArgumentsIterator(std::string_view commandLine) noexcept :
		thisCommandLine(commandLine)
	{
		operator ++ ();
	}

	constexpr ArgumentsIterator ArgumentsIterator::operator ++ (int) noexcept
	{
		auto previousThis = *this;

		operator ++ ();

		return previousThis;
	}

	constexpr ArgumentsIterator::pointer ArgumentsIterator::operator -> () const noexcept
	{
		return &thisCurrentArgument;
	}

	constexpr ArgumentsIterator &ArgumentsIterator::operator ++ () noexcept
	{
		if (TryIncrement() == false)
			*this = end();

		return *this;
	}

	constexpr ArgumentsIterator::reference ArgumentsIterator::operator * () const noexcept
	{
		return thisCurrentArgument;
	}

	constexpr ArgumentsIterator ArgumentsIterator::begin() const noexcept
	{
		return *this;
	}

	constexpr ArgumentsIterator ArgumentsIterator::end() const noexcept
	{
		return { };
	}

	constexpr bool ArgumentsIterator::TryIncrement() noexcept
	{
		auto isDelimiter = [] (char character) constexpr noexcept
		{
			return character == ' ' || character == '\t' || character == '\n' || character == '\r' || character == '\v' || character == '\f';
		};

		auto first = std::find_if_not(thisCommandLine.begin() + thisCurrentOffset, thisCommandLine.end(), isDelimiter);

		if (first == thisCommandLine.end()) // end of iteration (not an error)
		{
			*this = end();
			return true;
		}

		auto current = first;
		bool skip = false;
		bool withinSingleQuotes = false;
		bool withinDoubleQuotes = false;

		for (; current != thisCommandLine.end(); ++current)
		{
			if (skip)
				skip = false;
			else if (*current == '\\' && withinSingleQuotes == false)
				skip = true;
			else if (*current == '\'' && withinDoubleQuotes == false)
				withinSingleQuotes = !withinSingleQuotes;
			else if (*current == '\"' && withinSingleQuotes == false)
				withinDoubleQuotes = !withinDoubleQuotes;
			else if (isDelimiter(*current) && withinSingleQuotes == false && withinDoubleQuotes == false)
				break;
		}

		if (withinSingleQuotes || withinDoubleQuotes)
			return false;

		thisCurrentOffset = current - thisCommandLine.begin();
		thisCurrentArgument = thisCommandLine.substr(first - thisCommandLine.begin(), current - first);

		return true;
	}

	constexpr std::optional < std::string > Normalize(std::string_view string)
	{
		std::string normalizedString;

		normalizedString.reserve(string.length());

		auto first = string.begin();
		auto current = first;
		bool skip = false;
		bool update = false;
		bool withinSingleQuotes = false;
		bool withinDoubleQuotes = false;

		for (; current != string.end(); ++current)
		{
			if (skip)
			{
				skip = false;

				if (withinDoubleQuotes == false || *current == '\\' || *current == '\"') // 1st edge case
				{
					normalizedString.append(first, current - 1);
					first = current;
				}

				continue;
			}

			if (*current == '\\' && withinSingleQuotes == false)
				skip = true;
			else if (*current == '\'' && withinDoubleQuotes == false)
			{
				update = true;
				withinSingleQuotes = !withinSingleQuotes;
			}
			else if (*current == '\"' && withinSingleQuotes == false)
			{
				update = true;
				withinDoubleQuotes = !withinDoubleQuotes;
			}

			if (update)
			{
				update = false;
				normalizedString.append(first, current);
				first = current + 1;
			}
		}

		if (withinSingleQuotes || withinDoubleQuotes)
			return std::nullopt;

		normalizedString.append(first, current - skip); // 2nd edge case

		return std::move(normalizedString);
	}
}

#endif
