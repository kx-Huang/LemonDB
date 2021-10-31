#include "CopyTableQuery.h"
#include "../../db/Database.h"

constexpr const char *CopyTableQuery::qname;

QueryResult::Ptr CopyTableQuery::execute() {
  using namespace std;
  Database &db = Database::getInstance();
  try {
    auto old_table = db[this->targetTable];
    db.registerTable(std::make_unique<Table>(new_table, old_table));
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

std::string CopyTableQuery::toString() {
  return "QUERY = COPYTABLE from \"" + this->targetTable + "\" to \"" +
         this->new_table + "\"";
}
