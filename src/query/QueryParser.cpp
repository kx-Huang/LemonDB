#include "QueryParser.h"

#include <sstream>

#include "Query.h"
#include "QueryBuilders.h"

QueryParser::QueryParser() : first(nullptr), last(nullptr) {}

Query::Ptr QueryParser::parseQuery(std::string queryString) {
  if (first == nullptr)
    throw QueryBuilderMatchFailed(queryString);
  auto t = tokenizeQueryString(queryString);
  if (t.token.empty())
    throw QueryBuilderMatchFailed("");
  first->clear();
  return first->tryExtractQuery(t);
}

void QueryParser::registerQueryBuilder(QueryBuilder::Ptr &&qBuilder) {
  if (first == nullptr) {
    first = std::move(qBuilder);
    last = first.get();
    last->setNext(FailedQueryBuilder::getDefault());
  } else {
    QueryBuilder *temp = last;
    last = qBuilder.get();
    temp->setNext(std::move(qBuilder));
    last->setNext(FailedQueryBuilder::getDefault());
  }
}

TokenizedQueryString QueryParser::tokenizeQueryString(std::string queryString) {
  TokenizedQueryString t;
  t.rawQeuryString = queryString;
  std::stringstream s;
  s.str(queryString);
  std::string tStr;
  while (s >> tStr)
    t.token.push_back(tStr);
  return t;
}
