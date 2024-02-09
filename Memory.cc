#include "Memory.h"

namespace Matteaz
{
	AllocationError::AllocationError(HANDLE heap, SIZE_T bytes) noexcept :
		Exception(),
		heap(heap),
		bytes(bytes)
	{

	}
}