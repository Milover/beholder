/*---------------------------------------------------------------------------*\

	beholder - Copyright (C) 2024 P. Milovic

-------------------------------------------------------------------------------
License
	See the LICENSE file for license information.

Description
	A base class template to simplify wrapping C-structs.

\*---------------------------------------------------------------------------*/

#ifdef __cplusplus
#ifndef BEHOLDER_C_WRAPPER_BASE_H
#define BEHOLDER_C_WRAPPER_BASE_H

#include <type_traits>
#include <utility>

#include "util/Traits.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace beholder
{
namespace capi
{
namespace internal
{

/*---------------------------------------------------------------------------*\
                      Class CWrapperBase Declaration
\*---------------------------------------------------------------------------*/

namespace detail
{
	template<typename T>
	struct DfltCtor { T operator()() { return T {}; } };

	template<typename T>
	struct DfltDtor { void operator()(T&) {} };
}

template
<
	typename T,
	typename Ctor = detail::DfltCtor<T>,
	typename Dtor = detail::DfltDtor<T>,
	typename = std::enable_if_t
	<
		std::is_trivial_v<T>
	 && std::is_standard_layout_v<T>
	>,
	typename = std::enable_if_t
	<
		std::is_default_constructible_v<Ctor>
	 && std::is_invocable_r_v<T, Ctor>
	>,
	typename = std::enable_if_t
	<
		std::is_default_constructible_v<Dtor>
	 && std::is_invocable_r_v<void, Dtor, T&>
	>
>
class CWrapperBase
{
protected:

	// Private data

		//- The underlying C struct
		T t_;

public:

	using type = T;

	// Constructors

		//- Default constructor
		CWrapperBase()
		:
			t_ {Ctor {}()}
		{}

		//- Construct by forwarding arguments to T's constructor
		template<typename... Args>
		CWrapperBase(Args&&... args)
		:
			t_ {std::forward<Args>(args)...}
		{}

		//- Forwarding constructor from a T
		template<typename U, typename = enableIfSame_t<U, T>>
		CWrapperBase(U&& t)
		:
			t_ {std::forward<U>(t)}
		{}

		//- Default copy constructor
		CWrapperBase(const CWrapperBase&) = default;

		//- Default move constructor
		CWrapperBase(CWrapperBase&&) = default;


	//- Destructor
	virtual ~CWrapperBase()
	{
		Dtor {}(t_);
	}


	// Public member functions

		//- Return a reference to the underlying T
		virtual T& ref()
		{
			return t_;
		}

		//- Return a const reference to the underlying T
		virtual const T& cRef() const
		{
			return t_;
		}

		//- Return a copy of the underlying T
		virtual T toC() const
		{
			return t_;
		}

		//- Move the underlying T out
		virtual T&& moveToC() &&
		{
			return std::move(t_);
		}

	// Member operators

		//- Implicit conversion to a T&
		//operator T&() { return t_; }

		//- Implicit conversion to a const T&
		//operator const T&() const { return t_; }

		//- Default copy assignment operator
		CWrapperBase& operator=(const CWrapperBase&) = default;

		//- Default move assignment operator
		CWrapperBase& operator=(CWrapperBase&&) = default;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace internal
} // End namespace capi
} // End namespace beholder

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif
#endif	// __cplusplus

// ************************************************************************* //
