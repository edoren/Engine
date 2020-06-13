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

Scene::Scene(const json& data) : m_data(data) {}

Scene::~Scene() {
    Unload();
}

bool Scene::Load() {
    if (!m_data.is_object()) {
        LogError(sTag, "Data must be an object");
        return false;
    }

    const json& json_name = m_data["name"];
    if (json_name.is_string()) {
        m_name = json_name;
    } else {
        LogWarning(sTag, "Scene does not contain a name");
    }

    auto& file_system = FileSystem::GetInstance();

    const json& json_data = m_data["data"];
    if (json_data.is_object()) {
        const json& json_objects = json_data["objects"];
        for (const json& json_object : json_objects) {
            const json& model_json = json_object["model"];
            const json& position_json = json_object["position"];
            const json& rotation_json = json_object["rotation"];
            const json& scale_json = json_object["scale"];

            Transform model_matrix;
            if (!scale_json.is_null()) {
                model_matrix.Scale(math::vec3(float(scale_json)));
            }
            if (!rotation_json.is_null()) {
                model_matrix.Rotate({float(rotation_json[0]), float(rotation_json[1]), float(rotation_json[2])});
            }
            if (!position_json.is_null()) {
                model_matrix.Translate({float(position_json[0]), float(position_json[1]), float(position_json[2])});
            }

            String normalized_path = file_system.NormalizePath(model_json);

            Model* model = ModelManager::GetInstance().LoadFromFile(normalized_path);
            auto& transforms = m_models[model];
            transforms.emplace_back(model_matrix);

            auto found_it = m_num_model_instance.find(normalized_path);
            if (found_it == m_num_model_instance.end()) {
                m_num_model_instance.emplace(normalized_path, 1);
            } else {
                found_it->second += 1;
            }
        }
    } else {
        LogError(sTag, "Scene does not contain data");
    }

    return true;
}

bool Scene::Unload() {
    for (auto& model_pair : m_models) {
        Model* model = model_pair.first;
        std::for_each(model_pair.second.cbegin(), model_pair.second.cend(),
                      [&model](auto& /*unused*/) { ModelManager::GetInstance().Unload(model); });
    }
    return true;
}

void Scene::Draw(RenderWindow& target) {
    for (auto& model_pair : m_models) {
        Model* model = model_pair.first;

        RenderStates states;
        for (auto& transform : model_pair.second) {
            states.transform = transform;
            model->Draw(target, states);
        }
    }
}

const String& Scene::GetName() {
    return m_name;
}

}  // namespace engine
