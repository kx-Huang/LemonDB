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
/**********************************************/
void Sub_max(int id){
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  if (result.second) {
    for (auto row = head; row != tail; row++){
      if (copy_this->evalCondition(*row)) {
          found = true;
          for (size_t i(0); i < copy_operand->size(); i++) {
            if (int_arr[i] < (*row)[(*copy_operand)[i]]) {
              int_arr[i] = (*row)[(*copy_operand)[i]];
            }
          }
        }
    }
  }
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
    total_thread = (unsigned int) worker.Thread_count();
    for (size_t i(0); i < this->operands.size(); i++)
      int_arr[i] = INT32_MIN;
    if (total_thread < 2 || table.size() < 16){
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
    }
    else{
      copy_table = &table;
      copy_this = this;
      subtable_num = (unsigned int)(table.size()) / total_thread;
       std::vector<std::future<void>> futures((unsigned long)total_thread);
      for(int i = 0; i<(int)total_thread - 1; i++){
        futures[(unsigned long)i] = worker.Submit(Sub_max, i);
      }
      Sub_max((int)total_thread - 1);
      for (unsigned long i = 0; i<(unsigned long)total_thread - 1;i++){
        futures[i].get();
      }
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
