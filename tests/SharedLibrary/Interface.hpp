#include <string>
#include <Util/Config.hpp>

class ENGINE_SYMBOL_EXPORTS Operation {
public:
    virtual void Operate(float a, float b) = 0;

    virtual const char* GetName() = 0;
    float GetLastResult() { return last_result; };

protected:
    float last_result;
};
