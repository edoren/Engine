#include <Graphics/Image.hpp>
#include <Graphics/ImageLoader.hpp>

namespace engine {

Image::Image() : m_size(0, 0), m_pixels(0) {}

bool Image::LoadFromFile(const String& filename) {
    return io::ImageLoader::LoadFromFile(filename, m_pixels, m_size);
}

bool Image::LoadFromFileInMemory(const byte* buffer, uint32 len) {
    return io::ImageLoader::LoadFromFileInMemory(buffer, len, m_pixels, m_size);
}

bool Image::LoadFromMemory(const Color32* color_map, uint32 width, uint32 height) {
    m_size.x = width;
    m_size.y = height;
    const byte* data = reinterpret_cast<const byte*>(color_map);
    m_pixels.assign(data, data + (width * height * 4));
    return true;
}

void Image::Clear() {
    m_size.x = 0;
    m_size.y = 0;
    m_pixels.clear();
}

const math::uvec2& Image::GetSize() const {
    return m_size;
}

byte* Image::GetData() {
    return m_pixels.data();
}

const byte* Image::GetData() const {
    return m_pixels.data();
}

size_t Image::GetDataSize() const {
    return m_size.x * m_size.y * sizeof(byte) * 4;
}

}  // namespace engine
