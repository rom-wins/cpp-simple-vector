#pragma once
#include <utility>
#include "../include/simple_vector.h"

class X {
public:
    X()
        : X(5) {
    }
    X(size_t num)
        : x_(num) {
    }
    X(const X& other) = delete;
    X& operator=(const X& other) = delete;
    X(X&& other) {
        x_ = std::exchange(other.x_, 0);
    }
    X& operator=(X&& other) {
        x_ = std::exchange(other.x_, 0);
        return *this;
    }
    size_t GetX() const {
        return x_;
    }

private:
    size_t x_;
};
void TestReserveConstructor(); 

void TestReserveMethod();

inline void Test1();

inline void Test2();
SimpleVector<int> GenerateVector(size_t size);

void TestTemporaryObjConstructor();

void TestTemporaryObjOperator();

void TestNamedMoveConstructor();

void TestNamedMoveOperator();

void TestNoncopiableMoveConstructor();

void TestNoncopiablePushBack();

void TestNoncopiableInsert();

void TestNoncopiableErase();

void RunAllTests();
