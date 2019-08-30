#pragma once

#include <Util/Prerequisites.hpp>

#ifdef ENGINE_NOEXCEPTION
#define JSON_NOEXCEPTION
#endif
#include <nlohmann/json.hpp>

namespace engine {

using json = ::nlohmann::json;

class String;

ENGINE_API void to_json(json& j, const String& str);

ENGINE_API void from_json(const json& j, String& str);

}  // namespace engine
