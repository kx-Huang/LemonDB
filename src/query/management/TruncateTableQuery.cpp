#include "TruncateTableQuery.h"
#include "../../db/Database.h"

constexpr const char *TruncateTableQuery::qname;

QueryResult::Ptr TruncateTableQuery::execute() {
  using namespace std;
  Database &db = Database::getInstance();
  try {
    auto table = &db[this->targetTable];
    table->clear();
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

std::string TruncateTableQuery::toString() {
  return "QUERY = TRUNCATE table \"" + this->targetTable + "\"";
}
