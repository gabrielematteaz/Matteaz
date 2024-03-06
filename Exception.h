#pragma once

#include < Windows.h >

namespace Matteaz
{
	class Exception
	{
		HANDLE heap;
		const wchar_t* message;

	protected:
		static constexpr const wchar_t* const defaultMessages[] = {
			L"No explanatory message was provided",
			L"An invalid parameter was passed to the exception constructor",
			L"The allocation of the copy of the message failed"
		};

	public:
		explicit Exception(HANDLE heap, const wchar_t* message) noexcept;
		explicit Exception(const Exception& exception) noexcept;
		virtual ~Exception();
		Exception& operator = (const Exception& exception) noexcept;
		Exception& operator = (Exception&& exception) noexcept;

		constexpr explicit Exception() noexcept :
			heap(NULL),
			message(defaultMessages[0])
		{

		}

		constexpr explicit Exception(Exception&& exception) noexcept :
			heap(exception.heap),
			message(exception.message)
		{
			exception.heap = NULL;
			exception.message = defaultMessages[0];
		}

		[[nodiscard]] constexpr HANDLE Heap() const noexcept
		{
			return heap;
		}

		[[nodiscard]] constexpr virtual const wchar_t* Message() const noexcept
		{
			return message;
		}
	};
}