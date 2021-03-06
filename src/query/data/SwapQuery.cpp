#include "SwapQuery.h"
#include "../../db/Database.h"
#include "../../multithreads/MultiThread.hpp"

/**********************************************/
/*Define Global Varaibles*/
constexpr const char *SwapQuery::qname;
static unsigned int total_thread;
static unsigned int subtable_num;

static Table *copy_table;
static ComplexQuery *copy_this;
static std::pair<std::string, bool> result;
/**********************************************/

int Sub_Swap(int id, size_t fid1, size_t fid2) {
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  int sub_counter = 0;
  if (result.second) {
    for (auto it = head; it != tail; it++) {
      if (copy_this->evalCondition(*it)) {
        Table::ValueType temp = (*it)[fid2];
        (*it)[fid2] = (*it)[fid1];
        (*it)[fid1] = temp;
        sub_counter++;
      }
    }
  }
  return sub_counter;
}

QueryResult::Ptr SwapQuery::execute() {
  using namespace std;
  if (this->operands.size() != 2)
    return make_unique<ErrorMsgResult>(
        qname, this->targetTable.c_str(),
        "Invalid number of operands (? operands)."_f % operands.size());
  Database &db = Database::getInstance();
  try {
    auto &table = db[this->targetTable];
    result = initCondition(table);
    int counter = 0;
    total_thread = (unsigned int)worker.Thread_count();
    fid1 = table.getFieldIndex(this->operands[0]);
    fid2 = table.getFieldIndex(this->operands[1]);
    auto result = initCondition(table);
    if (total_thread < 2 || table.size() < 2000) {
      if (result.second) {
        for (auto it = table.begin(); it != table.end(); ++it) {
          if (this->evalCondition(*it)) {
            Table::ValueType temp = (*it)[fid2];
            (*it)[fid2] = (*it)[fid1];
            (*it)[fid1] = temp;
            ++counter;
          }
        }
      }
    } else {
      total_thread = (unsigned int)(table.size() / 2000 + 1);
      copy_table = &table;
      copy_this = this;
      subtable_num = (unsigned int)(table.size()) / total_thread;
      vector<future<int>> futures((unsigned long)total_thread);
      for (int i = 0; i < (int)total_thread; i++)
        futures[(unsigned long)i] = worker.Submit(Sub_Swap, i, fid1, fid2);
      for (unsigned long i = 0; i < (unsigned long)total_thread; i++)
        counter = counter + futures[i].get();
    }
    return make_unique<RecordCountResult>((size_t)counter);
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

std::string SwapQuery::toString() {
  return "Query = SWAP " + this->targetTable + "\"";
}
