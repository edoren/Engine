#include "Interface.hpp"

class EXPORTS OperationAdd : public Operation {
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
EXPORTS OperationAdd impl;
};
