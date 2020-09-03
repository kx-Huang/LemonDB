//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_PRINTTABLEQUERY_H
#define PROJECT_PRINTTABLEQUERY_H

#include "../Query.h"

class PrintTableQuery : public Query {
  static constexpr const char *qname = "SHOWTABLE";

public:
  using Query::Query;

  QueryResult::Ptr execute() override;

  std::string toString() override;
};

#endif // PROJECT_PRINTTABLEQUERY_H
