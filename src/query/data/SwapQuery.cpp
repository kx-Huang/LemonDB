//
// Created by MMM on 21-10-20
//

#include "SwapQuery.h"
#include "../../db/Database.h"

constexpr const char *SwapQuery::qname;

QueryResult::Ptr SwapQuery::execute() {
  using namespace std;
  if (this->operands.size() != 2)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  Table::SizeType counter = 0;
  try {
    auto &table = db[this->targetTable];
    fid1 = table.getFieldIndex(this->operands[0]);
    fid2 = table.getFieldIndex(this->operands[1]);
    auto result = initCondition(table);
    if (result.second) {
      for (auto it = table.begin(); it != table.end(); ++it) {
        if (this->evalCondition(*it)) {
          Table::ValueType temp = (*it)[fid2];
          (*it)[fid2] = (*it)[fid1];
          (*it)[fid1] = temp;
          ++counter;
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

std::string SwapQuery::toString() {
  return "Query = SWAP " + this->targetTable + "\"";
}
