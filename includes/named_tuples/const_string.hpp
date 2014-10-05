#ifndef NAMED_TUPLES_CONST_STRING_HEADER
#define NAMED_TUPLES_CONST_STRING_HEADER
#include <cstddef>
#include "type_traits.hpp"

namespace named_tuples {

using str_id_type = unsigned long long;

unsigned constexpr const_str_size(char const *input) {
  return *input ?  1u + const_str_size(input + 1) : 0;
}

unsigned constexpr const_hash(char const *input) {
  return *input ?  static_cast<unsigned int>(*input) + 33 * const_hash(input + 1) : 5381;
}

class const_string {
  const char * data_;
  std::size_t size_;
 public:
  template<std::size_t N> constexpr const_string(const char(&data)[N]) : data_(data), size_(N-1) {}
  constexpr const_string(const char * data) : data_(data), size_(const_str_size(data)) {}
  constexpr const_string(const char * data, std::size_t size) : data_(data), size_(size) {}
  constexpr char operator[](std::size_t n) const {
    return n < size_ ? data_[n] : throw std::out_of_range("");
  }
  constexpr std::size_t size() const { return size_; }
  constexpr char const* str() const { return data_; }
  constexpr operator unsigned() const { return const_hash(data_); }
};

//#ifdef NAMED_TUPLES_CPP14

template <char Value> struct constexpr_char {
  static constexpr char value = Value;
};

template <char ... Chars> class constexpr_string {
  //using char_list = type_list<Chars ..., '\0'>;
  using char_list = type_list<constexpr_char<Chars> ..., constexpr_char<'\0'>>;
  const char data_[sizeof ... (Chars) + 1u];
  const size_t size_;
 public:
  constexpr constexpr_string() : data_ {Chars..., '\0'}, size_(index_of<char_list, constexpr_char<'\0'>>()) {}
  constexpr char const* str() const { return data_; }
  constexpr size_t size() const { return size_; }
  constexpr char operator[] (size_t index) const { return data_[index]; }
};


// Trait to cut out null chars at the end
template <typename String, char ... Chars> struct strip_null_impl;
// Non-const version
template <char ... Head, char Current, char ... Tail> struct strip_null_impl<constexpr_string<Head ...>, Current, Tail ...>  {
  using type = typename std::conditional<
    Current == '\0'
    , typename strip_null_impl<constexpr_string<Head ...>, Tail ...>::type
    , typename strip_null_impl<constexpr_string<Head ..., Current>, Tail ...>::type>::type;
};
template <char ... T> struct strip_null_impl<constexpr_string<T ...>> {
  using type = constexpr_string<T ...>;
};

template <typename String> struct strip_null;
template <char ... T> struct strip_null<constexpr_string<T ...>> {
  using type = typename strip_null_impl<constexpr_string<>, T ...>::type;
};
// Const version
template <char ... Head, char Current, char ... Tail> struct strip_null_impl<const constexpr_string<Head ...>, Current, Tail ...>  {
  using type = typename std::conditional<
    Current == '\0'
    , typename strip_null_impl<const constexpr_string<Head ...>, Tail ...>::type
    , typename strip_null_impl<const constexpr_string<Head ..., Current>, Tail ...>::type>::type;
};
template <char ... T> struct strip_null_impl<const constexpr_string<T ...>> {
  using type = const constexpr_string<T ...>;
};
template <char ... T> struct strip_null<const constexpr_string<T ...>> {
  using type = typename strip_null_impl<const constexpr_string<>, T ...>::type;
};


template <typename ... Strings> struct concat_impl;

template <typename Head, typename ... Tail> struct concat_impl<Head, Tail...> {
  using type = typename concat_impl<Head, typename concat_impl<Tail...>::type>::type;
};

template <> struct concat_impl<> {
  using type = constexpr_string<>;
};

template <char ... Chars1, char ... Chars2> struct concat_impl<const constexpr_string<Chars1...>, const constexpr_string<Chars2...>> {
  using type = const constexpr_string<Chars1..., Chars2...>;
};
template <char ... Chars1, char ... Chars2> struct concat_impl<constexpr_string<Chars1...>, constexpr_string<Chars2...>> {
  using type = constexpr_string<Chars1..., Chars2...>;
};

template <typename ... Strings> struct concat {
  using type = typename concat_impl<typename strip_null<Strings>::type...>::type;
};

//template <typename ... Strings> struct concat {
  //using str_type = typename concat_impl<Strings ...>::str_type;
//};

//template <char ... Chars> struct concat<const constexpr_string<Chars...>> {
  //using str_type = const constexpr_string<Chars...>;
//}; 

//template <char ... Chars> struct concat<constexpr_string<Chars...>> {
  //using str_type = constexpr_string<Chars...>;
//}; 
//template <> struct concat<> {
  //using str_type = const constexpr_string<>;
//};

template <unsigned long long Value> class str8_rep {
  const constexpr_string<
      static_cast<char>(0xff & Value),
      static_cast<char>((0xff00 & Value) >> 8llu),
      static_cast<char>((0xff0000 & Value) >> 16llu),
      static_cast<char>((0xff000000 & Value) >> 24llu),
      static_cast<char>((0xff00000000 & Value) >> 32llu),
      static_cast<char>((0xff0000000000 & Value) >> 40llu),
      static_cast<char>((0xff000000000000 & Value) >> 48llu),
      static_cast<char>((0xff00000000000000 & Value) >> 56llu)
  > str_impl_;

 public:
  using str_type = decltype(str_impl_);
  constexpr str8_rep() {}
  constexpr char const* str() const { return str_impl_.str(); }
  constexpr size_t size() const { return str_impl_.size(); }
  constexpr char operator[] (size_t index) const { return str_impl_[index]; }
};

template <unsigned long long ... Values> struct concat_str8 {
  using str_type = typename concat<typename str8_rep<Values>::str_type ...>::type;
};

unsigned long long constexpr compute_str8_value(const_string const& str, unsigned long long nb_remain) {
  return nb_remain ? ((static_cast<unsigned long long>(str[nb_remain-1]) << (8llu*(nb_remain-1llu))) + compute_str8_value(str, nb_remain - 1ll)) : 0llu;
}

unsigned long long constexpr str_to_str8_part(const_string const& value) {
  return compute_str8_value(value, static_cast<unsigned long long>(value.size()));
}

template <unsigned long long Id, unsigned long long ... Ids> struct str12;

//#endif  // NAMED_TUPLES_CPP14

}  // namespace named_tuples

#endif  // NAMED_TUPLES_CONST_STRING_HEADER