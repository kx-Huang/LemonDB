#ifndef PROJECT_MAXQUERY_H
#define PROJECT_MAXQUERY_H

#include "../Query.h"

class MaxQuery : public ComplexQuery {
  static constexpr const char *qname = "MAX";
public:
  using ComplexQuery::ComplexQuery;
  QueryResult::Ptr execute() override;
  std::string toString() override;
};

#endif // PROJECT_MAXQUERY_H
