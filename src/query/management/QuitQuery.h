//
// Created by liu on 18-10-25.
//

#ifndef PROJECT_QUITQUERY_H
#define PROJECT_QUITQUERY_H

#include "../Query.h"

class QuitQuery : public Query {
  static constexpr const char *qname = "QUIT";

public:
  QuitQuery() = default;

  QueryResult::Ptr execute() override;

  std::string toString() override;
};

#endif // PROJECT_QUITQUERY_H
