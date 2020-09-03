//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_DROPTABLEQUERY_H
#define PROJECT_DROPTABLEQUERY_H

#include "../Query.h"

class DropTableQuery : public Query {
  static constexpr const char *qname = "DROP";

public:
  using Query::Query;

  QueryResult::Ptr execute() override;

  std::string toString() override;
};

#endif // PROJECT_DROPTABLEQUERY_H
