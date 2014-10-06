#ifndef NAMED_TUPLES_INTROSPECTION_HEADER
#define NAMED_TUPLES_INTROSPECTION_HEADER
#include <cstddef>
#include <array>
#include <string>
#include <typeinfo>
#include "tuple.hpp"
#include "constexpr_string.hpp"

namespace named_tuples {

template<typename T> struct str8_name;
template<llu ... Ids> struct str8_name<id_value<Ids...>> {
  static const typename concat_str8<Ids...>::type value;
};

template<llu ... Ids> const typename concat_str8<Ids...>::type str8_name<id_value<Ids...>>::value;

// Shared static data
template <typename ... T> class runtime_tuple_str8_data;
template <typename ... Ids, typename ... Types> struct runtime_tuple_str8_data< named_tuple<Types(Ids)...> > {  
  runtime_tuple_str8_data() = delete;
  static const std::array<std::string const, sizeof ... (Ids)> attributes;
  static const std::array<std::type_info const &, sizeof ... (Types)> type_infos;
  static const std::array<std::type_info const &, sizeof ... (Ids)> id_type_infos;
};

template <typename ... Ids, typename ... Types>
const std::array<std::string const, sizeof ... (Ids)> 
runtime_tuple_str8_data<named_tuple<Types(Ids)...>>::attributes = {{ std::string(str8_name<Ids>::value.str()) ...  }};

template <typename ... Ids, typename ... Types>
const std::array<std::type_info const &, sizeof ... (Types)> 
runtime_tuple_str8_data<named_tuple<Types(Ids)...>>::type_infos = {{ typeid(Types) ...  }};

template <typename ... Ids, typename ... Types>
const std::array<std::type_info const &, sizeof ... (Ids)> 
runtime_tuple_str8_data<named_tuple<Types(Ids)...>>::id_type_infos = {{ typeid(Ids) ...  }};

// Const version
template <typename ... T> class const_runtime_tuple_str8;
template <typename ... Ids, typename ... Types> class const_runtime_tuple_str8< named_tuple<Types(Ids)...> const > {
  using this_tuple_type =  named_tuple<Types(Ids)...> const;
  this_tuple_type& tuple_;
  std::array<void const *, sizeof ... (Ids)> pointers_;

  // Type is matching
  template <typename Target, size_t CurrentIndex, typename IdHead, typename ... IdTail> 
  auto runtime_get(size_t index) const -> 
  typename std::enable_if<(CurrentIndex < sizeof ... (Ids) && std::is_same<typename std::remove_reference<decltype(tuple_.template _<IdHead>())>::type, Target>::value), Target const&>::type 
  {
    if (CurrentIndex == index)
      return tuple_.template _<IdHead>();
    else
      return this->template runtime_get<Target, CurrentIndex+1, IdTail ...>(index);
  }

  // Type is not matching
  template <typename Target, size_t CurrentIndex, typename IdHead, typename ... IdTail> 
  auto runtime_get(size_t index) const -> 
  typename std::enable_if<(CurrentIndex < sizeof ... (Ids) && !std::is_same<typename std::remove_reference<decltype(tuple_.template _<IdHead>())>::type, Target>::value), Target const&>::type 
  {
    if (CurrentIndex == index)
      throw std::out_of_range("Attribute not found");
    else
      return this->template runtime_get<Target, CurrentIndex+1, IdTail ...>(index);
  }

  template <typename Target, size_t CurrentIndex, typename ... IdsSink> 
  auto runtime_get(size_t index) const -> 
  typename std::enable_if<(sizeof ... (Ids) <= CurrentIndex), Target const&>::type 
  {
    throw std::out_of_range("Attribute not found");
  }

 protected:
  size_t index_of_attr(std::string const& name) {
    size_t index = 0;
    for(;index < (sizeof ... (Ids)); ++index) {
      if (name == attributes[index]) break;
    }
    return index;
  };

 public:
  static const std::array<std::string const, sizeof ... (Ids)>& attributes;
  static const std::array<std::type_info const &, sizeof ... (Types)>& type_infos;
  static const std::array<std::type_info const &, sizeof ... (Ids)>& id_type_infos;

  const_runtime_tuple_str8(this_tuple_type& Value) noexcept : tuple_{Value}, pointers_{ &(Value.template _<Ids>()) ... } {}

  std::type_info const& type_of(size_t index) { return index < sizeof ... (Types) ? type_infos[index] : typeid(nullptr); } 
  std::type_info const& type_of(std::string const& name) { return type_of(index_of_attr(name)); } 

  std::type_info const& id_type_of(size_t index) { return index < sizeof ... (Ids) ? id_type_infos[index] : typeid(nullptr); } 
  std::type_info const& id_type_of(std::string const& name) { return id_type_of(index_of_attr(name)); } 

  template <typename T> T const * get_ptr(size_t index) const {
    static std::array<bool, sizeof ... (Ids)> attr_is_same {{ std::is_same<Types, T>::value ... }};
    if ((sizeof ... (Ids)) <= index) 
      return nullptr;
    
    if (attr_is_same.at(index))
      return reinterpret_cast<T const *>(pointers_[index]);

    return nullptr;
  }

  template <typename T> T const * get_ptr(std::string const& name) const {
    return get_ptr<T>(index_of_attr(name));
  }

  template <typename T> T const & get(size_t index) const {
    static std::array<bool, sizeof ... (Ids)> attr_is_same {{ std::is_same<Types, T>::value ... }};
    if ((sizeof ... (Ids)) <= index) 
      throw std::out_of_range("");
    
    if (attr_is_same.at(index))
      return runtime_get<T, 0, Ids ...>(index);

    throw std::out_of_range("");
  }

  template <typename T> T const & get(std::string const& name) const {
    return get<T>(index_of_attr(name));
  }
};

template <typename ... Ids, typename ... Types>
const std::array<std::string const, sizeof ... (Ids)>& 
const_runtime_tuple_str8<named_tuple<Types(Ids)...> const>::attributes = runtime_tuple_str8_data<named_tuple<Types(Ids)...>>::attributes;

template <typename ... Ids, typename ... Types>
const std::array<std::type_info const&, sizeof ... (Types)>& 
const_runtime_tuple_str8<named_tuple<Types(Ids)...> const>::type_infos = runtime_tuple_str8_data<named_tuple<Types(Ids)...>>::type_infos;

template <typename ... Ids, typename ... Types>
const std::array<std::type_info const&, sizeof ... (Ids)>& 
const_runtime_tuple_str8<named_tuple<Types(Ids)...> const>::id_type_infos = runtime_tuple_str8_data<named_tuple<Types(Ids)...>>::id_type_infos;

// Mutable version
template <typename ... T> class runtime_tuple_str8;
template <typename ... Ids, typename ... Types> class runtime_tuple_str8< named_tuple<Types(Ids)...> > : public const_runtime_tuple_str8<named_tuple<Types(Ids)...> const> {
  using this_tuple_type =  named_tuple<Types(Ids)...>;
  this_tuple_type& tuple_;
  std::array<void*, sizeof ... (Ids)> pointers_;

  // Type is matching
  template <typename Target, size_t CurrentIndex, typename IdHead, typename ... IdTail> 
  auto runtime_get(size_t index) -> 
  typename std::enable_if<(CurrentIndex < sizeof ... (Ids) && std::is_same<typename std::remove_reference<decltype(tuple_.template _<IdHead>())>::type, Target>::value), Target&>::type 
  {
    if (CurrentIndex == index)
      return tuple_.template _<IdHead>();
    else
      return this->template runtime_get<Target, CurrentIndex+1, IdTail ...>(index);
  }

  // Type is not matching
  template <typename Target, size_t CurrentIndex, typename IdHead, typename ... IdTail> 
  auto runtime_get(size_t index) -> 
  typename std::enable_if<(CurrentIndex < sizeof ... (Ids) && !std::is_same<typename std::remove_reference<decltype(tuple_.template _<IdHead>())>::type, Target>::value), Target&>::type 
  {
    if (CurrentIndex == index)
      throw std::out_of_range("Attribute not found");
    else
      return this->template runtime_get<Target, CurrentIndex+1, IdTail ...>(index);
  }

  template <typename Target, size_t CurrentIndex, typename ... IdsSink> 
  auto runtime_get(size_t index) -> 
  typename std::enable_if<(sizeof ... (Ids) <= CurrentIndex), Target&>::type 
  {
    throw std::out_of_range("Attribute not found");
  }

 public:
  runtime_tuple_str8(this_tuple_type& Value) noexcept : const_runtime_tuple_str8<named_tuple<Types(Ids)...> const>(Value), tuple_{Value}, pointers_{ &(Value.template _<Ids>()) ... } {}

  template <typename T> T* get_ptr(size_t index) {
    static std::array<bool, sizeof ... (Ids)> attr_is_same {{ std::is_same<Types, T>::value ... }};
    if ((sizeof ... (Ids)) <= index) 
      return nullptr;
    
    if (attr_is_same.at(index))
      return reinterpret_cast<T*>(pointers_[index]);

    return nullptr;
  }

  template <typename T> T* get_ptr(std::string const& name) {
    return get_ptr<T>(this->index_of_attr(name));
  }

  template <typename T> T& get(size_t index) {
    static std::array<bool, sizeof ... (Ids)> attr_is_same {{ std::is_same<Types, T>::value ... }};
    if ((sizeof ... (Ids)) <= index) 
      throw std::out_of_range("");
    
    if (attr_is_same.at(index))
      return runtime_get<T, 0, Ids ...>(index);

    throw std::out_of_range("");
  }

  template <typename T> T& get(std::string const& name) {
    return get<T>(this->index_of_attr(name));
  }
};


}  // namespace named_tuples

#endif  // NAMED_TUPLES_CONST_STRING_HEADER
