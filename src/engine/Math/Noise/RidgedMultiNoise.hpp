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

    int GetOctaveCount() const;

    float GetFrequency() const;

    float GetLacunarity() const;

    float GetOffset() const;

    float GetGain() const;

    float GetExponent() const;

    float GetValue(float x, float y, float z) const;

    void SetOctaveCount(int octave_count);

    void SetFrequency(float frequency);

    void SetLacunarity(float lacunarity);

    void SetOffset(float offset);

    void SetGain(float gain);

    void SetExponent(float exponent);

private:
    void CalcSpectralWeights();

    int m_octave_count;
    float m_frequency;
    float m_lacunarity;
    float m_offset;
    float m_gain;
    float m_exponent;

    std::vector<float> m_spectral_weights;
};

}  // namespace math

}  // namespace engine
