#include <Math/Noise/RidgedMultiNoise.hpp>

#include <algorithm>

namespace engine::math {

const int RidgedMultiNoise::sDefaultSeed = 0;
const int RidgedMultiNoise::sDefaultOctaveCount = 6;
const float RidgedMultiNoise::sDefaultFrequency = 1.0F;
const float RidgedMultiNoise::sDefaultLacunarity = 2.0F;
const float RidgedMultiNoise::sDefaultOffset = 1.0F;
const float RidgedMultiNoise::sDefaultGain = 2.0F;
const float RidgedMultiNoise::sDefaultExponent = 1.0F;
const int RidgedMultiNoise::sMaxOctaveCount = 30;

RidgedMultiNoise::RidgedMultiNoise() : RidgedMultiNoise(sDefaultSeed) {}

RidgedMultiNoise::RidgedMultiNoise(int seed)
      : BaseNoise(seed),
        m_octaveCount(sDefaultOctaveCount),
        m_frequency(sDefaultFrequency),
        m_lacunarity(sDefaultLacunarity),
        m_offset(sDefaultOffset),
        m_gain(sDefaultGain),
        m_exponent(sDefaultExponent) {
    calcSpectralWeights();
}

int RidgedMultiNoise::getOctaveCount() const {
    return m_octaveCount;
}

float RidgedMultiNoise::getFrequency() const {
    return m_frequency;
}

float RidgedMultiNoise::getLacunarity() const {
    return m_lacunarity;
}

float RidgedMultiNoise::getOffset() const {
    return m_offset;
}

float RidgedMultiNoise::getGain() const {
    return m_gain;
}

float RidgedMultiNoise::getExponent() const {
    return m_exponent;
}

float RidgedMultiNoise::getValue(float x, float y, float z) const {
    float value = 0.F;
    float signal = 0.F;
    float frequency = m_frequency;
    float weight = 1.F;

    for (int currentOctave = 0; currentOctave < m_octaveCount; currentOctave++) {
        signal = coherentNoise3D(x * frequency, y * frequency, z * frequency);

        signal = m_offset - std::abs(signal);
        signal *= signal;
        signal *= weight;

        weight = signal * m_gain;
        weight = std::max(0.F, std::min(1.F, weight));

        value += signal * m_spectralWeights[currentOctave];

        frequency *= m_lacunarity;
    }

    return std::min(1.F, std::max(-1.F, (value * 1.25F) - 1.0F));
}

void RidgedMultiNoise::setOctaveCount(int octaveCount) {
    m_octaveCount = octaveCount;
}

void RidgedMultiNoise::setFrequency(float frequency) {
    m_frequency = frequency;
    calcSpectralWeights();
}

void RidgedMultiNoise::setLacunarity(float lacunarity) {
    m_lacunarity = lacunarity;
    calcSpectralWeights();
}

void RidgedMultiNoise::setOffset(float offset) {
    m_offset = offset;
}

void RidgedMultiNoise::setGain(float gain) {
    m_gain = gain;
}

void RidgedMultiNoise::setExponent(float exponent) {
    m_exponent = exponent;
    calcSpectralWeights();
}

void RidgedMultiNoise::calcSpectralWeights() {
    m_spectralWeights.clear();
    m_spectralWeights.reserve(sMaxOctaveCount);

    float frequency = m_frequency;
    for (int i = 0; i < sMaxOctaveCount; i++) {
        m_spectralWeights.push_back(std::pow(frequency, -m_exponent));
        frequency *= m_lacunarity;
    }
}

}  // namespace engine::math
