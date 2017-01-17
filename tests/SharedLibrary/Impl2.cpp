#include "Interface.hpp"

class ENGINE_SYMBOL_EXPORTS OperationAdd : public Operation {
public:
    OperationAdd() {}

    virtual void Operate(float a, float b) {
        last_result = a + b;
    }

    const char* GetName() {
        return "Addition";
    }
};

extern "C" {
ENGINE_SYMBOL_EXPORTS OperationAdd impl;
};
