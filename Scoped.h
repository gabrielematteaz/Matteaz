#pragma once

namespace Matteaz
{
	template < class Type, class Deleter >
	class Scoped
	{
		Type& object;
		Deleter& function;

	public:
		Scoped(const Scoped&) = delete;
		Scoped(Scoped&&) = delete;
		Scoped& operator = (const Scoped&) = delete;
		Scoped& operator = (Scoped&&) = delete;

		constexpr Scoped(Type& object, Deleter& function) noexcept :
			object(object),
			function(function)
		{

		}

		~Scoped()
		{
			function(object);
		}
	};
}