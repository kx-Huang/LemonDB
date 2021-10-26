//
// Created by liu on 18-10-25.
//

#include "QueryBuilders.h"

#include <iomanip>
#include <iostream>

#include "../db/Database.h"
#include "data/InsertQuery.h"
#include "data/UpdateQuery.h"
#include "data/DeleteQuery.h"
#include "data/SwapQuery.h"
#include "data/SelectQuery.h"
#include "data/SumQuery.h"
#include "data/CountQuery.h"
#include "management/DropTableQuery.h"
#include "management/DumpTableQuery.h"
#include "management/ListTableQuery.h"
#include "management/LoadTableQuery.h"
#include "management/PrintTableQuery.h"
#include "management/QuitQuery.h"

// Prints out debugging information.
// Does no real work
Query::Ptr FakeQueryBuilder::tryExtractQuery(TokenizedQueryString &query) {
  std::cerr << "Query string: \n" << query.rawQeuryString << "\n";
  std::cerr << "Tokens:\n";
  int count = 0;
  for (const auto &tok : query.token) {
    std::cerr << std::setw(10) << "\"" << tok << "\"";
    count = (count + 1) % 5;
    if (count == 4)
      std::cerr << std::endl;
  }
  if (count != 4)
    std::cerr << std::endl;
  return this->nextBuilder->tryExtractQuery(query);
}

Query::Ptr
ManageTableQueryBuilder::tryExtractQuery(TokenizedQueryString &query) {
  if (query.token.size() == 2) {
    if (query.token.front() == "LOAD") {
      auto &db = Database::getInstance();
      auto tableName = db.getFileTableName(query.token[1]);
      return std::make_unique<LoadTableQuery>(tableName, query.token[1]);
    }
    if (query.token.front() == "DROP")
      return std::make_unique<DropTableQuery>(query.token[1]);
    if (query.token.front() == "TRUNCATE")
      return std::make_unique<NopQuery>(); // Not implemented
    // return std::make_unique<TruncateTableQuery>(query.token[1]);
  }
  if (query.token.size() == 3) {
    if (query.token.front() == "DUMP") {
      auto &db = Database::getInstance();
      db.updateFileTableName(query.token[2], query.token[1]);
      return std::make_unique<DumpTableQuery>(query.token[1], query.token[2]);
    }
    if (query.token.front() == "COPYTABLE")
      return std::make_unique<NopQuery>(); // Not implemented
    // return std::make_unique<CopyTableQuery>(query.token[1], query.token[2]);
  }
  return this->nextBuilder->tryExtractQuery(query);
}

Query::Ptr DebugQueryBuilder::tryExtractQuery(TokenizedQueryString &query) {
  if (query.token.size() == 1) {
    if (query.token.front() == "LIST")
      return std::make_unique<ListTableQuery>();
    if (query.token.front() == "QUIT")
      return std::make_unique<QuitQuery>();
  }
  if (query.token.size() == 2) {
    if (query.token.front() == "SHOWTABLE")
      return std::make_unique<PrintTableQuery>(query.token[1]);
  }
  return BasicQueryBuilder::tryExtractQuery(query);
}

void ComplexQueryBuilder::parseToken(TokenizedQueryString &query) {
  // Treats forms like:
  //
  // $OPER$ ( arg1 arg2 ... )
  // FROM table
  // WHERE ( KEY = $STR$ ) ( $field$ $OP$ $int$ ) ...
  //
  // The "WHERE" clause can be ommitted
  // The args of OPER clause can be ommitted

  auto it = query.token.cbegin();
  auto end = query.token.cend();
  it += 1; // Take to args;
  if (it == query.token.end())
    throw IllFormedQuery("Missing FROM clause");
  if (*it != "FROM") {
    if (*it != "(")
      throw IllFormedQuery("Ill-formed operand.");
    ++it;
    while (*it != ")") {
      this->operandToken.push_back(*it);
      ++it;
      if (it == end)
        throw IllFormedQuery("Ill-formed operand");
    }
    if (++it == end || *it != "FROM")
      throw IllFormedQuery("Missing FROM clause");
  }
  if (++it == end)
    throw IllFormedQuery("Missing targed table");
  this->targetTable = *it;
  if (++it == end) // the "WHERE" clause is ommitted
    return;
  if (*it != "WHERE")
    // Hmmm, C++11 style Raw-string literal
    // Reference:
    // http://en.cppreference.com/w/cpp/language/string_literal
    throw IllFormedQuery(R"(Expecting "WHERE", found "?".)"_f % *it);
  while (++it != end) {
    if (*it != "(")
      throw IllFormedQuery("Ill-formed query condition");
    QueryCondition cond;
    if (++it == end)
      throw IllFormedQuery("Missing field in condition");
    cond.field = *it;
    if (++it == end)
      throw IllFormedQuery("Missing operator in condition");
    cond.op = *it;
    if (++it == end)
      throw IllFormedQuery("Missing  in condition");
    cond.value = *it;
    if (++it == end || *it != ")")
      throw IllFormedQuery("Ill-formed query condition");
    this->conditionToken.push_back(cond);
  }
}

