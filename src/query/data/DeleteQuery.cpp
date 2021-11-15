#include "DeleteQuery.h"
#include "../../db/Database.h"
#include "../../multithreads/MultiThread.hpp"

QueryResult::Ptr DeleteQuery::execute() {
  using namespace std;
  if (this->operands.size() != 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  try {
    size_t counter = 0;
    auto &table = db[this->targetTable];
    auto result = initCondition(table);
    if (result.second) {
      if (this->condition.empty()) {
        counter = table.size();
        table.clear();
      } else {
        for (auto it = table.begin(); it != table.end(); it++) {
          if (this->evalCondition(*it)) {
            table.deleteDatum(it->key());
            counter++, it--;
          }
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
