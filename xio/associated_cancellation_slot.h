//
// associated_cancellation_slot.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef XIO_ASSOCIATED_CANCELLATION_SLOT_HPP
#define XIO_ASSOCIATED_CANCELLATION_SLOT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/associator.h>
#include <xio/cancellation_signal.h>
#include <functional>
#include <xio/detail/type_traits.h>

#include <xio/detail/push_options.h>

namespace xio {
    template<typename T, typename CancellationSlot>
    struct associated_cancellation_slot;

    namespace detail {
        template<typename T, typename = void>
        struct has_cancellation_slot_type : std::false_type {
        };

        template<typename T>
        struct has_cancellation_slot_type<T, void_t<typename T::cancellation_slot_type> >
                : std::true_type {
        };

        template<typename T, typename S, typename = void, typename = void>
        struct associated_cancellation_slot_impl {
            typedef void xio_associated_cancellation_slot_is_unspecialised;

            typedef S type;

            static type get(const T &) noexcept {
                return type();
            }

            static const type &get(const T &, const S &s) noexcept {
                return s;
            }
        };

        template<typename T, typename S>
        struct associated_cancellation_slot_impl<T, S,
                    void_t<typename T::cancellation_slot_type> > {
            typedef typename T::cancellation_slot_type type;

            static auto get(const T &t) noexcept
                -> decltype(t.get_cancellation_slot()) {
                return t.get_cancellation_slot();
            }

            static auto get(const T &t, const S &) noexcept
                -> decltype(t.get_cancellation_slot()) {
                return t.get_cancellation_slot();
            }
        };

        template<typename T, typename S>
        struct associated_cancellation_slot_impl<T, S,
                    std::enable_if_t<
                        !has_cancellation_slot_type<T>::value
                    >
                    ,
                    void_t<
                        typename associator<associated_cancellation_slot, T, S>::type
                    >
                >
                :
                associator<associated_cancellation_slot, T, S> {
        };
    } // namespace detail

    /// Traits type used to obtain the cancellation_slot associated with an object.
    /**
 * A program may specialise this traits type if the @c T template parameter in
 * the specialisation is a user-defined type. The template parameter @c
 * CancellationSlot shall be a type meeting the CancellationSlot requirements.
 *
 * Specialisations shall meet the following requirements, where @c t is a const
 * reference to an object of type @c T, and @c s is an object of type @c
 * CancellationSlot.
 *
 * @li Provide a nested typedef @c type that identifies a type meeting the
 * CancellationSlot requirements.
 *
 * @li Provide a noexcept static member function named @c get, callable as @c
 * get(t) and with return type @c type or a (possibly const) reference to @c
 * type.
 *
 * @li Provide a noexcept static member function named @c get, callable as @c
 * get(t,s) and with return type @c type or a (possibly const) reference to @c
 * type.
 */
    template<typename T, typename CancellationSlot = cancellation_slot>
    struct associated_cancellation_slot
            : detail::associated_cancellation_slot_impl<T, CancellationSlot>
    {
    };

    /// Helper function to obtain an object's associated cancellation_slot.
    /**
 * @returns <tt>associated_cancellation_slot<T>::get(t)</tt>
 */
    template<typename T>
    [[nodiscard]] inline

    typename associated_cancellation_slot<T>::type
    get_associated_cancellation_slot(const T &t) noexcept {
        return associated_cancellation_slot<T>::get(t);
    }

    /// Helper function to obtain an object's associated cancellation_slot.
    /**
 * @returns <tt>associated_cancellation_slot<T,
 * CancellationSlot>::get(t, st)</tt>
 */
    template<typename T, typename CancellationSlot>
    [[nodiscard]] inline auto get_associated_cancellation_slot(
        const T &t, const CancellationSlot &st) noexcept
        -> decltype(associated_cancellation_slot<T, CancellationSlot>::get(t, st)) {
        return associated_cancellation_slot<T, CancellationSlot>::get(t, st);
    }

    template<typename T, typename CancellationSlot = cancellation_slot>
    using associated_cancellation_slot_t =
    typename associated_cancellation_slot<T, CancellationSlot>::type;

    namespace detail {
        template<typename T, typename S, typename = void>
        struct associated_cancellation_slot_forwarding_base {
        };

        template<typename T, typename S>
        struct associated_cancellation_slot_forwarding_base<T, S,
                    std::enable_if_t<
                        std::is_same<
                            typename associated_cancellation_slot<T,
                                S>::xio_associated_cancellation_slot_is_unspecialised,
                            void>::value
                    >
                > {
            typedef void xio_associated_cancellation_slot_is_unspecialised;
        };
    } // namespace detail

    /// Specialisation of associated_cancellation_slot for @c
/// std::reference_wrapper.
    template<typename T, typename CancellationSlot>
    struct associated_cancellation_slot<std::reference_wrapper<T>, CancellationSlot>
            : detail::associated_cancellation_slot_forwarding_base<T, CancellationSlot>
    {
        /// Forwards @c type to the associator specialisation for the unwrapped type
  /// @c T.
        typedef typename associated_cancellation_slot<T, CancellationSlot>::type type;

        /// Forwards the request to get the cancellation slot to the associator
        /// specialisation for the unwrapped type @c T.
        static type get(std::reference_wrapper<T> t) noexcept {
            return associated_cancellation_slot<T, CancellationSlot>::get(t.get());
        }

        /// Forwards the request to get the cancellation slot to the associator
  /// specialisation for the unwrapped type @c T.
        static auto get(std::reference_wrapper<T> t, const CancellationSlot &s) noexcept
            -> decltype(
                associated_cancellation_slot<T, CancellationSlot>::get(t.get(), s)) {
            return associated_cancellation_slot<T, CancellationSlot>::get(t.get(), s);
        }
    };
} // namespace xio

#include <xio/detail/pop_options.h>

#endif // XIO_ASSOCIATED_CANCELLATION_SLOT_HPP
