#pragma once

#ifdef ENGINE_NOEXCEPTION
#define JSON_NOEXCEPTION
#endif
#include <nlohmann/json.hpp>

namespace engine {

using json = ::nlohmann::json;

class String;

void to_json(json& j, const String& str);

void from_json(const json& j, String& str);

}  // namespace engine
