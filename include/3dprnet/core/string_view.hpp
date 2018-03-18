#ifndef LIB3DPRNET_CORE_STRING_VIEW_HPP
#define LIB3DPRNET_CORE_STRING_VIEW_HPP

#if __cplusplus >= 201700 && __has_include( <string_view> )
#   include <string_view>
#   define PRNET_STRING_VIEW_NAMESPACE std
#elif __has_include( <experimental/string_view> )
#   include <experimental/string_view>
#   define PRNET_STRING_VIEW_NAMESPACE std::experimental
#else
#   include <boost/utility/string_view.hpp>
#   define PRNET_STRING_VIEW_NAMESPACE boost
#endif

namespace prnet {

template< typename CharT, typename Traits = std::char_traits< CharT > >
using basic_string_view = PRNET_STRING_VIEW_NAMESPACE::basic_string_view< CharT, Traits >;

using string_view = PRNET_STRING_VIEW_NAMESPACE::string_view;
using wstring_view = PRNET_STRING_VIEW_NAMESPACE::wstring_view;
using u16string_view = PRNET_STRING_VIEW_NAMESPACE::u16string_view;
using u32string_view = PRNET_STRING_VIEW_NAMESPACE::u32string_view;

} // namespace prnet

#undef PRNET_STRING_VIEW_NAMESPACE

#endif // LIB3DPRNET_CORE_STRING_VIEW_HPP
