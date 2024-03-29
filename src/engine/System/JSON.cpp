#include <System/JSON.hpp>

#include <System/String.hpp>

#include <string>

namespace engine {

void to_json(json& j, const String& str) {
    j = str.toUtf8();
}

void from_json(const json& j, String& str) {
    str = j.get<std::string>();
}

}  // namespace engine
