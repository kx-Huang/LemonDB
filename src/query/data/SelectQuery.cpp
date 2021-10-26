#include "SelectQuery.h"

#include "../../db/Database.h"

#include <queue>

struct prioritize {
  bool operator()(Table::Iterator &a, Table::Iterator &b) {
    const std::string &key1(a->key()), &key2(b->key());
    std::size_t key1_len(key1.size()), key2_len(key2.size());
    for (std::size_t i = 0;;i++) {
      if (i == key1_len) return false;
      if (i == key2_len) return true;
      if (key1[i] < key2[i]) return false;
      if (key1[i] > key2[i]) return true;
    }
  }
};

constexpr const char *SelectQuery::qname;

QueryResult::Ptr SelectQuery::execute() {
  using namespace std;
  Database &db = Database::getInstance();
  // start of try
  try {
    auto &table = db[this->targetTable];
    auto result = initCondition(table);
    priority_queue<Table::Iterator, vector<Table::Iterator>, prioritize> priorityQueue;
    std::ostringstream oss;
    if (result.second) {
      for (auto it(table.begin()); it != table.end(); ++it) {
        if (this->evalCondition(*it) == true)
          priorityQueue.push(it);
      }
      while (priorityQueue.size() != 0)
      {
        auto top = priorityQueue.top();
        oss << "( ";
        for (auto it = this->operands.begin(); it != this->operands.end(); ++it) {
          if (*it == "KEY") oss << top->key();
          else oss << (*top)[*it];
          oss << " ";
        }
        oss << ")" << std::endl;
        priorityQueue.pop();
      }
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
