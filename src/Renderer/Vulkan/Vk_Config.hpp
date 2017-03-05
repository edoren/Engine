#pragma once

#include <Util/Config.hpp>

#ifdef VULKAN_PLUGIN_EXPORTS
#define VULKAN_PLUGIN_API ENGINE_SYMBOL_EXPORTS
#else
#define VULKAN_PLUGIN_API ENGINE_SYMBOL_IMPORTS
#endif
