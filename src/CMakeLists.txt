project(lemondb)
# cmake_minimum_required(VERSION 2.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS
    "-g -Wall -Wextra -Werror -pedantic -Wno-unused-result -Wconversion -Wno-unused-parameter -O2"
)

set(SOURCE_FILES
    db/Database.h
    db/Database.cpp
    db/Table.h
    db/Table.cpp
    query/Query.h
    query/Query.cpp
    query/QueryParser.h
    query/QueryParser.cpp
    query/QueryResult.h
    query/QueryResult.cpp
    query/QueryBuilders.h
    query/QueryBuilders.cpp
    query/management/LoadTableQuery.h
    query/management/LoadTableQuery.cpp
    query/management/DropTableQuery.h
    query/management/DropTableQuery.cpp
    query/management/DumpTableQuery.h
    query/management/DumpTableQuery.cpp
    query/management/ListTableQuery.h
    query/management/ListTableQuery.cpp
    query/management/QuitQuery.h
    query/management/QuitQuery.cpp
    query/management/PrintTableQuery.h
    query/management/PrintTableQuery.cpp
    query/management/QuitQuery.h
    query/management/PrintTableQuery.h
    query/management/PrintTableQuery.cpp
    query/data/InsertQuery.h
    query/data/InsertQuery.cpp
    query/data/UpdateQuery.h
    query/data/UpdateQuery.cpp
    query/data/DeleteQuery.h
    query/data/DeleteQuery.cpp
    query/data/SwapQuery.h
    query/data/SwapQuery.cpp
    utils/formatter.h
    utils/uexception.h)
add_executable(lemondb main.cpp ${SOURCE_FILES})
target_link_libraries(lemondb pthread)
