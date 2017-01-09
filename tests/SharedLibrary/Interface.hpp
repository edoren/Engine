#include <string>
#include <Util/Platform.hpp>

#if PLATFORM_IS(PLATFORM_WINDOWS)
#define EXPORTS __declspec(dllexport)
#else
#define EXPORTS
#endif

class EXPORTS Operation {
public:
    virtual void Operate(float a, float b) = 0;

    virtual const char* GetName() = 0;
    float GetLastResult() { return last_result; };

protected:
    float last_result;
};
