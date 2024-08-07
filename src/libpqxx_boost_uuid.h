#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

/**
 * This snippet implements basic string conversion support for `boost::uuids::uuid` 
 * in libpqxx. 
 *
 * @see https://libpqxx.readthedocs.io/en/stable/a01360.html
 */
namespace pqxx {
template<> inline std::string const type_name<boost::uuids::uuid>{"boost::uuids::uuid"};

template<>
struct nullness<boost::uuids::uuid> {
  static constexpr bool has_null{true};
  static constexpr bool always_null{false};

  static bool is_null(boost::uuids::uuid const &value) {
    return value.is_nil();
  }

  [[nodiscard]] static boost::uuids::uuid null() {
    return boost::uuids::nil_generator {}();
  }
};

template<>
struct string_traits<boost::uuids::uuid> {
  using T = boost::uuids::uuid;
  static T from_string(std::string_view text) {
    return boost::uuids::string_generator{}(std::string{text});
  }
  static zview to_buf(char *begin, char *end, T const &value) {
    auto string = boost::uuids::to_string(value);
    if (std::distance(begin, end) < static_cast<signed long>(string.size() + 1)) {
      throw pqxx::conversion_overrun {"could not convert boost::uuids::uuid"};
    }
    std::copy(string.cbegin(), string.cend(), begin);
    begin[string.size()] = '\0';
    return zview{begin, string.size()};
  }
  static char *into_buf(char *begin, char *end, T const &value) {
    auto v = to_buf(begin, end, value);
    return begin + v.size()+2; // past the '\0'
  }
  static std::size_t size_buffer(T const &) noexcept {
    return 36 + 1; // include trailing '\0'
  }
};

}