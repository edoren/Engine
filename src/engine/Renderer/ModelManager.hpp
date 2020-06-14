#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Mesh.hpp>
#include <Renderer/Model.hpp>

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

    static ModelManager& GetInstance();
    static ModelManager* GetInstancePtr();

    virtual std::unique_ptr<Mesh> createMesh() = 0;

protected:
    virtual std::unique_ptr<Model> createModel() = 0;

    std::map<String, Model*> m_name_map;
    std::map<Model*, uint32> m_model_ref_count;
    std::vector<std::unique_ptr<Model>> m_models;
};

}  // namespace engine
