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

    void draw(RenderWindow& target);

private:
    explicit Scene(const json& data);

    bool load();
    bool unload();

    const String& getName();

    String m_name;
    std::map<Model*, std::vector<Transform>> m_models;
    std::map<String, uint32> m_numModelInstance;
    json m_data;
};

}  // namespace engine
