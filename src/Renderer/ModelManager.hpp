#pragma once

#include <Util/Prerequisites.hpp>

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

    Model* GetModel(const String& name);

protected:
    std::map<String, Model*> m_name_map;
    std::map<uint32, Model*> m_index_map;
    std::vector<std::unique_ptr<Model>> m_models;
};

}  // namespace engine
