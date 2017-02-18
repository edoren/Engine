#pragma once

#include <Util/Prerequisites.hpp>

namespace engine {

namespace math {

class ENGINE_API BaseNoise {
public:
    BaseNoise(int seed);

    virtual ~BaseNoise();

    int GetSeed() const;

    virtual float GetValue(float x, float y, float z) const = 0;

    void SetSeed(int seed);

    float CoherentNoise3D(float x, float y, float z) const;

private:
    void GeneratePermutationVector();

    float Grad(int hash, float x, float y, float z) const;

    float Fade(float t) const;

    int FastFloor(float x) const;

    float Lerp(float a, float b, float t) const;

private:
    int seed_;
    std::vector<int> perm_;
};

}  // namespace math

}  // namespace engine
