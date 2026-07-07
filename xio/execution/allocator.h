//
// execution/allocator.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_EXECUTION_ALLOCATOR_HPP
#define ASIO_EXECUTION_ALLOCATOR_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <xio/detail/config.h>
#include <xio/detail/type_traits.h>
#include <xio/execution/executor.h>
#include <xio/is_applicable_property.h>
#include <xio/traits/query_static_constexpr_member.h>
#include <xio/traits/static_query.h>

#include <xio/detail/push_options.h>

namespace xio {


#if defined(GENERATING_DOCUMENTATION)

    namespace execution {
        /// A property to describe which allocator an executor will use to allocate the
/// memory required to store a submitted function object.
        template<typename ProtoAllocator>
        struct allocator_t {
            /// The allocator_t property applies to executors.
            template<typename T>
            static constexpr bool is_applicable_property_v = is_executor_v<T>;

            /// The allocator_t property can be required.
            static constexpr bool is_requirable = true;

            /// The allocator_t property can be preferred.
            static constexpr bool is_preferable = true;

            /// Default constructor.
            constexpr allocator_t();

            /// Obtain the allocator stored in the allocator_t property object.
            /**
   * Present only if @c ProtoAllocator is non-void.
   */
            constexpr ProtoAllocator value() const;

            /// Create an allocator_t object with a different allocator.
            /**
   * Present only if @c ProtoAllocator is void.
   */
            template<typename OtherAllocator>
            allocator_t<OtherAllocator operator()(const OtherAllocator & a);
        };

        /// A special value used for accessing the allocator_t property.
        constexpr allocator_t<void> allocator;
    } // namespace execution

#else // defined(GENERATING_DOCUMENTATION)




    namespace execution {
        template<typename ProtoAllocator>
        struct allocator_t {
            template<typename T>
            static constexpr bool is_applicable_property_v = is_executor<T>::value;

            static constexpr bool is_requirable = true;
            static constexpr bool is_preferable = true;

            template<typename T>
            struct static_proxy {
                struct type {
                    template<typename P>
                    static constexpr auto query(P &&p)
                        noexcept(
                            noexcept(
                                conditional_t < true, T, P > ::query(static_cast<P &&>(p))
                            )
                        )
                        -> decltype(
                            conditional_t < true, T, P > ::query(static_cast<P &&>(p))
                        ) {
                        return T::query(static_cast<P &&>(p));
                    }
                };

            };

            template<typename T>
            struct query_static_constexpr_member :
                    traits::query_static_constexpr_member<
                        typename static_proxy<T>::type, allocator_t> {
            };


            template<typename T>
            static constexpr typename query_static_constexpr_member<T>::result_type
            static_query()
                noexcept(query_static_constexpr_member<T>::is_noexcept) {
                return query_static_constexpr_member<T>::value();
            }

            template<typename E, typename T = decltype(allocator_t::static_query<E>())>
            static constexpr const T static_query_v = allocator_t::static_query<E>();


            constexpr ProtoAllocator value() const {
                return a_;
            }

        private:
            friend struct allocator_t<void>;

            explicit constexpr allocator_t(const ProtoAllocator &a)
                : a_(a) {
            }

            ProtoAllocator a_;
        };


        template<typename ProtoAllocator>
        template<typename E, typename T>
        const T allocator_t<ProtoAllocator>::static_query_v;


        template<>
        struct allocator_t<void> {
            template<typename T>
            static constexpr bool is_applicable_property_v = is_executor<T>::value;

            static constexpr bool is_requirable = true;
            static constexpr bool is_preferable = true;

            constexpr allocator_t() {
            }

            template<typename T>
            struct static_proxy {
                struct type {
                    template<typename P>
                    static constexpr auto query(P &&p)
                        noexcept(
                            noexcept(
                                conditional_t < true, T, P > ::query(static_cast<P &&>(p))
                            )
                        )
                        -> decltype(
                            conditional_t < true, T, P > ::query(static_cast<P &&>(p))
                        ) {
                        return T::query(static_cast<P &&>(p));
                    }
                };

            };

            template<typename T>
            struct query_static_constexpr_member :
                    traits::query_static_constexpr_member<
                        typename static_proxy<T>::type, allocator_t> {
            };


            template<typename T>
            static constexpr typename query_static_constexpr_member<T>::result_type
            static_query()
                noexcept(query_static_constexpr_member<T>::is_noexcept) {
                return query_static_constexpr_member<T>::value();
            }

            template<typename E, typename T = decltype(allocator_t::static_query<E>())>
            static constexpr const T static_query_v = allocator_t::static_query<E>();


            template<typename OtherProtoAllocator>
            constexpr allocator_t<OtherProtoAllocator> operator()(
                const OtherProtoAllocator &a) const {
                return allocator_t<OtherProtoAllocator>(a);
            }
        };


        template<typename E, typename T>
        const T allocator_t<void>::static_query_v;

        inline constexpr allocator_t<void> allocator;
    } // namespace execution



#endif // defined(GENERATING_DOCUMENTATION)


} // namespace xio

#include <xio/detail/pop_options.h>

#endif // ASIO_EXECUTION_ALLOCATOR_HPP
