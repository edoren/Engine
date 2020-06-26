#pragma once

#include <Util/Prerequisites.hpp>

#include <Util/Container/Vector.hpp>

namespace engine {

namespace math {

class ENGINE_API BaseNoise {
public:
    BaseNoise(int seed);

    virtual ~BaseNoise();

    int getSeed() const;

    virtual float getValue(float x, float y, float z) const = 0;

    void setSeed(int seed);

    float coherentNoise3D(float x, float y, float z) const;

private:
    void generatePermutationVector();

    float grad(int hash, float x, float y, float z) const;

    float fade(float t) const;

    int fastFloor(float x) const;

    float lerp(float t, float a, float b) const;

    int m_seed;
    Vector<int> m_perm;
};

}  // namespace math

}  // namespace engine
