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

    int getOctaveCount() const;

    float getFrequency() const;

    float getLacunarity() const;

    float getPersistence() const;

    float getValue(float x, float y, float z) const override;

    void setOctaveCount(int octave_count);

    void setFrequency(float frequency);

    void setLacunarity(float lacunarity);

    void setPersistence(float persistence);

private:
    int m_octaveCount;
    float m_frequency;
    float m_lacunarity;
    float m_persistence;
};

}  // namespace math

}  // namespace engine
