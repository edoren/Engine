#include <Math/Noise/PerlinNoise.hpp>

namespace engine {

namespace math {

const int PerlinNoise::sDefaultSeed = 0;
const int PerlinNoise::sDefaultOctaveCount = 6;
const float PerlinNoise::sDefaultFrequency = 1.0F;
const float PerlinNoise::sDefaultLacunarity = 2.0F;
const float PerlinNoise::sDefaultPersistence = 0.5F;
const int PerlinNoise::sMaxOctaveCount = 30;

PerlinNoise::PerlinNoise() : PerlinNoise(sDefaultSeed) {}

PerlinNoise::PerlinNoise(int seed)
      : BaseNoise(seed),
        m_octave_count(sDefaultOctaveCount),
        m_frequency(sDefaultFrequency),
        m_lacunarity(sDefaultLacunarity),
        m_persistence(sDefaultPersistence) {}

int PerlinNoise::GetOctaveCount() const {
    return m_octave_count;
}

float PerlinNoise::GetFrequency() const {
    return m_frequency;
}

float PerlinNoise::GetLacunarity() const {
    return m_lacunarity;
}

float PerlinNoise::GetPersistence() const {
    return m_persistence;
}

float PerlinNoise::GetValue(float x, float y, float z) const {
    float value = 0.F;
    float signal = 0.F;
    float frequency = m_frequency;
    float amplitude = 1.F;

    for (int current_octave = 0; current_octave < m_octave_count; current_octave++) {
        signal = CoherentNoise3D(x * frequency, y * frequency, z * frequency);
        value += signal * amplitude;

        frequency *= m_lacunarity;
        amplitude *= m_persistence;
    }

    return std::min(1.F, std::max(-1.F, value));
}

void PerlinNoise::SetOctaveCount(int octave_count) {
    m_octave_count = octave_count;
}

void PerlinNoise::SetFrequency(float frequency) {
    m_frequency = frequency;
}

void PerlinNoise::SetLacunarity(float lacunarity) {
    m_lacunarity = lacunarity;
}

void PerlinNoise::SetPersistence(float persistence) {
    m_persistence = persistence;
}

}  // namespace math

}  // namespace engine
