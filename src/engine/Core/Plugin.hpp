#pragma once

#include <System/String.hpp>

namespace engine {

/**
 * @brief Class to load component to the engine
 */
class ENGINE_API Plugin {
public:
    virtual ~Plugin() = default;

    /**
     * @brief Returns the name of the plugin
     * @return String with the name
     */
    virtual const String& getName() const = 0;

    /**
     * @brief Install a new plugin
     *
     * This should register any component into the Engine
     */
    virtual void install() = 0;

    /**
     * @brief Initialize the plugin
     *
     * Called after all the engine core subsystems have been initialized
     */
    virtual void initialize() = 0;

    /**
     * @brief Shutdowns the plugin
     *
     * This should delete any dependency of the plugin that uses the engine core subsystems
     */
    virtual void shutdown() = 0;

    /**
     * @brief Uninstall the plugin
     *
     * Called before removing and unloading the plugin from the engine
     */
    virtual void uninstall() = 0;
};

}  // namespace engine
