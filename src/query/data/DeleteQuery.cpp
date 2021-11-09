//
// Created by MMM 21-10-20
//

#include "DeleteQuery.h"
#include "../../db/Database.h"
#include <iostream>
using namespace std;

constexpr const char *DeleteQuery::qname;

QueryResult::Ptr DeleteQuery::execute() {
  using namespace std;
  if (this->operands.size() != 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  // start of try
  try {
    size_t counter = 0;
    auto &table = db[this->targetTable];
    if (this->condition.empty())
      return make_unique<RecordCountResult>(counter);
    auto result = initCondition(table);
    if (result.second) {
      for (auto it = table.begin(); it != table.end(); it++) {
        auto key = it->key();
        if (this->evalCondition(*it)) {
          table.deleteDatum(key);
          counter++, it--;
        }
      }
    }
    return make_unique<RecordCountResult>(counter);
  } catch (const TableNameNotFound &e) {
    return make_unique<ErrorMsgResult>(qname, this->targetTable,
                                       "No such table."s);
  } catch (const IllFormedQueryCondition &e) {
    return make_unique<ErrorMsgResult>(qname, this->targetTable, e.what());
  } catch (const invalid_argument &e) {
    // Cannot convert operand to string
    return make_unique<ErrorMsgResult>(qname, this->targetTable,
                                       "Unknown error '?'"_f % e.what());
  } catch (const exception &e) {
    return make_unique<ErrorMsgResult>(qname, this->targetTable,
                                       "Unkonwn error '?'."_f % e.what());
  }
}

std::string DeleteQuery::toString() {
  return "QUERY = DELETE in" + this->targetTable + "\"";
}
