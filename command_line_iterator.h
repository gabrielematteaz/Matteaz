#ifndef MATTEAZ_COMMAND_LINE_ITERATOR_H
#define MATTEAZ_COMMAND_LINE_ITERATOR_H

#include <string_view>
#include <cstddef>
#include <iterator>
#include <algorithm>
#include <string>

namespace matteaz
{
	class command_line_iterator
	{
	public:
		using value_type = std::string_view;
		using difference_type = std::ptrdiff_t;
		using reference = const value_type &;
		using pointer = const value_type *;
		using iterator_category = std::forward_iterator_tag;

		command_line_iterator() = default;
		command_line_iterator(const command_line_iterator &) = default;
		command_line_iterator(command_line_iterator &&) = default;
		~command_line_iterator() = default;
		[[nodiscard]] bool operator == (const command_line_iterator &) const = default;
		command_line_iterator &operator = (const command_line_iterator &) = default;
		command_line_iterator &operator = (command_line_iterator &&) = default;

		constexpr command_line_iterator(std::string_view commandLine) :
			_CommandLine(commandLine)
		{
			if (try_increment() == false)
				throw std::logic_error("command_line_iterator::command_line_iterator(std::string_view)");
		}

		constexpr command_line_iterator operator ++ (int)
		{
			auto previous_this = *this;

			if (try_increment() == false)
				throw std::logic_error("command_line_iterator::operator ++ (int)");

			return previous_this;
		}

		[[nodiscard]] constexpr pointer operator -> () const noexcept
		{
			return &_Argument;
		}

		constexpr command_line_iterator &operator ++ ()
		{
			if (try_increment() == false)
				throw std::logic_error("command_line_iterator::operator ++ ()");

			return *this;
		}

		[[nodiscard]] constexpr reference operator * () const noexcept
		{
			return _Argument;
		}

		[[nodiscard]] constexpr command_line_iterator begin() const noexcept
		{
			return *this;
		}

		[[nodiscard]] constexpr command_line_iterator end() const noexcept
		{
			return { };
		}

		[[nodiscard]] constexpr bool try_increment() noexcept
		{
			auto isDelimiter = [] (char character) constexpr noexcept
			{
				return character == ' ' || character == '\t' || character == '\n' || character == '\r' || character == '\f' || character == '\v';
			};

			auto first = std::ranges::find_if_not(_CommandLine.begin() + _Offset, _CommandLine.end(), isDelimiter);

			if (first == _CommandLine.end())
			{
				*this = end();

				return true;
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
				return false;

			_Offset = current - _CommandLine.begin();
			_Argument = _CommandLine.substr(first - _CommandLine.begin(), current - first);

			return true;
		}

		[[nodiscard]] constexpr std::string get_normalized() const
		{
			std::string normalized;

			normalized.reserve(_Argument.length());

			auto first = _Argument.begin();
			auto current = first;
			bool skip = false;
			bool update = false;
			bool withinSingleQuotes = false;
			bool withinDoubleQuotes = false;

			for (; current != _Argument.end(); ++current)
			{
				if (skip)
				{
					if (withinDoubleQuotes == false || *current == '\"' || *current == '\\')
					{
						normalized.append(first, current - 1);
						first = current;
					}

					skip = false;
				}
				else if (*current == '\\' && withinSingleQuotes == false)
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
					normalized.append(first, current);
					first = current + 1;
					update = false;
				}
			}

			normalized.append(first, current - skip);

			return normalized;
		}

	private:
		std::string_view _CommandLine;
		std::size_t _Offset = 0;
		value_type _Argument;
	};
}

#endif


#endif
