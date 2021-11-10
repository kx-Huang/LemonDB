#include "DuplicateQuery.h"
#include "../../db/Database.h"
#include "../../db/Table.h"
#include "../../multithreads/MultiThread.hpp"
/**********************************************/
/*Define Global Varaibles*/
static size_t counter;
static unsigned int total_thread;
static unsigned int subtable_num;

static Table *copy_table;
static ComplexQuery *copy_this;
static std::pair<std::string, bool> result;
/**********************************************/

void Sub_Duplicate(int id) {
  using namespace std;
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  if (result.second) {
    vector<Table::KeyType> keys;
    for (auto it = head; it != tail; it++) {
      auto key = it->key();
      if (copy_this->evalCondition(*it) && copy_table->evalDuplicate(key)) {
        keys.push_back(key);
        counter++;
      }
    }
    for (auto it = keys.begin(); it != keys.end(); it++)
      copy_table->duplicateByKey(*it);
  }
  return;
}

QueryResult::Ptr DuplicateQuery::execute() {
  using namespace std;
  if (this->operands.size() != 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  // start of try
  try {
    auto &table = db[this->targetTable];
    auto result = initCondition(table);
    total_thread = (unsigned int)worker.Thread_count();
    counter = 0;
    if (total_thread < 2 || table.size() < 16) {
      if (result.second) {
        auto end = table.end();
        vector<Table::KeyType> keys;
        for (auto it = table.begin(); it != end; it++) {
          auto key = it->key();
          if (this->evalCondition(*it) && table.evalDuplicate(key)) {
            keys.push_back(key);
            counter++;
          }
        }
        for (auto it = keys.begin(); it != keys.end(); it++)
          table.duplicateByKey(*it);
      }
    } else {
      copy_table = &table;
      copy_this = this;
      subtable_num = (unsigned int)(table.size()) / total_thread;
      std::vector<std::future<void>> futures((unsigned long)total_thread);
      for (int i = 0; i < (int)total_thread - 1; i++) {
        futures[(unsigned long)i] = worker.Submit(Sub_Duplicate, i);
      }
      Sub_Duplicate((int)total_thread - 1);
      for (unsigned long i = 0; i < (unsigned long)total_thread - 1; i++) {
        futures[i].get();
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

std::string DuplicateQuery::toString() {
  return "QUERY = DUPLICATE " + this->targetTable + "\"";
}
