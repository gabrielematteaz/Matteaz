#ifndef MATTEAZ_COMMANDLINE_H
#define MATTEAZ_COMMANDLINE_H

#include <expected>
#include <string>
#include <string_view>
#include <cstddef>
#include <iterator>
#include <optional>
#include <algorithm>

namespace Matteaz
{
	enum class NormalizationError;
	class CommandLineIterator;
	[[nodiscard]] constexpr std::expected < std::string, NormalizationError > Normalize(std::string_view string);

	enum class NormalizationError
	{
		UnmatchedSingleQuote,
		UnmatchedDoubleQuote,
	};

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
		std::string_view::size_type thisOffset = 0;
		std::string_view thisArgument;

	public:
		CommandLineIterator() = default;
		constexpr explicit CommandLineIterator(std::string_view commandLine) noexcept;
		constexpr CommandLineIterator operator ++ (int) noexcept;
		[[nodiscard]] constexpr pointer operator -> () const noexcept;
		constexpr CommandLineIterator &operator ++ () noexcept;
		// note: the returned argument is not normalized
		[[nodiscard]] constexpr reference operator * () const noexcept;
		[[nodiscard]] bool operator == (const CommandLineIterator &) const = default;
		// note: iteration termination is not an error
		[[nodiscard]] constexpr std::optional < NormalizationError > TryIncrement() noexcept;
		[[nodiscard]] constexpr CommandLineIterator begin() const noexcept;
		[[nodiscard]] constexpr CommandLineIterator end() const noexcept;
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
		return &thisArgument;
	}

	constexpr CommandLineIterator &CommandLineIterator::operator ++ () noexcept
	{
		auto result = TryIncrement();

		if (result.has_value())
			*this = end();

		return *this;
	}

	constexpr CommandLineIterator::reference CommandLineIterator::operator * () const noexcept
	{
		return thisArgument;
	}

	constexpr std::optional < NormalizationError > CommandLineIterator::TryIncrement() noexcept
	{
		auto isDelimiter = [] (char character) constexpr noexcept
		{
			return character == ' ' ||
				character == '\t' || character == '\n' || character == '\r' || character == '\v' || character == '\f';
		};

		auto first = std::find_if_not(thisCommandLine.begin() + thisOffset, thisCommandLine.end(), isDelimiter);

		if (first == thisCommandLine.end())
		{
			*this = end();
			return std::nullopt;
		}

		auto current = first;
		bool skip = false;
		bool withinSingleQuotes = false;
		bool withinDoubleQuotes = false;

		for (; current != thisCommandLine.end(); ++current)
		{
			if (skip)
			{
				skip = false;
				continue;
			}

			if (*current == '\\' && withinSingleQuotes == false)
				skip = true;
			else if (*current == '\'' && withinDoubleQuotes == false)
				withinSingleQuotes = !withinSingleQuotes;
			else if (*current == '\"' && withinSingleQuotes == false)
				withinDoubleQuotes = !withinDoubleQuotes;
			else if (isDelimiter(*current) && withinSingleQuotes == false && withinDoubleQuotes == false)
				break;
		}

		if (withinSingleQuotes)
			return NormalizationError::UnmatchedSingleQuote;

		if (withinDoubleQuotes)
			return NormalizationError::UnmatchedDoubleQuote;

		thisOffset = current - thisCommandLine.begin();
		thisArgument = thisCommandLine.substr(first - thisCommandLine.begin(), current - first);

		return std::nullopt;
	}

	constexpr CommandLineIterator CommandLineIterator::begin() const noexcept
	{
		return *this;
	}

	constexpr CommandLineIterator CommandLineIterator::end() const noexcept
	{
		return { };
	}

	constexpr std::expected < std::string, NormalizationError > Normalize(std::string_view string)
	{
		std::string normalizedString;

		normalizedString.reserve(string.length());

		auto first = string.begin();
		auto current = first;
		bool skip = false;
		bool withinSingleQuotes = false;
		bool withinDoubleQuotes = false;
		bool update = false;

		for (; current != string.end(); ++current)
		{
			if (skip)
			{
				skip = false;
				continue;
			}

			if (*current == '\\' && withinSingleQuotes == false)
			{
				skip = true;
				update = true;
			}
			else if (*current == '\'' && withinDoubleQuotes == false)
			{
				withinSingleQuotes = !withinSingleQuotes;
				update = true;
			}
			else if (*current == '\"' && withinSingleQuotes == false)
			{
				withinDoubleQuotes = !withinDoubleQuotes;
				update = true;
			}

			if (update)
			{
				normalizedString.append(first, current);
				first = current + 1;
				update = false;
			}
		}

		if (withinSingleQuotes)
			return std::unexpected(NormalizationError::UnmatchedSingleQuote);
		
		if (withinDoubleQuotes)
			return std::unexpected(NormalizationError::UnmatchedDoubleQuote);

		normalizedString.append(first, current);

		return std::move(normalizedString);
	}
}

#endif