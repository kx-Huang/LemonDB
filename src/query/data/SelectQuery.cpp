#include "SelectQuery.h"

#include "../../db/Database.h"
#include "../../multithreads/MultiThread.hpp"
#include <queue>
/**********************************************/
/*Define Global Varaibles*/
struct prioritize {
  bool operator()(Table::Iterator &a, Table::Iterator &b) {
    const std::string &key1(a->key()), &key2(b->key());
    std::size_t key1_len(key1.size()), key2_len(key2.size());
    for (std::size_t i = 0;; i++) {
      if (i == key1_len)
        return false;
      if (i == key2_len)
        return true;
      if (key1[i] < key2[i])
        return false;
      if (key1[i] > key2[i])
        return true;
    }
  }
};

constexpr const char *SelectQuery::qname;
static unsigned int total_thread;
static unsigned int subtable_num;
static Table *copy_table;
static ComplexQuery *copy_this;
static std::pair<std::string, bool> result;
using namespace std;
static priority_queue<Table::Iterator, vector<Table::Iterator>, prioritize>
    priorityQueue;
/**********************************************/
void Sub_Select(int id) {
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  if (result.second) {
    for (auto item = head; item != tail; item++) {
      if (copy_this->evalCondition(*item) == true)
        priorityQueue.push(item);
    }
  }
  return;
}

QueryResult::Ptr SelectQuery::execute() {
  using namespace std;
  Database &db = Database::getInstance();
  // start of try
  try {
    auto &table = db[this->targetTable];
    result = initCondition(table);
    total_thread = (unsigned int)worker.Thread_count();

    std::ostringstream oss;

    if (result.second) {
      for (auto it(table.begin()); it != table.end(); ++it) {
        if (this->evalCondition(*it) == true)
          priorityQueue.push(it);
      }
    } else {
      copy_table = &table;
      copy_this = this;
      subtable_num = (unsigned int)(table.size()) / total_thread;
      std::vector<std::future<void>> futures((unsigned long)total_thread);
      for (int i = 0; i < (int)total_thread - 1; i++) {
        futures[(unsigned long)i] = worker.Submit(Sub_Select, i);
      }
      Sub_Select((int)total_thread - 1);
      for (unsigned long i = 0; i < (unsigned long)total_thread - 1; i++) {
        futures[i].get();
      }
    }
    if (priorityQueue.size() == 0)
      return make_unique<NullQueryResult>();
    while (priorityQueue.size() != 0) {
      auto top = priorityQueue.top();
      oss << "( ";
      for (auto it = this->operands.begin(); it != this->operands.end(); ++it) {
        if (*it == "KEY")
          oss << top->key();
        else
          oss << (*top)[*it];
        oss << " ";
      }
      oss << ")";
      if (priorityQueue.size() > 1)
        oss << "\n";
      priorityQueue.pop();
    }
    return make_unique<SuccessMsgResult>(oss.str());
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

std::string SelectQuery::toString() {
  return "QUERY = SELECT " + this->targetTable + "\"";
}
