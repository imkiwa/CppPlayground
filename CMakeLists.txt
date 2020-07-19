cmake_minimum_required(VERSION 3.8)
project(v9)

include_directories(include)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES src/v9.cpp
        include/v9/v9.hpp
        include/v9/algorithm/qsort.hpp
        include/v9/algorithm/queens.h
        include/v9/algorithm/palindrome.h
        include/v9/bits/types.hpp
        include/v9/bits/traits.hpp
        include/v9/expression/evaluator.h
        include/v9/memory/delegate.h
        include/v9/fp/curry.hpp
        include/v9/fp/fix.hpp
        include/v9/fp/fp.hpp
        include/v9/fp/basic.hpp
        include/v9/kit/event.hpp
        include/v9/kit/optional.hpp
        include/v9/kit/server.hpp
        include/v9/kit/string.hpp
        )
add_library(v9 ${SOURCE_FILES})

add_executable(qsort tests/qsort.cpp)
add_executable(exp tests/exp.cpp)
add_executable(queens tests/queens.cpp)
add_executable(palindrome tests/palindrome.cpp)
add_executable(split tests/split.cpp)
add_executable(delegate tests/delegate.cpp)
add_executable(time-calculator tests/time-calculator.cpp)
add_executable(super-max tests/super-max.cpp)
add_executable(traits tests/traits.cpp)
add_executable(fp-curry tests/fp-curry.cpp)
add_executable(fp-basic tests/fp-basic.cpp)
add_executable(fp-fix tests/fp-fix.cpp)
add_executable(huffman tests/huffman.cpp)
add_executable(optional tests/optional.cpp)
add_executable(event-emitter tests/event-emitter.cpp)
add_executable(io-server tests/io-server.cpp)
add_executable(hack-vptr tests/hack-vptr.cpp)
add_executable(vptr-hacker tests/vptr-hacker.cpp)
add_executable(timeout tests/timeout.cpp)
add_executable(string tests/string.cpp)
add_executable(typelist tests/typelist.cpp)
add_executable(staticlist tests/staticlist.cpp)
add_executable(tuple tests/tuple.cpp)
add_executable(ai-compiler tests/ai-compiler.cpp)
add_executable(tmp00 tests/tmp-00.cpp)
add_executable(tmp01 tests/tmp-01.cpp)
add_executable(bind tests/bind.cpp)
add_executable(ref-value tests/ref-value.cpp)
add_executable(vptr tests/vptr.cpp)
add_executable(oop-c tests/oop.c)
add_executable(vm tests/vm.cpp)
add_executable(dustbin tests/dustbin.cpp)
add_executable(pay tests/pay.cpp)
add_executable(lifetime tests/lifetime.cpp)
add_executable(callcc tests/callcc.cpp)
add_executable(leetcode524 tests/leetcode524.cpp)
add_executable(linkedlist tests/linkedlist.c)
add_executable(linkedlist-new tests/linkedlist-new.cpp)
add_executable(cyaron tests/cyaron-lang.cpp)
add_executable(ptr tests/ptr.cpp)
add_executable(http-server tests/http-server.cpp)
add_executable(sv tests/sv.c)
add_executable(ph tests/ph.c)
add_executable(clt tests/clt.cpp)
add_executable(calc-lang tests/calc-lang.cpp)

if (EXISTS ${CMAKE_SOURCE_DIR}/works)
    add_subdirectory(works)
endif ()
