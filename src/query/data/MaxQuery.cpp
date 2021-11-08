#include "MaxQuery.h"
#include "../../db/Database.h"

constexpr const char *MaxQuery::qname;

QueryResult::Ptr MaxQuery::execute() {
  using namespace std;
  auto size = this->operands.size();
  if (size == 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % size);
  Database &db = Database::getInstance();
  // start of try
  try {
    bool found = false;
    auto &table = db[this->targetTable];
    auto result = initCondition(table);
    int *int_arr = new int[size];
    for (size_t i = 0; i < size; i++)
      int_arr[i] = INT32_MIN;
    if (result.second) {
      for (auto row = table.begin(); row != table.end(); ++row) {
        if (this->evalCondition(*row)) {
          found = true;
          for (size_t i(0); i < size; i++) {
            if (int_arr[i] < (*row)[this->operands[i]]) {
              int_arr[i] = (*row)[this->operands[i]];
            }
          }
        }
      }
    }
    if (found)
      return make_unique<SuccessMsgResult>(int_arr, size);
    else
      return make_unique<NullQueryResult>();
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

std::string MaxQuery::toString() {
  return "QUERY = MAX " + this->targetTable + "\"";
}
