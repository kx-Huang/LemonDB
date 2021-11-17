#include "SubQuery.h"
#include <iostream>

#include "../../db/Database.h"
#include "../../multithreads/MultiThread.hpp"

/**********************************************/
/*Define Global Varaibles*/
static unsigned int total_thread;
static unsigned int subtable_num;

static Table *copy_table;
static std::pair<std::string, bool> result;
static ComplexQuery *copy_this;
static std::vector<std::string> *copy_operand;
constexpr const char *SubQuery::qname;
/**********************************************/

int Sub_SubQuery(int id) {
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  int sub_counter = 0;
  if (result.second) {
    for (auto it = head; it != tail; it++) {
      if (copy_this->evalCondition(*it)) {
        sub_counter++;
        int ans = (*it)[*(copy_operand->begin())];
        for (auto key = copy_operand->begin() + 1;
             key != copy_operand->end() - 1; key++) {
          ans -= (*it)[*key];
        }
        (*it)[*(copy_operand->end() - 1)] = ans;
      }
    }
  }
  return sub_counter;
}

QueryResult::Ptr SubQuery::execute() {
  using namespace std;
  if (this->operands.size() <= 1)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  // start of try
  try {
    auto &table = db[this->targetTable];
    result = initCondition(table);
    total_thread = (unsigned int)worker.Thread_count();
    int counter = 0;
    if (total_thread < 2 || table.size() < 2000) {
      if (result.second) {
        for (auto it = table.begin(); it != table.end(); ++it) {
          if (this->evalCondition(*it)) {
            auto key = this->operands.begin();
            int total = (*it)[*key++];
            while (key != this->operands.end() - 1)
              total -= (*it)[*key++];
            (*it)[*(this->operands.end() - 1)] = total;
            counter++;
          }
        }
      }
    } else {
      total_thread = (unsigned int)(table.size() / 2000 + 1);
      copy_table = &table;
      copy_this = this;
      copy_operand = &this->operands;
      subtable_num = (unsigned int)(table.size()) / total_thread;
      vector<future<int>> futures((unsigned long)total_thread);
      for (int i = 0; i < (int)total_thread; i++)
        futures[(unsigned long)i] = worker.Submit(Sub_SubQuery, i);
      for (int i = 0; i < (int)total_thread; i++)
        counter = counter + futures[(unsigned long)i].get();
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

std::string SubQuery::toString() {
  return "QUERY = SUB " + this->targetTable + "\"";
}
