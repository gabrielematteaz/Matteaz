#pragma once

#include < type_traits >

namespace Matteaz
{
	template < typename Type >
	concept NoExtentsDestructible = std::is_nothrow_destructible_v < std::remove_all_extents_t < Type > >;
}