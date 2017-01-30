#pragma once

#include <Util/Prerequisites.hpp>

#include <Graphics/ResourceManager.hpp>
#include <Input/InputManager.hpp>
#include <Renderer/Renderer.hpp>

namespace engine {

class ENGINE_API Main : public Singleton<Main> {
public:
    Main(int argc, char* argv[]);

    ~Main();

    bool Initialize();

    void ShutDown();

    Renderer& GetRenderer();

    Renderer* GetRendererPtr();

    // Override standard Singleton retrieval.
    //     @remarks
    //         Why do we do this? Well, it's because the Singleton
    //         implementation is in a .h file, which means it gets compiled
    //         into anybody who includes it. This is needed for the
    //         Singleton template to work, but we actually only want it
    //         compiled into the implementation of the class based on the
    //         Singleton, not all of them. If we don't change this, we get
    //         link errors when trying to use the Singleton-based class from
    //         an outside dll.
    //     @par
    //         This method just delegates to the template version anyway,
    //         but the implementation stays in this single compilation unit,
    //         preventing link errors.
    static Main& GetInstance();

    // Override standard Singleton retrieval.
    //     @remarks
    //         Why do we do this? Well, it's because the Singleton
    //         implementation is in a .h file, which means it gets compiled
    //         into anybody who includes it. This is needed for the
    //         Singleton template to work, but we actually only want it
    //         compiled into the implementation of the class based on the
    //         Singleton, not all of them. If we don't change this, we get
    //         link errors when trying to use the Singleton-based class from
    //         an outside dll.
    //     @par
    //         This method just delegates to the template version anyway,
    //         but the implementation stays in this single compilation unit,
    //         preventing link errors.
    static Main* GetInstancePtr();

private:
    bool is_initialized_;

    // Singletons
    InputManager* m_input;
    ResourceManager* m_resources;
    Renderer* m_renderer;
};

}  // namespace engine
