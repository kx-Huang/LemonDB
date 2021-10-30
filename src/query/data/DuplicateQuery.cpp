#include "DuplicateQuery.h"
#include "../../db/Database.h"
#include "../../db/Table.h"

QueryResult::Ptr DuplicateQuery::execute() {
  using namespace std;
  if (this->operands.size() != 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  // start of try
  try {
    auto table = &db[this->targetTable];
    auto result = initCondition(*table);
    vector<Table::KeyType> keys;
    Table::SizeType counter(0);
    if (result.second) {
      auto end = table->end();
      for (auto it = table->begin(); it != end; ++it) {
        if (this->evalCondition(*it)) {
          keys.push_back((*it).key());
          ++counter;
        }
      }
    }
    if (counter > 0) {
      for (auto it = keys.begin(); it != keys.end(); it++)
        table->duplicateByIndex((*it));
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

std::string DuplicateQuery::toString() {
  return "QUERY = Duplicate " + this->targetTable + "\"";
}
