#include "Interface.hpp"

class EXPORTS OperationMult : public Operation {
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
EXPORTS OperationMult impl;
};
