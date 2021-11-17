#include "SelectQuery.h"

#include "../../db/Database.h"
#include "../../multithreads/MultiThread.hpp"

#include <queue>

/**********************************************/
/*Define Global Varaibles*/
struct prioritize {
  bool operator()(Table::Iterator &a, Table::Iterator &b) {
    if (a->key() < b->key())
      return false;
    return true;
  }
};
constexpr const char *SelectQuery::qname;
static unsigned int total_thread;
static unsigned int subtable_num;
static Table *copy_table;
static ComplexQuery *copy_this;
static std::pair<std::string, bool> result;
static std::priority_queue<Table::Iterator, std::vector<Table::Iterator>,
                           prioritize>
    priorityQueue;
static std::mutex m_mutex;
/**********************************************/

void Sub_Select(int id) {
  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  if (result.second) {
    for (auto item = head; item != tail; item++) {
      if (copy_this->evalCondition(*item) == true) {
        m_mutex.lock();
        priorityQueue.push(item);
        m_mutex.unlock();
      }
    }
  }
}

QueryResult::Ptr SelectQuery::execute() {
  using namespace std;
  Database &db = Database::getInstance();
  // start of try
  try {
    auto &table = db[this->targetTable];
    result = initCondition(table);
    total_thread = (unsigned int)worker.Thread_count();
    ostringstream oss;
    if (total_thread < 2 || table.size() < 2000) {
      if (result.second) {
        for (auto it(table.begin()); it != table.end(); ++it) {
          if (this->evalCondition(*it) == true)
            priorityQueue.push(it);
        }
        if (priorityQueue.size() == 0)
          return make_unique<NullQueryResult>();
      }
    } else {
      total_thread = (unsigned int)(table.size() / 2000 + 1);
      copy_table = &table;
      copy_this = this;
      subtable_num = (unsigned int)(table.size()) / total_thread;
      vector<std::future<void>> futures((unsigned long)total_thread);
      for (int i = 0; i < (int)total_thread; i++)
        futures[(unsigned long)i] = worker.Submit(Sub_Select, i);
      for (unsigned long i = 0; i < (unsigned long)total_thread; i++)
        futures[i].get();
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