Query::Ptr ComplexQueryBuilder::tryExtractQuery(TokenizedQueryString &query) {
  try {
    this->parseToken(query);
  } catch (const IllFormedQuery &e) {
    std::cerr << e.what() << std::endl;
    return this->nextBuilder->tryExtractQuery(query);
  }
  std::string operation = query.token.front();
  if (operation == "INSERT")
    return std::make_unique<InsertQuery>(this->targetTable, this->operandToken,
                                         this->conditionToken);
  if (operation == "UPDATE")
    return std::make_unique<UpdateQuery>(this->targetTable, this->operandToken,
                                         this->conditionToken);
  if (operation == "SELECT")
    // return std::make_unique<NopQuery>(); // Not implemented
  return std::make_unique<SelectQuery>(
          this->targetTable, this->operandToken, this->conditionToken);
  if (operation == "DELETE")
    return std::make_unique<DeleteQuery>(
          this->targetTable, this->operandToken, this->conditionToken);
  if (operation == "DUPLICATE")
    return std::make_unique<NopQuery>(); // Not implemented
  /*return std::make_unique<DuplicateQuery>(
          this->targetTable, this->operandToken, this->conditionToken);*/
  if (operation == "COUNT")
    // return std::make_unique<NopQuery>(); // Not implemented
    return std::make_unique<CountQuery>(
            this->targetTable, this->operandToken, this->conditionToken);
  if (operation == "SUM")
    // return std::make_unique<NopQuery>(); // Not implemented
    return std::make_unique<SumQuery>(
            this->targetTable, this->operandToken, this->conditionToken);
  if (operation == "MIN")
    return std::make_unique<NopQuery>(); // Not implemented
                                         /*return std::make_unique<MinQuery>(
                                                 this->targetTable, this->operandToken, this->conditionToken);*/
  if (operation == "MAX")
    return std::make_unique<NopQuery>(); // Not implemented
                                         /*return std::make_unique<MaxQuery>(
                                                 this->targetTable, this->operandToken, this->conditionToken);*/
  if (operation == "ADD")
    return std::make_unique<NopQuery>(); // Not implemented
                                         /*return std::make_unique<AddQuery>(
                                                 this->targetTable, this->operandToken, this->conditionToken);*/
  if (operation == "SUB")
    return std::make_unique<NopQuery>(); // Not implemented
                                         /*return std::make_unique<SubQuery>(
                                                 this->targetTable, this->operandToken, this->conditionToken);*/
  if (operation == "SWAP")
    // return std::make_unique<NopQuery>(); // Not implemented
    return std::make_unique<SwapQuery>(this->targetTable, this->operandToken, this->conditionToken);
  std::cerr << "Complicated query found!" << std::endl;
  std::cerr << "Operation = " << query.token.front() << std::endl;
  std::cerr << "    Operands : ";
  for (const auto &oprand : this->operandToken)
    std::cerr << oprand << " ";
  std::cerr << std::endl;
  std::cerr << "Target Table = " << this->targetTable << std::endl;
  if (this->conditionToken.empty())
    std::cerr << "No WHERE clause specified." << std::endl;
  else
    std::cerr << "Conditions = ";
  for (const auto &cond : this->conditionToken)
    std::cerr << cond.field << cond.op << cond.value << " ";
  std::cerr << std::endl;

  return this->nextBuilder->tryExtractQuery(query);
}

void ComplexQueryBuilder::clear() {
  this->conditionToken.clear();
  this->targetTable = "";
  this->operandToken.clear();
  this->nextBuilder->clear();
}