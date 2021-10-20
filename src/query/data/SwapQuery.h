//
// Created by MMM on 21-10-20
//

#ifndef PROJECT_SWAPQUERY_H
#define PROJECT_SWAPQUERY_H
#include "../Query.h"

class SwapQuery : public ComplexQuery {
  static constexpr const char *qname = "SWAP";
  Table::FieldIndex fid1; // fieldId 1 to be swapped
  Table::FieldIndex fid2; // fieldId 2 to be swapped
public:
  using ComplexQuery::ComplexQuery;
  QueryResult::Ptr execute() override;
  std::string toString() override;
};

#endif //PROJECT_SWAP_QUERY_H
