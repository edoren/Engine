#include <Util/Config.hpp>

class ENGINE_SYMBOL_EXPORTS Operation {
public:
    virtual ~Operation() {};

    virtual float Operate(float a, float b) = 0;

    virtual const char* GetName() = 0;
};
