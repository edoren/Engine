#include <Math/Noise/RidgedMultiNoise.hpp>

namespace engine {

namespace math {

const int RidgedMultiNoise::DEFAULT_SEED = 0;
const int RidgedMultiNoise::DEFAULT_OCTAVE_COUNT = 6;
const float RidgedMultiNoise::DEFAULT_FREQUENCY = 1.0f;
const float RidgedMultiNoise::DEFAULT_LACUNARITY = 2.0f;
const float RidgedMultiNoise::DEFAULT_OFFSET = 1.0f;
const float RidgedMultiNoise::DEFAULT_GAIN = 2.0f;
const float RidgedMultiNoise::DEFAULT_EXPONENT = 1.0f;
const int RidgedMultiNoise::MAX_OCTAVE_COUNT = 30;

RidgedMultiNoise::RidgedMultiNoise() : RidgedMultiNoise(DEFAULT_SEED) {}

RidgedMultiNoise::RidgedMultiNoise(int seed)
      : BaseNoise(seed),
        octave_count_(DEFAULT_OCTAVE_COUNT),
        frequency_(DEFAULT_FREQUENCY),
        lacunarity_(DEFAULT_LACUNARITY),
        offset_(DEFAULT_OFFSET),
        gain_(DEFAULT_GAIN),
        exponent_(DEFAULT_EXPONENT) {
    CalcSpectralWeights();
}

int RidgedMultiNoise::GetOctaveCount() const {
    return octave_count_;
}

float RidgedMultiNoise::GetFrequency() const {
    return frequency_;
}

float RidgedMultiNoise::GetLacunarity() const {
    return lacunarity_;
}

float RidgedMultiNoise::GetOffset() const {
    return offset_;
}

float RidgedMultiNoise::GetGain() const {
    return gain_;
}

float RidgedMultiNoise::GetExponent() const {
    return exponent_;
}

float RidgedMultiNoise::GetValue(float x, float y, float z) const {
    float value = 0.f;
    float signal = 0.f;
    float frequency = frequency_;
    float weight = 1.f;

    for (int current_octave = 0; current_octave < octave_count_;
         current_octave++) {
        signal = CoherentNoise3D(x * frequency, y * frequency, z * frequency);

        signal = offset_ - std::abs(signal);
        signal *= signal;
        signal *= weight;

        weight = signal * gain_;
        weight = std::max(0.f, std::min(1.f, weight));

        value += signal * spectral_weights_[current_octave];

        frequency *= lacunarity_;
    }

    return std::min(1.f, std::max(-1.f, (value * 1.25f) - 1.0f));
}

void RidgedMultiNoise::SetOctaveCount(int octave_count) {
    octave_count_ = octave_count;
}

void RidgedMultiNoise::SetFrequency(float frequency) {
    frequency_ = frequency;
    CalcSpectralWeights();
}

void RidgedMultiNoise::SetLacunarity(float lacunarity) {
    lacunarity_ = lacunarity;
    CalcSpectralWeights();
}

void RidgedMultiNoise::SetOffset(float offset) {
    offset_ = offset;
}

void RidgedMultiNoise::SetGain(float gain) {
    gain_ = gain;
}

void RidgedMultiNoise::SetExponent(float exponent) {
    exponent_ = exponent;
    CalcSpectralWeights();
}

void RidgedMultiNoise::CalcSpectralWeights() {
    spectral_weights_.clear();
    spectral_weights_.reserve(MAX_OCTAVE_COUNT);

    float frequency = frequency_;
    for (int i = 0; i < MAX_OCTAVE_COUNT; i++) {
        spectral_weights_.push_back(std::pow(frequency, -exponent_));
        frequency *= lacunarity_;
    }
}

}  // namespace math

}  // namespace engine
