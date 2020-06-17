#include <Renderer/Scene.hpp>

#include <Renderer/ModelManager.hpp>
#include <Renderer/RenderStates.hpp>
#include <Renderer/RenderWindow.hpp>
#include <System/FileSystem.hpp>
#include <System/JSON.hpp>
#include <System/LogManager.hpp>

namespace engine {

namespace {

const String sTag("Scene");
}

Scene::Scene(json data) : m_data(std::move(data)) {}

Scene::~Scene() {
    unload();
}

bool Scene::load() {
    if (!m_data.is_object()) {
        LogError(sTag, "Data must be an object");
        return false;
    }

    const json& jsonName = m_data["name"];
    if (jsonName.is_string()) {
        m_name = jsonName;
    } else {
        LogWarning(sTag, "Scene does not contain a name");
    }

    auto& fileSystem = FileSystem::GetInstance();

    const json& jsonData = m_data["data"];
    if (jsonData.is_object()) {
        const json& jsonObjects = jsonData["objects"];
        for (const json& jsonObject : jsonObjects) {
            const json& modelJson = jsonObject["model"];
            const json& positionJson = jsonObject["position"];
            const json& rotationJson = jsonObject["rotation"];
            const json& scaleJson = jsonObject["scale"];

            Transform modelMatrix;
            if (!scaleJson.is_null()) {
                modelMatrix.scale(math::vec3(float(scaleJson)));
            }
            if (!rotationJson.is_null()) {
                modelMatrix.rotate({float(rotationJson[0]), float(rotationJson[1]), float(rotationJson[2])});
            }
            if (!positionJson.is_null()) {
                modelMatrix.translate({float(positionJson[0]), float(positionJson[1]), float(positionJson[2])});
            }

            String normalizedPath = fileSystem.normalizePath(modelJson);

            Model* model = ModelManager::GetInstance().loadFromFile(normalizedPath);
            auto& transforms = m_models[model];
            transforms.emplace_back(modelMatrix);

            auto foundIt = m_numModelInstance.find(normalizedPath);
            if (foundIt == m_numModelInstance.end()) {
                m_numModelInstance.emplace(normalizedPath, 1);
            } else {
                foundIt->second += 1;
            }
        }
    } else {
        LogError(sTag, "Scene does not contain data");
    }

    return true;
}

bool Scene::unload() {
    for (auto& modelPair : m_models) {
        Model* model = modelPair.first;
        std::for_each(modelPair.second.cbegin(), modelPair.second.cend(),
                      [&model](auto& /*unused*/) { ModelManager::GetInstance().unload(model); });
    }
    return true;
}

void Scene::draw(RenderWindow& target) {
    for (auto& modelPair : m_models) {
        Model* model = modelPair.first;

        RenderStates states;
        for (auto& transform : modelPair.second) {
            states.transform = transform;
            model->draw(target, states);
        }
    }
}

const String& Scene::getName() {
    return m_name;
}

}  // namespace engine
