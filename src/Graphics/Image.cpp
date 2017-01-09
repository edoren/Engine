#include <Graphics/Image.hpp>
#include <System/IO/ImageLoader.hpp>

namespace engine {

Image::Image() : size_(0, 0), pixels_(0) {}

bool Image::LoadFromFile(const String& filename) {
    return io::ImageLoader::LoadFromFile(filename, pixels_, size_);
}

bool Image::LoadFromFileInMemory(const byte* buffer, uint32 len) {
    return io::ImageLoader::LoadFromFileInMemory(buffer, len, pixels_, size_);
}

bool Image::LoadFromMemory(const Color32* color_map, uint32 width,
                           uint32 height) {
    size_.x = width;
    size_.y = height;
    const byte* data = reinterpret_cast<const byte*>(color_map);
    pixels_.assign(data, data + (width * height * 4));
    return true;
}

void Image::Clear() {
    size_.x = 0;
    size_.y = 0;
    pixels_.clear();
}

const math::uvec2& Image::GetSize() const {
    return size_;
}

byte* Image::GetData() {
    return pixels_.data();
}

const byte* Image::GetData() const {
    return pixels_.data();
}

}  // namespace engine
