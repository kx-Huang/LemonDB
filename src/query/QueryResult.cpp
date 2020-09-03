//
// Created by liu on 18-10-26.
//

#include "QueryResult.h"

std::ostream &operator<<(std::ostream &os, const QueryResult &table) {
  return table.output(os);
}
