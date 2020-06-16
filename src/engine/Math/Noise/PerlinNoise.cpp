#include <Math/Noise/PerlinNoise.hpp>

namespace engine::math {

const int PerlinNoise::sDefaultSeed = 0;
const int PerlinNoise::sDefaultOctaveCount = 6;
const float PerlinNoise::sDefaultFrequency = 1.0F;
const float PerlinNoise::sDefaultLacunarity = 2.0F;
const float PerlinNoise::sDefaultPersistence = 0.5F;
const int PerlinNoise::sMaxOctaveCount = 30;

PerlinNoise::PerlinNoise() : PerlinNoise(sDefaultSeed) {}

PerlinNoise::PerlinNoise(int seed)
      : BaseNoise(seed),
        m_octaveCount(sDefaultOctaveCount),
        m_frequency(sDefaultFrequency),
        m_lacunarity(sDefaultLacunarity),
        m_persistence(sDefaultPersistence) {}

int PerlinNoise::getOctaveCount() const {
    return m_octaveCount;
}

float PerlinNoise::getFrequency() const {
    return m_frequency;
}

float PerlinNoise::getLacunarity() const {
    return m_lacunarity;
}

float PerlinNoise::getPersistence() const {
    return m_persistence;
}

float PerlinNoise::getValue(float x, float y, float z) const {
    float value = 0.F;
    float signal = 0.F;
    float frequency = m_frequency;
    float amplitude = 1.F;

    for (int current_octave = 0; current_octave < m_octaveCount; current_octave++) {
        signal = coherentNoise3D(x * frequency, y * frequency, z * frequency);
        value += signal * amplitude;

        frequency *= m_lacunarity;
        amplitude *= m_persistence;
    }

    return std::min(1.F, std::max(-1.F, value));
}

void PerlinNoise::setOctaveCount(int octaveCount) {
    m_octaveCount = octaveCount;
}

void PerlinNoise::setFrequency(float frequency) {
    m_frequency = frequency;
}

void PerlinNoise::setLacunarity(float lacunarity) {
    m_lacunarity = lacunarity;
}

void PerlinNoise::setPersistence(float persistence) {
    m_persistence = persistence;
}

}  // namespace engine::math
