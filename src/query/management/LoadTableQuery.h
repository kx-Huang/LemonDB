//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_LOADTABLEQUERY_H
#define PROJECT_LOADTABLEQUERY_H

#include "../Query.h"

class LoadTableQuery : public Query {
  static constexpr const char *qname = "LOAD";
  const std::string fileName;

public:
  explicit LoadTableQuery(std::string table, std::string fileName)
      : Query(std::move(table)), fileName(std::move(fileName)) {}

  QueryResult::Ptr execute() override;

  std::string toString() override;
};

#endif // PROJECT_LOADTABLEQUERY_H
