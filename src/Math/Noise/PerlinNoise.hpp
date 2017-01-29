#pragma once

#include <Math/Noise/BaseNoise.hpp>

namespace engine {

namespace math {

class ENGINE_API PerlinNoise : public BaseNoise {
public:
    static const int DEFAULT_SEED;
    static const int DEFAULT_OCTAVE_COUNT;
    static const float DEFAULT_FREQUENCY;
    static const float DEFAULT_LACUNARITY;
    static const float DEFAULT_PERSISTENCE;
    static const int MAX_OCTAVE_COUNT;

public:
    PerlinNoise();

    PerlinNoise(int seed);

    int GetOctaveCount() const;

    float GetFrequency() const;

    float GetLacunarity() const;

    float GetPersistence() const;

    float GetValue(float x, float y, float z) const;

    void SetOctaveCount(int octave_count);

    void SetFrequency(float frequency);

    void SetLacunarity(float lacunarity);

    void SetPersistence(float persistence);

private:
    int octave_count_;
    float frequency_;
    float lacunarity_;
    float persistence_;
};

}  // namespace math

}  // namespace engine
