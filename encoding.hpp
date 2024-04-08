#ifndef MATTEAZ_ENCODING_HPP
#define MATTEAZ_ENCODING_HPP

#include <iterator>

namespace matteaz
{
	template <
		std::forward_iterator dest_iter_t,
		std::forward_iterator src_iter_t >
	struct encode_result_t
	{
		dest_iter_t dest_first;
		src_iter_t src_first;
	};

	template <
		std::forward_iterator dest_iter_t,
		std::forward_iterator src_iter_t >
	constexpr encode_result_t < dest_iter_t, src_iter_t > encode_utf8_utf16(
		dest_iter_t dest_first, std::iter_difference_t < dest_iter_t > dest_size,
		src_iter_t src_first, std::iter_difference_t < src_iter_t > src_size) noexcept
	{
		while (src_size > 0)
		{
			if (0xD800 <= src_first[0])
			{
				/* UTF-16 surrogate pairs always translate to 4 UTF-8 code units */
				if (src_size > 1 && 0xDC00 <= src_first[1] && dest_size > 3)
				{
					char32_t chr = 0x10000 + ((src_first[0] & 0x3FF) << 10) | (src_first[1] & 0x3FF);

					dest_first[0] = 0xF0 | (chr >> 18);
					dest_first[1] = 0x80 | ((chr >> 12) & 0x3F);
					dest_first[2] = 0x80 | ((chr >> 6) & 0x3F);
					dest_first[3] = 0x80 | (chr & 0x3F);
					dest_first += 4;
					dest_size -= 4;
					src_first += 2;
					src_size -= 2;

					continue;
				}
			}
			else
			{
				char32_t chr = *src_first;

				if (chr < 0x80)
				{
					if (dest_size > 0)
					{
						*dest_first = *src_first;
						dest_first++;
						dest_size--;
						src_first++;
						src_size--;

						continue;
					}
				}
				else if (chr < 0x800)
				{
					if (dest_size > 1)
					{
						dest_first[0] = 0xC0 | (chr >> 6);
						dest_first[1] = 0x80 | (chr & 0x3F);
						dest_first += 2;
						dest_size -= 2;
						src_first++;
						src_size--;

						continue;
					}
				}
				else if (dest_size > 2)
				{
					dest_first[0] = 0xE0 | (chr >> 12);
					dest_first[1] = 0x80 | ((chr >> 6) & 0x3F);
					dest_first[2] = 0x80 | (chr & 0x3F);
					dest_first += 3;
					dest_size -= 3;
					src_first++;
					src_size--;

					continue;				
				}
			}

			break;
		}

		return { dest_first, src_first };
	}
}

#endif