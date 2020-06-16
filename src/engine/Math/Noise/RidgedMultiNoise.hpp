#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Noise/BaseNoise.hpp>

namespace engine {

namespace math {

class ENGINE_API RidgedMultiNoise : public BaseNoise {
public:
    static const int sDefaultSeed;
    static const int sDefaultOctaveCount;
    static const float sDefaultFrequency;
    static const float sDefaultLacunarity;
    static const float sDefaultOffset;
    static const float sDefaultGain;
    static const float sDefaultExponent;
    static const int sMaxOctaveCount;

    RidgedMultiNoise();

    RidgedMultiNoise(int seed);

    int getOctaveCount() const;

    float getFrequency() const;

    float getLacunarity() const;

    float getOffset() const;

    float getGain() const;

    float getExponent() const;

    float getValue(float x, float y, float z) const override;

    void setOctaveCount(int octave_count);

    void setFrequency(float frequency);

    void setLacunarity(float lacunarity);

    void setOffset(float offset);

    void setGain(float gain);

    void setExponent(float exponent);

private:
    void calcSpectralWeights();

    int m_octaveCount;
    float m_frequency;
    float m_lacunarity;
    float m_offset;
    float m_gain;
    float m_exponent;

    std::vector<float> m_spectralWeights;
};

}  // namespace math

}  // namespace engine
