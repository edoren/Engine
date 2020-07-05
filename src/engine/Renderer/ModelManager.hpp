#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Mesh.hpp>
#include <Renderer/Model.hpp>
#include <Util/Container/Vector.hpp>
#include <Util/Singleton.hpp>

#include <map>
#include <memory>

namespace engine {

class Model;

class ENGINE_API ModelManager : public Singleton<ModelManager> {
public:
    ModelManager();

    virtual ~ModelManager();

    virtual void initialize();

    virtual void shutdown();

    Model* loadFromFile(const String& basename);

    void unload(Model* model);
    void unloadFromFile(const String& basename);

    virtual std::unique_ptr<Mesh> createMesh() = 0;  // TODO: protected

protected:
    virtual std::unique_ptr<Model> createModel() = 0;

    std::map<String, Model*> m_nameMap;
    std::map<Model*, uint32> m_modelRefCount;
    Vector<std::unique_ptr<Model>> m_models;
};

}  // namespace engine
