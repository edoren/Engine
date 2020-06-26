#include <Graphics/ImageLoader.hpp>
#include <System/FileSystem.hpp>
#include <System/LogManager.hpp>
#include <Util/Container/Vector.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace engine {

namespace {

const String sTag("ImageLoader");

}  // namespace

namespace io {

bool ImageLoader::LoadFromFile(const String& filename, Vector<byte>& pixels, math::uvec2& size) {
    Vector<byte> out;
    FileSystem& fs = FileSystem::GetInstance();
    if (fs.loadFileData(filename, &out)) {
        return LoadFromFileInMemory(out.data(), static_cast<uint32>(out.size()), pixels, size);
    }
    return false;
}

bool ImageLoader::LoadFromFileInMemory(const byte* buffer, uint32 len, Vector<byte>& pixels, math::uvec2& size) {
    int width;
    int height;
    int comp;

    byte* data = stbi_load_from_memory(buffer, len, &width, &height, &comp, STBI_rgb_alpha);

    if (data != nullptr) {
        size_t numPixels = width * height;
        size_t dataSize = (numPixels * STBI_rgb_alpha);
        byte* dataEnd = data + dataSize;
        pixels.assign(data, dataEnd);
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
