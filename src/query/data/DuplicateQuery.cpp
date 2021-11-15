#include "DuplicateQuery.h"
#include "../../db/Database.h"
#include "../../multithreads/MultiThread.hpp"

/**********************************************/
/*Define Global Varaibles*/
static size_t counter;
static unsigned int total_thread;
static unsigned int subtable_num;
static Table *copy_table;
static ComplexQuery *copy_this;
static std::pair<std::string, bool> result;
static std::vector<Table::KeyType> keys;
struct Ret_Dup {
  size_t subcounter;
  std::vector<Table::KeyType> sub_keys;
};
/**********************************************/

Ret_Dup Sub_Duplicate(int id) {
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  Ret_Dup temp;
  if (result.second) {
    std::vector<Table::KeyType> sub_keys;
    size_t sub_counter = 0;
    for (auto it = head; it != tail; it++) {
      auto key = it->key();
      if (copy_this->evalCondition(*it) && copy_table->evalDuplicate(key)) {
        sub_keys.push_back(key);
        sub_counter++;
      }
    }
    temp.sub_keys = sub_keys;
    temp.subcounter = sub_counter;
  }
  return temp;
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
    result = initCondition(table);
    total_thread = (unsigned int)worker.Thread_count();
    counter = 0;
    keys.clear();
    if (total_thread < 2 || table.size() < 16) {
      if (result.second) {
        auto end = table.end();
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
      vector<future<Ret_Dup>> futures((unsigned long)total_thread);
      for (int i = 0; i < (int)total_thread - 1; i++) {
        futures[(unsigned long)i] = worker.Submit(Sub_Duplicate, i);
      }
      Ret_Dup temp = Sub_Duplicate((int)total_thread - 1);
      counter = temp.subcounter;
      keys.insert(keys.end(), temp.sub_keys.begin(), temp.sub_keys.end());
      for (unsigned long i = 0; i < (unsigned long)total_thread - 1; i++) {
        temp = futures[i].get();
        keys.insert(keys.end(), temp.sub_keys.begin(), temp.sub_keys.end());
        counter = counter + temp.subcounter;
      }
      for (auto it = keys.begin(); it != keys.end(); it++)
        table.duplicateByKey(*it);
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
