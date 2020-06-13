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

    virtual void Initialize();

    virtual void Shutdown();

    Model* LoadFromFile(const String& basename);

    void Unload(Model* model);
    void UnloadFromFile(const String& basename);

    static ModelManager& GetInstance();
    static ModelManager* GetInstancePtr();

    virtual std::unique_ptr<Mesh> CreateMesh() = 0;

protected:
    virtual std::unique_ptr<Model> CreateModel() = 0;

    std::map<String, Model*> m_name_map;
    std::map<Model*, uint32> m_model_ref_count;
    std::vector<std::unique_ptr<Model>> m_models;
};

}  // namespace engine
