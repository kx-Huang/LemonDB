#ifndef SRC_FORMATTER_H
#define SRC_FORMATTER_H

#include <string>
#include <vector>

template <typename T> static inline std::string to_string(std::vector<T> &vec) {
  std::string str;
  for (const auto &val : vec) {
    str += std::to_string(val) + " ";
  }
  return str;
}

template <typename T> static inline std::string to_string(T t) {
  return std::to_string(t);
}

template <typename T> inline std::string operator%(std::string format, T t) {
  auto ind = format.find('?');
  if (ind == 0 || format[ind - 1] != '\\') {
    format.replace(ind, 1u, to_string(t));
  }
  return format;
}

template <> inline std::string operator%(std::string format, std::string s) {
  auto ind = format.find('?');
  if (ind == 0 || format[ind - 1] != '\\') {
    format.replace(ind, 1u, s);
  }
  return format;
}

template <> inline std::string operator%(std::string format, const char *s) {
  auto ind = format.find('?');
  if (ind == 0 || format[ind - 1] != '\\') {
    format.replace(ind, 1u, s);
  }
  return format;
}

inline std::string operator""_f(const char *str, size_t size) {
  return std::string(str);
}

#endif // SRC_FORMATTER_H
