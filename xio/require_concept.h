//
// require_concept.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_REQUIRE_CONCEPT_HPP
#define XIO_REQUIRE_CONCEPT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/is_applicable_property.h>
#include <xio/traits/require_concept_member.h>
#include <xio/traits/require_concept_free.h>
#include <xio/traits/static_require_concept.h>

#include <xio/detail/push_options.h>


namespace XIO_VERSIONED_NAME(require_concept_fn) {
    using std::conditional_t;
    using std::decay_t;
    using std::declval;
    using std::enable_if_t;
    using xio::is_applicable_property;
    using xio::traits::require_concept_free;
    using xio::traits::require_concept_member;
    using xio::traits::static_require_concept;

    void require_concept();

    enum overload_type {
        identity,
        call_member,
        call_free,
        ill_formed
    };

    template
    <
    typename Impl, typename T, typename Properties, typename = void,
            typename = void, typename = void, typename = void, typename = void >
    struct call_traits {
        static constexpr overload_type overload = ill_formed;
        static constexpr bool is_noexcept = false;
        typedef void result_type;
    };

    template
    <
    typename Impl, typename T, typename Property >
    struct call_traits<Impl, T, void(Property),
                std::enable_if_t<
                    is_applicable_property<
                        std::decay_t<T>,
                        std::decay_t<Property>
                    >::value
                >,
                std::enable_if_t<
                    std::decay_t<Property>::is_requirable_concept
                >,
                std::enable_if_t<
                    static_require_concept<T, Property>::is_valid
                > > {
        static constexpr overload_type overload = identity;
        static constexpr bool is_noexcept = true;
        typedef T &&result_type;
    };

    template
    <
    typename Impl, typename T, typename Property >
    struct call_traits<Impl, T, void(Property),
        std::enable_if_t<
            is_applicable_property<
                std::decay_t<T>,
                std::decay_t<Property>
            >::value
        >,
        std::enable_if_t<
            std::decay_t<Property>::is_requirable_concept
        >,
        std::enable_if_t <
        !static_require_concept<T, Property>::is_valid
    >
    ,
    std::enable_if_t<
        require_concept_member<
            typename Impl::template proxy<T>::type,
            Property
        >::is_valid
    > >
    :
    require_concept_member<
        typename Impl::template proxy<T>::type,
        Property
    >
    {
  static constexpr overload_type overload = call_member;


    };

    template
    <
    typename Impl, typename T, typename Property >
    struct call_traits<Impl, T, void(Property),
        std::enable_if_t<
            is_applicable_property<
                std::decay_t<T>,
                std::decay_t<Property>
            >::value
        >,
        std::enable_if_t<
            std::decay_t<Property>::is_requirable_concept
        >,
        std::enable_if_t <
        !static_require_concept<T, Property>::is_valid
    >
    ,
    std::enable_if_t <
            !require_concept_member<
                typename Impl::template proxy<T>::type,
                Property
            >::is_valid
            >,
            std::enable_if_t<
                require_concept_free<T, Property>::is_valid
            > >
    :
    require_concept_free<T, Property>
    {
  static constexpr overload_type overload = call_free;


    };

    struct impl {
        template<typename T>
        struct proxy {
            struct type {
                template<typename P>
                auto require_concept(P &&p)
                    noexcept(
                        noexcept(
                            std::declval<std::conditional_t<true, T, P> >().require_concept(
                                static_cast<P &&>(p))
                        )
                    )
                    -> decltype(
                        std::declval<std::conditional_t<true, T, P> >().require_concept(
                            static_cast<P &&>(p))
                    );
            };

        };

        template<typename T, typename Property>
        [[nodiscard]] constexpr std::enable_if_t<
            call_traits<impl, T, void(Property)>::overload == identity,
            typename call_traits<impl, T, void(Property)>::result_type
        >

        operator()(T &&t, Property &&) const
            noexcept(call_traits<impl, T, void(Property)>::is_noexcept) {
            return static_cast<T &&>(t);
        }

        template<typename T, typename Property>
        [[nodiscard]] constexpr std::enable_if_t<
            call_traits<impl, T, void(Property)>::overload == call_member,
            typename call_traits<impl, T, void(Property)>::result_type
        >

        operator()(T &&t, Property &&p) const
            noexcept(call_traits<impl, T, void(Property)>::is_noexcept) {
            return static_cast<T &&>(t).require_concept(static_cast<Property &&>(p));
        }

        template<typename T, typename Property>
        [[nodiscard]] constexpr std::enable_if_t<
            call_traits<impl, T, void(Property)>::overload == call_free,
            typename call_traits<impl, T, void(Property)>::result_type
        >

        operator()(T &&t, Property &&p) const
            noexcept(call_traits<impl, T, void(Property)>::is_noexcept) {
            return require_concept(static_cast<T &&>(t), static_cast<Property &&>(p));
        }
    };

    template
    <
    typename T = impl >
    struct static_instance {
        static const T instance;
    };

    template
    <
    typename T >


    const T static_instance<T>::instance = {};
} // namespace XIO_VERSIONED_NAME(require_concept_fn)
namespace xio {


    inline constexpr XIO_VERSIONED_NAME (require_concept_fn)::impl
    require_concept {
    };


    typedef XIO_VERSIONED_NAME (require_concept_fn)::impl require_concept_t;

    template<typename T, typename Property>
    struct can_require_concept :
            std::integral_constant<bool,
                XIO_VERSIONED_NAME(require_concept_fn)::call_traits<
                                                           require_concept_t, T, void(Property)>::overload !=
                                                       XIO_VERSIONED_NAME(require_concept_fn)::ill_formed> {
    };


    template<typename T, typename Property>
    constexpr bool can_require_concept_v = can_require_concept<T, Property>::value;


    template<typename T, typename Property>
    struct is_nothrow_require_concept :
            std::integral_constant<bool,
                XIO_VERSIONED_NAME(require_concept_fn)::call_traits<
                    require_concept_t, T, void(Property)>::is_noexcept> {
    };

    template<typename T, typename Property>
    constexpr bool is_nothrow_require_concept_v
            = is_nothrow_require_concept<T, Property>::value;

    template<typename T, typename Property>
    struct require_concept_result {
        typedef typename XIO_VERSIONED_NAME (require_concept_fn)::call_traits<
            require_concept_t, T, void(Property)>::result_type type;
    };

    template<typename T, typename Property>
    using require_concept_result_t =
    typename require_concept_result<T, Property>::type;


} // namespace xio


#include <xio/detail/pop_options.h>

#endif // XIO_REQUIRE_CONCEPT_HPP
