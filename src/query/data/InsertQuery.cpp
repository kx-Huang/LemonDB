//
// Created by liu on 18-10-25.
//

#include "InsertQuery.h"

#include <algorithm>

#include "../../db/Database.h"
#include "../QueryResult.h"

constexpr const char *InsertQuery::qname;

QueryResult::Ptr InsertQuery::execute() {
  using namespace std;
  if (this->operands.empty())
    return make_unique<ErrorMsgResult>(qname, this->targetTable.c_str(),
                                       "No operand (? operands)."_f %
                                           operands.size());
  Database &db = Database::getInstance();
  try {
    auto &table = db[this->targetTable];
    auto &key = this->operands.front();
    vector<Table::ValueType> data;
    data.reserve(this->operands.size() - 1);
    for (auto it = ++this->operands.begin(); it != this->operands.end(); ++it) {
      data.emplace_back(strtol(it->c_str(), nullptr, 10));
    }
    table.insertByIndex(key, move(data));
    return std::make_unique<SuccessMsgResult>(qname, targetTable);
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

std::string InsertQuery::toString() {
  return "QUERY = INSERT " + this->targetTable + "\"";
}
