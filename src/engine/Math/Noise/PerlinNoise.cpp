#include <Math/Noise/PerlinNoise.hpp>

namespace engine {

namespace math {

const int PerlinNoise::DEFAULT_SEED = 0;
const int PerlinNoise::DEFAULT_OCTAVE_COUNT = 6;
const float PerlinNoise::DEFAULT_FREQUENCY = 1.0f;
const float PerlinNoise::DEFAULT_LACUNARITY = 2.0f;
const float PerlinNoise::DEFAULT_PERSISTENCE = 0.5f;
const int PerlinNoise::MAX_OCTAVE_COUNT = 30;

PerlinNoise::PerlinNoise() : PerlinNoise(DEFAULT_SEED) {}

PerlinNoise::PerlinNoise(int seed)
      : BaseNoise(seed),
        m_octave_count(DEFAULT_OCTAVE_COUNT),
        m_frequency(DEFAULT_FREQUENCY),
        m_lacunarity(DEFAULT_LACUNARITY),
        m_persistence(DEFAULT_PERSISTENCE) {}

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
    float value = 0.f;
    float signal = 0.f;
    float frequency = m_frequency;
    float amplitude = 1.f;

    for (int current_octave = 0; current_octave < m_octave_count; current_octave++) {
        signal = CoherentNoise3D(x * frequency, y * frequency, z * frequency);
        value += signal * amplitude;

        frequency *= m_lacunarity;
        amplitude *= m_persistence;
    }

    return std::min(1.f, std::max(-1.f, value));
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
