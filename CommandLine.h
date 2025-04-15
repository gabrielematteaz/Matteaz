#ifndef MATTEAZ_COMMANDLINE_H
#define MATTEAZ_COMMANDLINE_H

#include <optional>
#include <string>
#include <string_view>
#include <cstddef>
#include <iterator>
#include <algorithm>

namespace Matteaz
{
	class ArgumentsIterator;
	[[nodiscard]] constexpr std::optional < std::string > NormalizeArgument(std::string_view argument);
	
	class ArgumentsIterator
	{
	public:
		using value_type = std::string_view;
		using difference_type = std::ptrdiff_t;
		using reference = const value_type &;
		using pointer = const value_type *;
		using iterator_category = std::forward_iterator_tag;
		
	private:
		std::string_view _CommandLine;
		std::string_view::size_type _Offset = 0;
		value_type _Argument;
		
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
	};
	
	constexpr ArgumentsIterator::ArgumentsIterator(std::string_view commandLine) noexcept :
		_CommandLine(commandLine)
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
		return &_Argument;
	}
	
	constexpr ArgumentsIterator &ArgumentsIterator::operator ++ () noexcept
	{
		auto isDelimiter = [] (char character) constexpr noexcept
		{
			return character == ' ' || character == '\t' || character == '\n' || character == '\r' || character == '\v' || character == '\f';
		};
		
		auto first = std::find_if_not(_CommandLine.begin() + _Offset, _CommandLine.end(), isDelimiter);
		
		if (first == _CommandLine.end())
		{
			*this = end();
			return *this;
		}
		
		auto current = first;
		bool skip = false;
		bool withinSingleQuotes = false;
		bool withinDoubleQuotes = false;
		
		for (; current != _CommandLine.end(); ++current)
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
		{
			*this = end();
			return *this;
		}
		
		_Offset = current - _CommandLine.begin();
		_Argument = _CommandLine.substr(first - _CommandLine.begin(), current - first);
		
		return *this;
	}
	
	constexpr ArgumentsIterator::reference ArgumentsIterator::operator * () const noexcept
	{
		return _Argument;
	}
	
	constexpr ArgumentsIterator ArgumentsIterator::begin() const noexcept
	{
		return *this;
	}
	
	constexpr ArgumentsIterator ArgumentsIterator::end() const noexcept
	{
		return { };
	}
	
	constexpr std::optional < std::string > NormalizeArgument(std::string_view argument)
	{
		std::string normalizedArgument;
		
		normalizedArgument.reserve(argument.length());
		
		auto first = argument.begin();
		auto current = first;
		bool skip = false;
		bool update = false;
		bool withinSingleQuotes = false;
		bool withinDoubleQuotes = false;
		
		for (; current != argument.end(); ++current)
		{
			if (skip)
			{
				skip = false;
				
				if (withinDoubleQuotes == false || *current == '\"' || *current == '\\') // first edge case
				{
					normalizedArgument.append(first, current - 1);
					first = current;
				}
			}
			else if (*current == '\\' && withinSingleQuotes == false)
				skip = true;
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
				update = false;
				normalizedArgument.append(first, current);
				first = current + 1;
			}
		}
		
		if (withinSingleQuotes || withinDoubleQuotes)
			return std::nullopt;
		
		normalizedArgument.append(first, current - skip); // second edge case
		
		return normalizedArgument;
	}
}

#endif