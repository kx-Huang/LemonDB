//
// Created by MMM 21-10-20
//

#include "DeleteQuery.h"
#include "../../db/Database.h"

constexpr const char *DeleteQuery::qname;

QueryResult::Ptr DeleteQuery::execute() {
  using namespace std;
  if (this->operands.size() != 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  Table::SizeType counter = 0; // number of affected rows;
  try {
    auto &table = db[this->targetTable];
    if (this->condition.empty()) {
      counter = table.clear();
      return make_unique<RecordCountResult>(counter);
    }
    auto result = initCondition(table);
    if (result.second) {
      size_t offset = 0;
      for (auto it = table.begin(); it != table.end(); ++it) {
        if (this->evalCondition(*it)) {
          auto temp = it--;
          table.deleteRow(temp, offset);
          counter++;
        }
        else offset++;
      }
    }
    return make_unique<RecordCountResult>(counter);
  } catch (const TableNameNotFound &e) {
      return make_unique<ErrorMsgResult>(qname, this->targetTable, "No such table."s);
  } catch (const IllFormedQueryCondition &e) {
      return make_unique<ErrorMsgResult>(qname, this->targetTable, e.what());
  } catch (const invalid_argument &e) {
      // Cannot convert operand to string
      return make_unique<ErrorMsgResult>(qname, this->targetTable, "Unknown error '?'"_f % e.what());
  } catch (const exception &e) {
      return make_unique<ErrorMsgResult>(qname, this->targetTable, "Unkonwn error '?'."_f % e.what());
  }
}

std::string DeleteQuery::toString() {
  return "QUERY = DELETE QUERY IN" + this->targetTable + "\"";
}
