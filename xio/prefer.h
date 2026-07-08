//
// prefer.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_PREFER_HPP
#define XIO_PREFER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/is_applicable_property.h>
#include <xio/traits/prefer_free.h>
#include <xio/traits/prefer_member.h>
#include <xio/traits/require_free.h>
#include <xio/traits/require_member.h>
#include <xio/traits/static_require.h>

#include <xio/detail/push_options.h>


namespace XIO_VERSIONED_NAME(prefer_fn) {
    using std::conditional_t;
    using std::decay_t;
    using std::declval;
    using std::enable_if_t;
    using xio::is_applicable_property;
    using xio::traits::prefer_free;
    using xio::traits::prefer_member;
    using xio::traits::require_free;
    using xio::traits::require_member;
    using xio::traits::static_require;

    void prefer();
    void require();

    enum overload_type {
        identity,
        call_require_member,
        call_require_free,
        call_prefer_member,
        call_prefer_free,
        two_props,
        n_props,
        ill_formed
    };

    template
    <
    typename Impl, typename T, typename Properties,
            typename = void, typename = void, typename = void, typename = void,
            typename = void, typename = void, typename = void >
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
                    std::decay_t<Property>::is_preferable
                >,
                std::enable_if_t<
                    static_require<T, Property>::is_valid
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
            std::decay_t<Property>::is_preferable
        >,
        std::enable_if_t <
        !static_require<T, Property>::is_valid
    >
    ,
    std::enable_if_t<
        require_member<typename Impl::template proxy<T>::type, Property>::is_valid
    > >
    :
    require_member<typename Impl::template proxy<T>::type, Property>
    {
  static constexpr overload_type overload = call_require_member;


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
            std::decay_t<Property>::is_preferable
        >,
        std::enable_if_t <
        !static_require<T, Property>::is_valid
    >
    ,
    std::enable_if_t <
            !require_member<typename Impl::template proxy<T>::type, Property>::is_valid
            >,
            std::enable_if_t<
                require_free<T, Property>::is_valid
            > >
    :
    require_free<T, Property>
    {
  static constexpr overload_type overload = call_require_free;


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
            std::decay_t<Property>::is_preferable
        >,
        std::enable_if_t <
        !static_require<T, Property>::is_valid
    >
    ,
    std::enable_if_t <
            !require_member<typename Impl::template proxy<T>::type, Property>::is_valid
            >,
            std::enable_if_t <
            !require_free<T, Property>::is_valid
            >,
            std::enable_if_t<
                prefer_member<typename Impl::template proxy<T>::type, Property>::is_valid
            > >
    :
    prefer_member<typename Impl::template proxy<T>::type, Property>
    {
  static constexpr overload_type overload = call_prefer_member;


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
            std::decay_t<Property>::is_preferable
        >,
        std::enable_if_t <
        !static_require<T, Property>::is_valid
    >
    ,
    std::enable_if_t <
            !require_member<typename Impl::template proxy<T>::type, Property>::is_valid
            >,
            std::enable_if_t <
            !require_free<T, Property>::is_valid
            >,
            std::enable_if_t <
            !prefer_member<typename Impl::template proxy<T>::type, Property>::is_valid
            >,
            std::enable_if_t<
                prefer_free<T, Property>::is_valid
            > >
    :
    prefer_free<T, Property>
    {
  static constexpr overload_type overload = call_prefer_free;


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
            std::decay_t<Property>::is_preferable
        >,
        std::enable_if_t <
        !static_require<T, Property>::is_valid
    >
    ,
    std::enable_if_t <
            !require_member<typename Impl::template proxy<T>::type, Property>::is_valid
            >,
            std::enable_if_t <
            !require_free<T, Property>::is_valid
            >,
            std::enable_if_t <
            !prefer_member<typename Impl::template proxy<T>::type, Property>::is_valid
            >,
            std::enable_if_t <
            !prefer_free<T, Property>::is_valid
            >>
    {
        static constexpr overload_type overload = identity;
        static constexpr bool is_noexcept = true;

        typedef T &&result_type;
    };

    template
    <
    typename Impl, typename T, typename P0, typename P1 >
    struct call_traits<Impl, T, void(P0, P1),
        std::enable_if_t <
        call_traits<Impl, T, void(P0)>::overload != ill_formed
    >
    ,
    std::enable_if_t <
            call_traits<
                Impl,
                typename call_traits<Impl, T, void(P0)>::result_type,
                void(P1)
            >::overload != ill_formed
            >>
    {
        static constexpr overload_type overload = two_props;

        static constexpr bool is_noexcept =
        (
            call_traits<Impl, T, void(P0)>::is_noexcept
            &&
            call_traits<
                Impl,
                typename call_traits<Impl, T, void(P0)>::result_type,
                void(P1)
            >::is_noexcept
        );

        typedef std::decay_t<
            typename call_traits<
                Impl,
                typename call_traits<Impl, T, void(P0)>::result_type,
                void(P1)
            >::result_type
        > result_type;
    };

    template
    <
    typename Impl, typename T, typename P0,
            typename P1, typename

    ...
    PN >
    struct call_traits<Impl, T, void(P0, P1, PN...),
        std::enable_if_t <
        call_traits<Impl, T, void(P0)>::overload != ill_formed
    >
    ,
    std::enable_if_t <
            call_traits<
                Impl,
                typename call_traits<Impl, T, void(P0)>::result_type,
                void(P1, PN...)
            >::overload != ill_formed
            >>
    {
        static constexpr overload_type overload = n_props;

        static constexpr bool is_noexcept =
        (
            call_traits<Impl, T, void(P0)>::is_noexcept
            &&
            call_traits<
                Impl,
                typename call_traits<Impl, T, void(P0)>::result_type,
                void(P1, PN...)
            >::is_noexcept
        );

        typedef std::decay_t<
            typename call_traits<
                Impl,
                typename call_traits<Impl, T, void(P0)>::result_type,
                void(P1, PN...)
            >::result_type
        > result_type;
    };

    struct impl {
        template<typename T>
        struct proxy {
            struct type {
                template<typename P>
                auto require(P &&p)
                    noexcept(
                        noexcept(
                            std::declval<std::conditional_t<true, T, P> >().require(static_cast<P &&>(p))
                        )
                    )
                    -> decltype(
                        std::declval<std::conditional_t<true, T, P> >().require(static_cast<P &&>(p))
                    );

                template<typename P>
                auto prefer(P &&p)
                    noexcept(
                        noexcept(
                            std::declval<std::conditional_t<true, T, P> >().prefer(static_cast<P &&>(p))
                        )
                    )
                    -> decltype(
                        std::declval<std::conditional_t<true, T, P> >().prefer(static_cast<P &&>(p))
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
            call_traits<impl, T, void(Property)>::overload == call_require_member,
            typename call_traits<impl, T, void(Property)>::result_type
        >

        operator()(T &&t, Property &&p) const
            noexcept(call_traits<impl, T, void(Property)>::is_noexcept) {
            return static_cast<T &&>(t).require(static_cast<Property &&>(p));
        }

        template<typename T, typename Property>
        [[nodiscard]] constexpr std::enable_if_t<
            call_traits<impl, T, void(Property)>::overload == call_require_free,
            typename call_traits<impl, T, void(Property)>::result_type
        >

        operator()(T &&t, Property &&p) const
            noexcept(call_traits<impl, T, void(Property)>::is_noexcept) {
            return require(static_cast<T &&>(t), static_cast<Property &&>(p));
        }

        template<typename T, typename Property>
        [[nodiscard]] constexpr std::enable_if_t<
            call_traits<impl, T, void(Property)>::overload == call_prefer_member,
            typename call_traits<impl, T, void(Property)>::result_type
        >

        operator()(T &&t, Property &&p) const
            noexcept(call_traits<impl, T, void(Property)>::is_noexcept) {
            return static_cast<T &&>(t).prefer(static_cast<Property &&>(p));
        }

        template<typename T, typename Property>
        [[nodiscard]] constexpr std::enable_if_t<
            call_traits<impl, T, void(Property)>::overload == call_prefer_free,
            typename call_traits<impl, T, void(Property)>::result_type
        >

        operator()(T &&t, Property &&p) const
            noexcept(call_traits<impl, T, void(Property)>::is_noexcept) {
            return prefer(static_cast<T &&>(t), static_cast<Property &&>(p));
        }

        template<typename T, typename P0, typename P1>
        [[nodiscard]] constexpr std::enable_if_t<
            call_traits<impl, T, void(P0, P1)>::overload == two_props,
            typename call_traits<impl, T, void(P0, P1)>::result_type
        >

        operator()(T &&t, P0 &&p0, P1 &&p1) const
            noexcept(call_traits<impl, T, void(P0, P1)>::is_noexcept) {
            return (*this)(
                (*this)(static_cast<T &&>(t), static_cast<P0 &&>(p0)),
                static_cast<P1 &&>(p1));
        }

        template<typename T, typename P0, typename P1,
            typename... PN>
        [[nodiscard]] constexpr std::enable_if_t<
            call_traits<impl, T, void(P0, P1, PN...)>::overload == n_props,
            typename call_traits<impl, T, void(P0, P1, PN...)>::result_type
        >

        operator()(T &&t, P0 &&p0, P1 &&p1, PN &&... pn) const
            noexcept(call_traits<impl, T, void(P0, P1, PN...)>::is_noexcept) {
            return (*this)(
                (*this)(static_cast<T &&>(t), static_cast<P0 &&>(p0)),
                static_cast<P1 &&>(p1), static_cast<PN &&>(pn)...);
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
} // namespace XIO_VERSIONED_NAME(prefer_fn)
namespace xio {


    inline constexpr XIO_VERSIONED_NAME (prefer_fn)::impl prefer{};


    typedef XIO_VERSIONED_NAME (prefer_fn)::impl prefer_t;

    template<typename T, typename... Properties>
    struct can_prefer :
            std::integral_constant<bool,
                XIO_VERSIONED_NAME(prefer_fn)::call_traits<
                                                  prefer_t, T, void(Properties...)>::overload
                                              != XIO_VERSIONED_NAME(prefer_fn)::ill_formed> {
    };


    template<typename T, typename... Properties>
    constexpr bool can_prefer_v
            = can_prefer<T, Properties...>::value;


    template<typename T, typename... Properties>
    struct is_nothrow_prefer :
            std::integral_constant<bool,
                XIO_VERSIONED_NAME(prefer_fn)::call_traits<
                    prefer_t, T, void(Properties...)>::is_noexcept> {
    };


    template<typename T, typename... Properties>
    constexpr bool is_nothrow_prefer_v = is_nothrow_prefer<T, Properties...>::value;

    template<typename T, typename... Properties>
    struct prefer_result {
        typedef typename XIO_VERSIONED_NAME (prefer_fn)::call_traits<
            prefer_t, T, void(Properties...)>::result_type type;
    };

    template<typename T, typename... Properties>
    using prefer_result_t = typename prefer_result<T, Properties...>::type;


} // namespace xio


#include <xio/detail/pop_options.h>

#endif // XIO_PREFER_HPP
