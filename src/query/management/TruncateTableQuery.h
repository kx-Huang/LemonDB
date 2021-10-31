#ifndef PROJECT_TRUNCATEQUERY_H
#define PROJECT_TRUNCATEQUERY_H

#include "../Query.h"

class TruncateTableQuery : public Query {
  static constexpr const char *qname = "TRUNCATE";

public:
  explicit TruncateTableQuery(std::string table) : Query(std::move(table)) {}
  QueryResult::Ptr execute() override;
  std::string toString() override;
};

#endif // PROJECT_TRUNCATEQUERY_H
