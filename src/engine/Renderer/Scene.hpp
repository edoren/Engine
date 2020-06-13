#pragma once

#include <Util/Prerequisites.hpp>

#include <Renderer/Model.hpp>
#include <Renderer/Transform.hpp>
#include <System/JSON.hpp>
#include <System/String.hpp>

namespace engine {

class ENGINE_API Scene : NonCopyable {
    friend class SceneManager;

public:
    ~Scene();

    void Draw(RenderWindow& target);

private:
    explicit Scene(const json& data);

    bool Load();
    bool Unload();

    const String& GetName();

private:
    String m_name;
    std::map<Model*, std::vector<Transform>> m_models;
    std::map<String, uint32> m_num_model_instance;
    json m_data;
};

}  // namespace engine
