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
	class CommandLineIterator;
	[[nodiscard]] constexpr std::optional < std::string > Normalize(std::string_view string);

	class CommandLineIterator
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
		value_type thisCurrentArgument;

	public:
		CommandLineIterator() = default;
		constexpr CommandLineIterator(std::string_view commandLine) noexcept;
		constexpr CommandLineIterator operator ++ (int) noexcept;
		[[nodiscard]] constexpr pointer operator -> () const noexcept;
		constexpr CommandLineIterator &operator ++ () noexcept;
		[[nodiscard]] constexpr reference operator * () const noexcept;
		[[nodiscard]] bool operator == (const CommandLineIterator &) const = default;
		[[nodiscard]] constexpr CommandLineIterator begin() const noexcept;
		[[nodiscard]] constexpr CommandLineIterator end() const noexcept;
		constexpr bool TryIncrement() noexcept;
	};

	constexpr CommandLineIterator::CommandLineIterator(std::string_view commandLine) noexcept :
		thisCommandLine(commandLine)
	{
		operator ++ ();
	}

	constexpr CommandLineIterator CommandLineIterator::operator ++ (int) noexcept
	{
		auto previousThis = *this;

		operator ++ ();

		return previousThis;
	}

	constexpr CommandLineIterator::pointer CommandLineIterator::operator -> () const noexcept
	{
		return &thisCurrentArgument;
	}

	constexpr CommandLineIterator &CommandLineIterator::operator ++ () noexcept
	{
		if (TryIncrement() == false)
			*this = end();

		return *this;
	}

	constexpr CommandLineIterator::reference CommandLineIterator::operator * () const noexcept
	{
		return thisCurrentArgument;
	}

	constexpr CommandLineIterator CommandLineIterator::begin() const noexcept
	{
		return *this;
	}

	constexpr CommandLineIterator CommandLineIterator::end() const noexcept
	{
		return { };
	}

	constexpr bool CommandLineIterator::TryIncrement() noexcept
	{
		auto isDelimiter = [] (char character) constexpr noexcept
		{
			return character == ' ' || character == '\t' || character == '\n' || character == '\r' || character == '\v' || character == '\f';
		};

		auto first = std::find_if_not(thisCommandLine.begin() + thisCurrentOffset, thisCommandLine.end(), isDelimiter);

		if (first == thisCommandLine.end())
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

				if (withinDoubleQuotes == false || *current == '\\' || *current == '\"')
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
				normalizedString.append(first, current);
				first = current + 1;
				update = false;
			}
		}

		if (withinSingleQuotes || withinDoubleQuotes)
			return std::nullopt;

		normalizedString.append(first, current);

		return std::move(normalizedString);
	}
}

#endif