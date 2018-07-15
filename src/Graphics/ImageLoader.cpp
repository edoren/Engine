#include <Graphics/ImageLoader.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace engine {

namespace {

const String sTag("ImageLoader");

}  // namespace

namespace io {

bool ImageLoader::LoadFromFile(const String& filename,
                               std::vector<byte>& pixels, math::uvec2& size) {
    std::vector<byte> out;
    FileSystem& fs = FileSystem::GetInstance();
    if (fs.LoadFileData(filename, &out)) {
        return LoadFromFileInMemory(out.data(), static_cast<uint32>(out.size()),
                                    pixels, size);
    }
    return false;
}

bool ImageLoader::LoadFromFileInMemory(const byte* buffer, uint32 len,
                                       std::vector<byte>& pixels,
                                       math::uvec2& size) {
    int width;
    int height;
    int comp;

    byte* data = stbi_load_from_memory(buffer, len, &width, &height, &comp,
                                       STBI_rgb_alpha);

    if (data != nullptr) {
        size_t num_pixels = width * height;
        size_t data_size = (num_pixels * STBI_rgb_alpha);
        byte* data_end = data + data_size;
        pixels.assign(data, data_end);
    } else {
        String error = String("STB_Image error: ") + stbi_failure_reason();
        LogError(sTag, error);
        stbi_image_free(data);
        return false;
    }

    size.x = static_cast<uint32>(width);
    size.y = static_cast<uint32>(height);

    stbi_image_free(data);
    return true;
}

}  // namespace io

}  // namespace engine
