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
        include/v9/fp/basic.hpp)
add_library(v9 ${SOURCE_FILES})

add_executable(qsort tests/qsort.cpp)
add_executable(exp tests/exp.cpp)
add_executable(queens tests/queens.cpp)
add_executable(palindrome tests/palindrome.cpp)
add_executable(split tests/split.cpp)
add_executable(delegate tests/delegate.cpp)
add_executable(time-calculator tests/time-calculator.cpp)
add_executable(work tests/work.cpp)
add_executable(super-max tests/super-max.cpp)
add_executable(traits tests/traits.cpp)
add_executable(fp-curry tests/fp-curry.cpp)
add_executable(fp-basic tests/fp-basic.cpp)
add_executable(fp-fix tests/fp-fix.cpp)
