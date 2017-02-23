#include "Operation.hpp"

class ENGINE_SYMBOL_EXPORTS OperationMult : public Operation {
public:
    OperationMult() {}

    float Operate(float a, float b) {
        return a * b;
    }

    const char* GetName() {
        return "Multiplication";
    }
};

static OperationMult s_impl;

extern "C" {

ENGINE_SYMBOL_EXPORTS Operation* GetOperation() {
    return &s_impl;
}

};
