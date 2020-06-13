#pragma once

#include <Util/Prerequisites.hpp>

#include <Math/Noise/BaseNoise.hpp>

namespace engine {

namespace math {

class ENGINE_API PerlinNoise : public BaseNoise {
public:
    static const int sDefaultSeed;
    static const int sDefaultOctaveCount;
    static const float sDefaultFrequency;
    static const float sDefaultLacunarity;
    static const float sDefaultPersistence;
    static const int sMaxOctaveCount;

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
    int m_octave_count;
    float m_frequency;
    float m_lacunarity;
    float m_persistence;
};

}  // namespace math

}  // namespace engine
