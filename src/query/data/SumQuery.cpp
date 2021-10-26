#include "SumQuery.h"
#include "../../db/Database.h"

constexpr const char *SumQuery::qname;

QueryResult::Ptr SumQuery::execute() {
  using namespace std;
  Database &db = Database::getInstance();
  if (this->operands.size() <= 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  // start of try
  try {
    auto &table = db[this->targetTable];
    auto result = initCondition(table);
    int *int_arr = new int[(this->operands).size()];
    for (size_t i (0); i < this->operands.size(); i++) int_arr[i] = 0;
    if (result.second) {
      for (auto row = table.begin(); row != table.end(); ++row) {
        if (this->evalCondition(*row)) {
          for (size_t i (0); i < this->operands.size(); i++) {
            int_arr[i] += (*row)[this->operands[i]];
          }
        }
      }
    }
    return make_unique<SuccessMsgResult>(int_arr, this->operands.size());
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

std::string SumQuery::toString() {
  return "QUERY = Sum " + this->targetTable + "\"";
}
