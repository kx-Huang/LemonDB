#ifndef SRC_QEURY_EXCEPTION_H
#define SRC_QEURY_EXCEPTION_H

#include <exception>
#include <stdexcept>
#include <string>

#include "formatter.h"

struct UnableToOpenFile : public std::runtime_error {
  explicit UnableToOpenFile(const std::string &file)
      : std::runtime_error("Unable to open \"" + file + "\"") {}
};

struct DuplicatedTableName : public std::invalid_argument {
  explicit DuplicatedTableName(const std::string &str)
      : std::invalid_argument(str) {}
};

struct TableNameNotFound : public std::invalid_argument {
  explicit TableNameNotFound(const std::string &str)
      : std::invalid_argument(str) {}
};

struct ConflictingKey : public std::invalid_argument {
  explicit ConflictingKey(const std::string &str)
      : std::invalid_argument(str) {}
};

struct MultipleKey : public std::invalid_argument {
  explicit MultipleKey(const std::string &str) : std::invalid_argument(str) {}
};

struct TableFieldNotFound : public std::out_of_range {
  explicit TableFieldNotFound(const std::string &str)
      : std::out_of_range(str) {}
};

struct LoadFromStreamException : public std::runtime_error {
  explicit LoadFromStreamException(const std::string &str)
      : std::runtime_error(str) {}
};

struct IllFormedQuery : public std::invalid_argument {
  explicit IllFormedQuery(const std::string &str)
      : std::invalid_argument(str) {}
};

struct IllFormedQueryCondition : public std::invalid_argument {
  explicit IllFormedQueryCondition(const std::string &str)
      : std::invalid_argument(str) {}
};

class QueryBuilderMatchFailed : public std::invalid_argument {
public:
  explicit QueryBuilderMatchFailed(const std::string &qString)
      : std::invalid_argument(R"(Failed to parse query string: "?")"_f %
                              qString) {}
};

#endif // SRC_QEURY_EXCEPTION_H
