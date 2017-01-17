#include "Interface.hpp"

class ENGINE_SYMBOL_EXPORTS OperationMult : public Operation {
public:
    OperationMult() {}

    virtual void Operate(float a, float b) {
        last_result = a * b;
    }

    const char* GetName() {
        return "Multiplication";
    }
};

extern "C" {
ENGINE_SYMBOL_EXPORTS OperationMult impl;
};
