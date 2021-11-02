#include "SubQuery.h"
#include <iostream>

#include "../../db/Database.h"
#include "../../multithreads/MultiThread.h"
#include <algorithm>
#include <pthread.h>
#include <semaphore.h>

/**********************************************/
/*Define Global Varaibles*/

static size_t counter;
static unsigned int current_thread;
static unsigned int total_thread;
static unsigned int subtable_num;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static Table *copy_table;
static std::pair<std::string, bool> result;
static ComplexQuery *copy_this;
static std::vector<std::string> *copy_operand;
constexpr const char *SubQuery::qname;
/**********************************************/

void *Sub_SubQuery(void *) {
  pthread_mutex_lock(&mutex);
  int id = (int)current_thread;
  current_thread++;
  pthread_mutex_unlock(&mutex);

  auto head = copy_table->begin() + (id * (int)subtable_num);
  auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                          : head + (int)subtable_num;
  if (result.second) {
    int sub_counter = 0;
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
    pthread_mutex_lock(&mutex);
    counter = counter + (size_t)sub_counter;
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
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
    total_thread = get_ThreadNum();
    counter = 0;
    copy_operand = &this->operands;
    if (table.size() < 16 || total_thread < 2) {
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
      copy_table = &table;
      copy_this = this;
      subtable_num = (unsigned int)(table.size()) / total_thread;
      pthread_t *store = new pthread_t[total_thread - 1];
      current_thread = 0;
      for (int i = 0; i < (int)total_thread - 1; i++) {
        pthread_create(store + i, NULL, Sub_SubQuery, &i);
      }
      Sub_SubQuery(NULL);
      for (int i = 0; i < (int)total_thread - 1; i++) {
        pthread_join(*(store + i), NULL);
      }
      delete[] store;
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
