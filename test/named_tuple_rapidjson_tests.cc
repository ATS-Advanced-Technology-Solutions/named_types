#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <array>
#include <functional>
#include <memory>
#include <named_types/named_tuple.hpp>
#include <named_types/literals/integral_string_literal.hpp>
#include <named_types/rt_named_tuple.hpp>
#include <named_types/extensions/rapidjson.hpp>
#include "catch.hpp"

namespace {
size_t constexpr operator"" _s(const char* c, size_t s) {
  return named_types::basic_lowcase_charset_format::encode(c, s);
}
template <size_t EncStr>
using attr = named_types::named_tag<
    typename named_types::basic_lowcase_charset_format::decode<EncStr>::type>;

using name = attr<"name"_s>;
using age = attr<"age"_s>;
using size = attr<"size"_s>;
using children = attr<"children"_s>;
using child1 = attr<"child1"_s>;
using matrix = attr<"matrix"_s>;
using miles = attr<"miles"_s>;
using list = attr<"list"_s>;
using func = attr<"func"_s>;
};

// Testing the factory
TEST_CASE("RapidJson1", "[RapidJson1]") {
  // using namespace named_types::extensions::parsing;
  // using namespace named_types;
  using named_types::extensions::rapidjson::make_reader_handler;

  using Tuple = named_types::named_tuple<std::string(name),
                                         int(age),
                                         double(size),
                                         std::vector<int>(list),
                                         std::function<int(int)>(func)>;

  std::string input = R"json({"age":57,"name":"Marcelo","size":1.8})json";
  Tuple output{"Roger", 52, 1.9, {}, nullptr};
  // Tuple output { "Roger", 52, 1.9, {} };
  auto handler = make_reader_handler(output);

  ::rapidjson::Reader reader;
  ::rapidjson::StringStream ss(input.c_str());
  CHECK(reader.Parse(ss, handler));
  CHECK("Marcelo" == named_types::get<name>(output));
  CHECK(57 == named_types::get<age>(output));
}

TEST_CASE("RapidJson2", "[RapidJson2]") {
  // using namespace named_types::extensions::parsing;
  using namespace named_types;
  using named_types::extensions::rapidjson::make_reader_handler;

  using MyTuple = named_tuple<
      std::string(name),
      int(age),
      double(size),
      named_tuple<std::string(name), size_t(age)>(child1),
      std::vector<named_tuple<std::string(name), size_t(age)>>(children),
      std::vector<int>(miles),
      std::vector<std::vector<int>>(matrix)>;

  MyTuple t1;
  std::string input1 = R"json({"age":57,"name":"Marcelo","size":1.8,")json"
                       R"json(child1":{"name":"Coucou","age":3},"children")json"
                       R"json(:[{"name":"Albertine","age":4}],"miles":[1,)json"
                       R"json(2,3],"matrix":[[1,2],[3,4]]})json";
  auto handler = make_reader_handler(t1);
  ::rapidjson::Reader reader;
  ::rapidjson::StringStream ss(input1.c_str());
  CHECK(reader.Parse(ss, handler));
  CHECK(3 == t1.get<child1>().get<age>());
  CHECK("Albertine" == t1.get<children>()[0].get<name>());
  CHECK(4 == t1[children()][0][age()]);
  CHECK(4 == t1.get<matrix>()[1][1]);
}

TEST_CASE("RapidJson3", "[RapidJson3]") {
  using namespace named_types;
  using named_types::extensions::rapidjson::make_reader_handler;

  using MyTuple =
      named_tuple<std::string(name),
                  int(age),
                  double(size),
                  std::map<std::string, named_tuple<size_t(age)>>(children)>;

  MyTuple t1;
  std::string input1 = R"json({"age":57,"name":"Marcelo","size":1.8,")json"
                       R"json(children":{"Albertine":{"age":4},"Rupert":{")json"
                       R"json(age":5}}})json";
  auto handler = make_reader_handler(t1);
  ::rapidjson::Reader reader;
  ::rapidjson::StringStream ss(input1.c_str());
  CHECK(reader.Parse(ss, handler));
  CHECK(4 == t1.get<children>()["Albertine"].get<age>());
}

TEST_CASE("RapidJson4", "[RapidJson4]") {
  using namespace named_types;
  using named_types::extensions::rapidjson::make_reader_handler;

  using MyTuple = named_tuple<std::string(name), int(age)>;

  std::vector<MyTuple> data;
  std::string input1 =
      R"json([{"name":"Robert","age":48},{"age":57,"name":"Marcelo"}])json";
  auto handler = make_reader_handler(data);
  ::rapidjson::Reader reader;
  ::rapidjson::StringStream ss(input1.c_str());
  CHECK(reader.Parse(ss, handler));
  CHECK("Marcelo" == data[1].get<name>());
}
