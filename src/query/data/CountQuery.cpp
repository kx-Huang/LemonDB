#include "CountQuery.h"
#include "../../db/Database.h"

constexpr const char *CountQuery::qname;

QueryResult::Ptr CountQuery::execute() {
  using namespace std;
    if (this->operands.size() > 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  // start of try
  try {
    auto &table = db[this->targetTable];
    auto result = initCondition(table);
    int counter(0);
    if (result.second) {
      for (auto it = table.begin(); it != table.end(); ++it)
        if (this->evalCondition(*it))
          counter++;
    }
    return make_unique<SuccessMsgResult>(counter);
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

std::string CountQuery::toString() {
  return "QUERY = COUNT " + this->targetTable + "\"";
}
