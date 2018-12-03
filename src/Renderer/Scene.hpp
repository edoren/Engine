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

    const String& GetName();

private:
    String m_name;
    std::vector<std::pair<Model*, Transform>> m_models;
    json m_data;
};

}  // namespace engine
