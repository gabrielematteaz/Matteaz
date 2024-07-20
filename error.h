#pragma once

#include < string >
#include < Windows.h >

namespace matteaz
{
	class exception
	{
		HANDLE heap_;
		std::size_t *references_;
		wchar_t *message_;

	protected:
		[[nodiscard]] constexpr wchar_t *message() const noexcept
		{
			return message_;
		}

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
			auto size = std::char_traits < wchar_t >::length(message) + 1;
			auto copy = static_cast < wchar_t* > (HeapAlloc(heap, 0, size * sizeof(wchar_t)));

			if (references == nullptr || copy == nullptr) {
				HeapFree(heap, 0, references);
				HeapFree(heap, 0, copy);
				return;
			}

			*references = 1;
			references_ = references;
			std::char_traits < wchar_t >::copy(copy, message, size);
			message_ = copy;
		}

		constexpr exception(const exception &other) noexcept :
			heap_(other.heap_), references_(other.references_), message_(other.message_)
		{
			if (references_ == nullptr) return;

			++*references_;
		}

		constexpr exception(exception &&other) noexcept :
			heap_(other.heap_), references_(other.references_), message_(other.message_)
		{
			other.heap_ = NULL;
			other.references_ = nullptr;
			other.message_ = nullptr;
		}

		constexpr virtual ~exception()
		{
			if (references_ == nullptr) return;

			--*references_;

			if (*references_ == 0) {
				HeapFree(heap_, 0, references_);
				HeapFree(heap_, 0, message_);
			}
		}

		constexpr exception &operator = (exception other) noexcept
		{
			swap(*this, other);

			return *this;
		}

		[[nodiscard]] constexpr HANDLE heap() const noexcept
		{
			return heap_;
		}

		[[nodiscard]] constexpr std::size_t use_count() const noexcept
		{
			return references_ == nullptr ? 0 : *references_;
		}

		[[nodiscard]] constexpr virtual const wchar_t *what() const noexcept
		{
			return message_ == nullptr ? L"matteaz::exception" : message_;
		}

		friend constexpr void swap(exception &left, exception &right) noexcept
		{
			auto heap = left.heap_;
			auto references = left.references_;
			auto message = left.message_;

			left.heap_ = right.heap_;
			left.references_ = right.references_;
			left.message_ = right.message_;
			right.heap_ = heap;
			right.references_ = references;
			right.message_ = message;
		}
	};

	struct HRESULT_error : public exception
	{
		HRESULT code_;

		constexpr HRESULT_error(HRESULT code, const wchar_t *message = nullptr, HANDLE heap = NULL) noexcept :
			exception(message, heap), code_(code)
		{

		}

		[[nodiscard]] constexpr const wchar_t *what() const noexcept override
		{
			auto message = this->message();

			return message == nullptr ? L"matteaz::HRESULT_error" : message;
		}
	};
}