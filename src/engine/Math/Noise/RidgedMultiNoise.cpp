#include <Math/Noise/RidgedMultiNoise.hpp>

namespace engine {

namespace math {

const int RidgedMultiNoise::DEFAULT_SEED = 0;
const int RidgedMultiNoise::DEFAULT_OCTAVE_COUNT = 6;
const float RidgedMultiNoise::DEFAULT_FREQUENCY = 1.0F;
const float RidgedMultiNoise::DEFAULT_LACUNARITY = 2.0F;
const float RidgedMultiNoise::DEFAULT_OFFSET = 1.0F;
const float RidgedMultiNoise::DEFAULT_GAIN = 2.0F;
const float RidgedMultiNoise::DEFAULT_EXPONENT = 1.0F;
const int RidgedMultiNoise::MAX_OCTAVE_COUNT = 30;

RidgedMultiNoise::RidgedMultiNoise() : RidgedMultiNoise(DEFAULT_SEED) {}

RidgedMultiNoise::RidgedMultiNoise(int seed)
      : BaseNoise(seed),
        m_octave_count(DEFAULT_OCTAVE_COUNT),
        m_frequency(DEFAULT_FREQUENCY),
        m_lacunarity(DEFAULT_LACUNARITY),
        m_offset(DEFAULT_OFFSET),
        m_gain(DEFAULT_GAIN),
        m_exponent(DEFAULT_EXPONENT) {
    CalcSpectralWeights();
}

int RidgedMultiNoise::GetOctaveCount() const {
    return m_octave_count;
}

float RidgedMultiNoise::GetFrequency() const {
    return m_frequency;
}

float RidgedMultiNoise::GetLacunarity() const {
    return m_lacunarity;
}

float RidgedMultiNoise::GetOffset() const {
    return m_offset;
}

float RidgedMultiNoise::GetGain() const {
    return m_gain;
}

float RidgedMultiNoise::GetExponent() const {
    return m_exponent;
}

float RidgedMultiNoise::GetValue(float x, float y, float z) const {
    float value = 0.F;
    float signal = 0.F;
    float frequency = m_frequency;
    float weight = 1.F;

    for (int current_octave = 0; current_octave < m_octave_count; current_octave++) {
        signal = CoherentNoise3D(x * frequency, y * frequency, z * frequency);

        signal = m_offset - std::abs(signal);
        signal *= signal;
        signal *= weight;

        weight = signal * m_gain;
        weight = std::max(0.F, std::min(1.F, weight));

        value += signal * m_spectral_weights[current_octave];

        frequency *= m_lacunarity;
    }

    return std::min(1.F, std::max(-1.F, (value * 1.25F) - 1.0F));
}

void RidgedMultiNoise::SetOctaveCount(int octave_count) {
    m_octave_count = octave_count;
}

void RidgedMultiNoise::SetFrequency(float frequency) {
    m_frequency = frequency;
    CalcSpectralWeights();
}

void RidgedMultiNoise::SetLacunarity(float lacunarity) {
    m_lacunarity = lacunarity;
    CalcSpectralWeights();
}

void RidgedMultiNoise::SetOffset(float offset) {
    m_offset = offset;
}

void RidgedMultiNoise::SetGain(float gain) {
    m_gain = gain;
}

void RidgedMultiNoise::SetExponent(float exponent) {
    m_exponent = exponent;
    CalcSpectralWeights();
}

void RidgedMultiNoise::CalcSpectralWeights() {
    m_spectral_weights.clear();
    m_spectral_weights.reserve(MAX_OCTAVE_COUNT);

    float frequency = m_frequency;
    for (int i = 0; i < MAX_OCTAVE_COUNT; i++) {
        m_spectral_weights.push_back(std::pow(frequency, -m_exponent));
        frequency *= m_lacunarity;
    }
}

}  // namespace math

}  // namespace engine
