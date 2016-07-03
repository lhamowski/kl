#pragma once

#include "kl/ctti.hpp"

#include <boost/mpl/range_c.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/mpl.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/adapted/struct.hpp>

namespace kl {

template <typename T>
struct type_info<
    T,
    std::enable_if_t<std::is_same<typename boost::mpl::sequence_tag<T>::type,
                                  boost::fusion::fusion_sequence_tag>::value>>
{
    using this_type = std::decay_t<T>;
    using base_types = type_pack<>;

    static const char* name() { return typeid(T).name(); }
    static const char* full_name() { return typeid(T).name(); }

    static const bool is_reflectable = true;

    static const std::size_t num_fields =
        boost::fusion::result_of::size<T>::value;

    template <typename Type, typename Visitor>
    static void reflect(Type&& obj, Visitor&& visitor)
    {
        using indices_t = boost::mpl::range_c<unsigned, 0, num_fields>;
        visitor_wrapper2<Type&&, Visitor&&> wrapper{
            std::forward<Type>(obj), std::forward<Visitor>(visitor)};
        boost::fusion::for_each(indices_t{}, wrapper);
    }

    template <typename Type, typename Visitor>
    static void reflect(Visitor&& visitor)
    {
        using indices_t = boost::mpl::range_c<unsigned, 0, num_fields>;
        visitor_wrapper1<Type&&, Visitor&&> wrapper{
            std::forward<Visitor>(visitor)};
        boost::fusion::for_each(indices_t{}, wrapper);
    }

    template <std::size_t N>
    using type =
        typename boost::fusion::result_of::value_at_c<this_type, N>::type;

    template <typename U, std::size_t N>
    using at_type =
        typename boost::fusion::result_of::at_c<std::remove_reference_t<U>,
                                                N>::type;

    // Returns reference to N-th element in a Sequence
    template <std::size_t N, typename Type>
    static at_type<Type, N> at(Type&& instance)
    {
        static_assert(!std::is_rvalue_reference<Type&&>::value,
                      "instance can't be a rvalue reference");
        return boost::fusion::at_c<N>(std::forward<Type>(instance));
    }

    template <std::size_t N>
    static const char* field_name()
    {
        return boost::fusion::extension::struct_member_name<this_type,
                                                            N>::call();
    }

private:
    template <typename Type, typename Visitor>
    struct visitor_wrapper2
    {
        Type instance;
        Visitor visitor;

        template <typename Index>
        void operator()(Index) const
        {
            using non_ref = std::remove_reference_t<Type>;
            using member_type =
                typename boost::fusion::result_of::value_at<non_ref,
                                                            Index>::type;
            using field_info = detail::field_info<non_ref, member_type>;

            std::forward<Visitor>(visitor)(
                field_info{boost::fusion::at<Index>(instance),
                           boost::fusion::extension::struct_member_name<
                               T, Index::value>::call()});
        }
    };

    template <typename Type, typename Visitor>
    struct visitor_wrapper1
    {
        Visitor visitor;

        template <typename Index>
        void operator()(Index) const
        {
            using non_ref = std::remove_reference_t<Type>;
            using member_type =
                typename boost::fusion::result_of::value_at<non_ref,
                                                            Index>::type;

            using field_info = detail::field_type_info<non_ref, member_type>;
            std::forward<Visitor>(visitor)(
                field_info{boost::fusion::extension::struct_member_name<
                    T, Index::value>::call()});
        }
    };
};
} // namespace kl
