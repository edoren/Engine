#include <System/IO/ImageLoader.hpp>
#include <System/LogManager.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace engine {

namespace io {

bool ImageLoader::LoadFromFile(const String& filename,
                               std::vector<byte>& pixels, math::uvec2& size) {
    int width;
    int height;
    int comp;

    byte* data =
        stbi_load(filename.GetData(), &width, &height, &comp, STBI_rgb_alpha);

    if (data != nullptr && comp == STBI_rgb_alpha) {
        // Fill the vector with the pixels
        pixels.assign(data, data + (width * height * comp));
        size.x = static_cast<uint32>(width);
        size.y = static_cast<uint32>(height);
    } else {
        String error = String("STB_Image error: ") + stbi_failure_reason();
        LogError("ImageLoader", error);
        return false;
    }

    stbi_image_free(data);

    return true;
}

bool ImageLoader::LoadFromFileInMemory(const byte* buffer, uint32 len,
                                       std::vector<byte>& pixels,
                                       math::uvec2& size) {
    int width;
    int height;
    int comp;

    byte* data = stbi_load_from_memory(buffer, len, &width, &height, &comp,
                                       STBI_rgb_alpha);

    if (data != nullptr && comp == STBI_rgb_alpha) {
        // Fill the vector with the pixels
        pixels.assign(data, data + (width * height * comp));
        size.x = static_cast<uint32>(width);
        size.y = static_cast<uint32>(height);
    } else {
        String error = String("STB_Image error: ") + stbi_failure_reason();
        LogError("ImageLoader", error);
        return false;
    }

    return true;
}

}  // namespace io

}  // namespace engine
