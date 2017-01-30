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
        octave_count_(DEFAULT_OCTAVE_COUNT),
        frequency_(DEFAULT_FREQUENCY),
        lacunarity_(DEFAULT_LACUNARITY),
        persistence_(DEFAULT_PERSISTENCE) {}

int PerlinNoise::GetOctaveCount() const {
    return octave_count_;
}

float PerlinNoise::GetFrequency() const {
    return frequency_;
}

float PerlinNoise::GetLacunarity() const {
    return lacunarity_;
}

float PerlinNoise::GetPersistence() const {
    return persistence_;
}

float PerlinNoise::GetValue(float x, float y, float z) const {
    float value = 0.f;
    float signal = 0.f;
    float frequency = frequency_;
    float amplitude = 1.f;

    for (int current_octave = 0; current_octave < octave_count_;
         current_octave++) {
        signal = CoherentNoise3D(x * frequency, y * frequency, z * frequency);
        value += signal * amplitude;

        frequency *= lacunarity_;
        amplitude *= persistence_;
    }

    return std::min(1.f, std::max(-1.f, value));
}

void PerlinNoise::SetOctaveCount(int octave_count) {
    octave_count_ = octave_count;
}

void PerlinNoise::SetFrequency(float frequency) {
    frequency_ = frequency;
}

void PerlinNoise::SetLacunarity(float lacunarity) {
    lacunarity_ = lacunarity;
}

void PerlinNoise::SetPersistence(float persistence) {
    persistence_ = persistence;
}

}  // namespace math

}  // namespace engine
