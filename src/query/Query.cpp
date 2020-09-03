//
// Created by liu on 18-10-25.
//

#include "Query.h"

#include <cassert>

std::pair<std::string, bool> ComplexQuery::initCondition(const Table &table) {
  const std::unordered_map<std::string, int> opmap{
      {">", '>'}, {"<", '<'}, {"=", '='}, {">=", 'g'}, {"<=", 'l'},
  };
  std::pair<std::string, bool> result = {"", true};
  for (auto &cond : condition) {
    if (cond.field == "KEY") {
      if (cond.op != "=") {
        throw IllFormedQueryCondition("Can only compare equivalence on KEY");
      } else if (result.first.empty()) {
        result.first = cond.value;
      } else if (result.first != cond.value) {
        result.second = false;
        return result;
      }
    } else {
      cond.fieldId = table.getFieldIndex(cond.field);
      cond.valueParsed =
          (Table::ValueType)std::strtol(cond.value.c_str(), nullptr, 10);
      int op = 0;
      try {
        op = opmap.at(cond.op);
      } catch (const std::out_of_range &e) {
        throw IllFormedQueryCondition(
            R"("?" is not a valid condition operator.)"_f % cond.op);
      }
      switch (op) {
      case '>':
        cond.comp = std::greater<>();
        break;
      case '<':
        cond.comp = std::less<>();
        break;
      case '=':
        cond.comp = std::equal_to<>();
        break;
      case 'g':
        cond.comp = std::greater_equal<>();
        break;
      case 'l':
        cond.comp = std::less_equal<>();
        break;
      default:
        assert(0); // should never be here
      }
    }
  }
  return result;
}

bool ComplexQuery::evalCondition(const Table::Object &object) {
  bool ret = true;
  for (const auto &cond : condition) {
    if (cond.field != "KEY") {
      ret = ret && cond.comp(object[cond.fieldId], cond.valueParsed);
    } else {
      ret = ret && (object.key() == cond.value);
    }
  }
  return ret;
}

bool ComplexQuery::testKeyCondition(
    Table &table, std::function<void(bool, Table::Object::Ptr &&)> function) {
  auto condResult = initCondition(table);
  if (!condResult.second) {
    function(false, nullptr);
    return true;
  }
  if (!condResult.first.empty()) {
    auto object = table[condResult.first];
    if (object != nullptr && evalCondition(*object)) {
      function(true, std::move(object));
    } else {
      function(false, nullptr);
    }
    return true;
  }
  return false;
}
