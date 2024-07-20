#include "filesystem.h"

namespace matteaz
{
	directory_iterator::directory_iterator(const wchar_t * path, const allocator < state > &allocator) :
		state_(allocator, INVALID_HANDLE_VALUE)
	{
		wchar_t *file_name;
		auto code = PathAllocCombine(path, L"*", PATHCCH_ALLOW_LONG_PATHS, &file_name);

		if (code != S_OK) throw HRESULT_error(code, L"PathAllocCombine failed");

		auto state = state_.get();

		state->find_file_ = FindFirstFileW(file_name, &state->find_data_);
		LocalFree(file_name);
	}

	directory_iterator directory_iterator::begin() const noexcept
	{
		return *this;
	}
}