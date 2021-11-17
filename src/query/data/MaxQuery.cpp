#include "MaxQuery.h"
#include "../../db/Database.h"
#include "../../multithreads/MultiThread.hpp"

/**********************************************/
/*Define Global Varaibles*/
constexpr const char *MaxQuery::qname;
static unsigned int total_thread;
static unsigned int subtable_num;
static Table *copy_table;
static ComplexQuery *copy_this;
static std::vector<std::string> *copy_operand;
static std::pair<std::string, bool> result;
static int *int_arr;
static bool found;
static std::mutex m_mutex;
/**********************************************/

void Sub_max(int id) {
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  int *sub_int_arr = new int[copy_operand->size()];
  for (size_t i(0); i < copy_operand->size(); i++)
    sub_int_arr[i] = INT32_MIN;
  if (result.second) {
    for (auto row = head; row != tail; row++) {
      if (copy_this->evalCondition(*row)) {
        found = true;
        for (size_t i(0); i < copy_operand->size(); i++) {
          if (sub_int_arr[i] < (*row)[(*copy_operand)[i]]) {
            sub_int_arr[i] = (*row)[(*copy_operand)[i]];
          }
        }
      }
    }
  }
  m_mutex.lock();
  for (size_t i(0); i < copy_operand->size(); i++) {
    if (int_arr[i] < sub_int_arr[i]) {
      int_arr[i] = sub_int_arr[i];
    }
  }
  m_mutex.unlock();
}

QueryResult::Ptr MaxQuery::execute() {
  using namespace std;
  if (this->operands.size() == 0)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  // start of try
  try {
    found = false;
    auto &table = db[this->targetTable];
    result = initCondition(table);
    int_arr = new int[(this->operands).size()];
    total_thread = (unsigned int)worker.Thread_count();
    copy_operand = &this->operands;
    for (size_t i(0); i < this->operands.size(); i++)
      int_arr[i] = INT32_MIN;
    if (total_thread < 2 || table.size() < 2000) {
      if (result.second) {
        for (auto row = table.begin(); row != table.end(); ++row) {
          if (this->evalCondition(*row)) {
            found = true;
            for (size_t i(0); i < this->operands.size(); i++) {
              if (int_arr[i] < (*row)[this->operands[i]]) {
                int_arr[i] = (*row)[this->operands[i]];
              }
            }
          }
        }
      }
    } else {
      total_thread = (unsigned int)(table.size() / 2000 + 1);
      copy_table = &table;
      copy_this = this;
      subtable_num = (unsigned int)(table.size()) / total_thread;
      vector<future<void>> futures((unsigned long)total_thread);
      for (int i = 0; i < (int)total_thread; i++)
        futures[(unsigned long)i] = worker.Submit(Sub_max, i);
      for (unsigned long i = 0; i < (unsigned long)total_thread; i++)
        futures[i].get();
    }
    if (found)
      return make_unique<SuccessMsgResult>(int_arr, this->operands.size());
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
