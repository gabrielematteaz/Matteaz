#pragma once

#include < string >
#include < Windows.h >

namespace matteaz
{
	class exception
	{
	protected:
		HANDLE heap_;
		std::size_t *references_;
		wchar_t *message_;

	public:
		constexpr explicit exception(const wchar_t *message = nullptr, HANDLE heap = NULL) noexcept :
			heap_(NULL), references_(nullptr), message_(nullptr)
		{
			if (heap == NULL) {
				message_ = const_cast < wchar_t* > (message);
				return;
			}

			if (message == nullptr) return;

			auto references = static_cast < std::size_t* > (HeapAlloc(heap, 0, sizeof(std::size_t)));

			if (references == nullptr) return;

			auto size = std::char_traits < wchar_t >::length(message) + 1;
			auto copy = static_cast < wchar_t* > (HeapAlloc(heap, 0, size * sizeof(wchar_t)));

			if (copy == nullptr) {
				HeapFree(heap, 0, references);
				return;
			}

			heap_ = heap;
			*references = 1;
			references_ = references;
			std::char_traits < wchar_t >::copy(copy, message, size);
			message_ = copy;
		}

		constexpr exception(const exception &other) noexcept :
			heap_(other.heap_),
			references_(other.references_),
			message_(other.message_)
		{
			if (references_ == nullptr) return;

			++*references_;
		}

		constexpr exception(exception &&other) noexcept :
			heap_(other.heap_),
			references_(other.references_),
			message_(other.message_)
		{
			other.heap_ = NULL;
			other.references_ = nullptr;
			other.message_ = nullptr;
		}

		constexpr virtual ~exception()
		{
			if (heap_ == NULL) return;

			--*references_;

			if (*references_ == 0) {
				HeapFree(heap_, 0, references_);
				HeapFree(heap_, 0, message_);
			}
		}

		constexpr exception &operator = (const exception &other) noexcept
		{
			if (this == &other) return *this;

			this->~exception();
			heap_ = other.heap_;
			references_ = other.references_;
			message_ = other.message_;

			if (references_ == nullptr) return *this;

			++*references_;

			return *this;
		}

		constexpr exception &operator = (exception &&other) noexcept
		{
			if (this == &other) return *this;

			this->~exception();
			heap_ = other.heap_;
			references_ = other.references_;
			message_ = other.message_;
			other.heap_ = NULL;
			other.references_ = nullptr;
			other.message_ = nullptr;

			return *this;
		}

		[[nodiscard]] constexpr HANDLE heap() const noexcept
		{
			return heap_;
		}

		[[nodiscard]] constexpr std::size_t references() const noexcept
		{
			return references_ == nullptr ? 0 : *references_;
		}

		[[nodiscard]] constexpr virtual const wchar_t *message() const noexcept
		{
			return message_ == nullptr ? L"matteaz::exception" : message_;
		}
	};

	struct HRESULT_error : public exception
	{
		HRESULT code_;

		constexpr explicit HRESULT_error(HRESULT code, const wchar_t *message = nullptr, HANDLE heap = nullptr) noexcept :
			exception(message, heap), code_(code)
		{
		
		}

		[[nodiscard]] constexpr const wchar_t *message() const noexcept
		{
			return message_ == nullptr ? L"matteaz::HRESULT_error" : message_;
		}
	};
}