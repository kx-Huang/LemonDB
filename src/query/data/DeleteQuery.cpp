//
// Created by MMM 21-10-20
//

#include "DeleteQuery.h"
#include "../../db/Database.h"
#include "../../multithreads/MultiThread.hpp"

/**********************************************/
/*Define Global Varaibles*/
constexpr const char *DeleteQuery::qname;
static size_t counter;

static unsigned int total_thread;
static unsigned int subtable_num;

static Table *copy_table;
static ComplexQuery *copy_this;
static std::vector<std::string> *copy_operand;
static std::pair<std::string, bool> result;
/**********************************************/
void Sub_Delete (int id){
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  if (result.second){
    for (auto item = head; item != tail; item++) {
        auto key = item->key();
        if (copy_this->evalCondition(*item)) {
          copy_table->deleteDatum(key);
          counter++, item--;
        }
    }
  }
  return;
}


QueryResult::Ptr DeleteQuery::execute() {
  using namespace std;
  if (this->operands.size() != 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  Table::SizeType counter(0); // number of affected rows;
  try {
    counter = 0;
    auto &table = db[this->targetTable];
    if (this->condition.empty())
      return make_unique<RecordCountResult>(counter);
    result = initCondition(table);
    total_thread = (unsigned int) worker.Thread_count();
    copy_operand = &this->operands;
    if (total_thread < 2 || table.size() < 16){
    if (result.second) {
      for (auto it = table.begin(); it != table.end(); it++) {
        auto key = it->key();
        if (this->evalCondition(*it)) {
          table.deleteDatum(key);
          counter++, it--;
        }
      }
    }
    }
    else{
      copy_table = &table;
      copy_this = this;
      subtable_num = (unsigned int)(table.size()) / total_thread;
      std::vector<std::future<void>> futures((unsigned long)total_thread);
      for(int i = 0; i<(int)total_thread - 1; i++){
        futures[(unsigned long)i] = worker.Submit(Sub_Delete, i);
      }
      Sub_Delete((int)total_thread - 1);
      for (unsigned long i = 0; i<(unsigned long)total_thread - 1;i++){
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

std::string DeleteQuery::toString() {
  return "QUERY = DELETE in" + this->targetTable + "\"";
}
