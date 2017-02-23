#include "Operation.hpp"

class ENGINE_SYMBOL_EXPORTS OperationAdd : public Operation {
public:
    OperationAdd() {}

    float Operate(float a, float b) {
        return a + b;
    }

    const char* GetName() {
        return "Addition";
    }
};

static OperationAdd s_impl;

extern "C" {

ENGINE_SYMBOL_EXPORTS Operation* GetOperation() {
    return &s_impl;
}

};
