#ifndef SRC_QUERY_PARSER_H
#define SRC_QUERY_PARSER_H

#include <memory>
#include <string>
#include <vector>

#include "Query.h"

struct TokenizedQueryString {
  std::vector<std::string> token;
  std::string rawQeuryString;
};

class QueryBuilder {
public:
  typedef std::unique_ptr<QueryBuilder> Ptr;

  virtual Query::Ptr tryExtractQuery(TokenizedQueryString &queryString) = 0;
  virtual void setNext(Ptr &&builder) = 0;
  virtual void clear() = 0;

  virtual ~QueryBuilder() = default;
};

class QueryParser {
  QueryBuilder::Ptr first; // An owning pointer
  QueryBuilder *last;      // None owning reference

  TokenizedQueryString tokenizeQueryString(std::string queryString);

public:
  Query::Ptr parseQuery(std::string queryString);
  void registerQueryBuilder(QueryBuilder::Ptr &&qBuilder);

  QueryParser();
  ~QueryParser() = default;
};

#endif // SRC_QUERY_PARSER_H
