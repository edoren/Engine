#pragma once

#include <algorithm>
#include <vector>

#include "BaseNoise.hpp"

namespace engine {

namespace math {

class RidgedMultiNoise : public BaseNoise {
public:
    static const int DEFAULT_SEED;
    static const int DEFAULT_OCTAVE_COUNT;
    static const float DEFAULT_FREQUENCY;
    static const float DEFAULT_LACUNARITY;
    static const float DEFAULT_OFFSET;
    static const float DEFAULT_GAIN;
    static const float DEFAULT_EXPONENT;
    static const int MAX_OCTAVE_COUNT;

public:
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

private:
    int octave_count_;
    float frequency_;
    float lacunarity_;
    float offset_;
    float gain_;
    float exponent_;

    std::vector<float> spectral_weights_;
};

}  // namespace math

}  // namespace engine
