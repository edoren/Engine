#include <Math/Noise/BaseNoise.hpp>

namespace engine {

namespace math {

BaseNoise::BaseNoise(int seed) : m_seed(seed) {
    generatePermutationVector();
}

BaseNoise::~BaseNoise() = default;

int BaseNoise::getSeed() const {
    return m_seed;
}

void BaseNoise::setSeed(int seed) {
    m_seed = seed;
    generatePermutationVector();
}

void BaseNoise::generatePermutationVector() {
    m_perm.resize(512);

    // Fill p with values from 0 to 255
    std::iota(m_perm.begin(), m_perm.begin() + 256, 0);

    // Initialize a random engine with seed
    std::default_random_engine engine(m_seed);

    // Suffle the values using the above random engine
    std::shuffle(m_perm.begin(), m_perm.begin() + 256, engine);

    // Duplicate the permutation vector
    std::copy(m_perm.begin(), m_perm.begin() + 256, m_perm.begin() + 256);
}

float BaseNoise::coherentNoise3D(float x, float y, float z) const {
    // Find the unit cube that contains the point
    int X = fastFloor(x) & 255;
    int Y = fastFloor(y) & 255;
    int Z = fastFloor(z) & 255;

    // Find relative x, y, z of point in cube
    x -= static_cast<float>(fastFloor(x));
    y -= static_cast<float>(fastFloor(y));
    z -= static_cast<float>(fastFloor(z));

    // Compute fade curves for each of x, y, z
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    // Hash coordinates of the 8 cube corners
    int AAA = m_perm[m_perm[m_perm[X] + Y] + Z];
    int ABA = m_perm[m_perm[m_perm[X] + Y + 1] + Z];
    int AAB = m_perm[m_perm[m_perm[X] + Y] + Z + 1];
    int ABB = m_perm[m_perm[m_perm[X] + Y + 1] + Z + 1];
    int BAA = m_perm[m_perm[m_perm[X + 1] + Y] + Z];
    int BBA = m_perm[m_perm[m_perm[X + 1] + Y + 1] + Z];
    int BAB = m_perm[m_perm[m_perm[X + 1] + Y] + Z + 1];
    int BBB = m_perm[m_perm[m_perm[X + 1] + Y + 1] + Z + 1];

    // Calculate noise contributions from each of the eight corners
    float n000 = grad(AAA, x, y, z);
    float n100 = grad(BAA, x - 1, y, z);
    float n010 = grad(ABA, x, y - 1, z);
    float n110 = grad(BBA, x - 1, y - 1, z);
    float n001 = grad(AAB, x, y, z - 1);
    float n101 = grad(BAB, x - 1, y, z - 1);
    float n011 = grad(ABB, x, y - 1, z - 1);
    float n111 = grad(BBB, x - 1, y - 1, z - 1);

    // Add blended results from 8 corners of cube

    // Interpolate along x the contributions from each of the corners
    float nx00 = lerp(u, n000, n100);
    float nx01 = lerp(u, n001, n101);
    float nx10 = lerp(u, n010, n110);
    float nx11 = lerp(u, n011, n111);

    // Interpolate the four results along y
    float nxy0 = lerp(v, nx00, nx10);
    float nxy1 = lerp(v, nx01, nx11);

    // Interpolate the two last results along z
    float nxyz = lerp(w, nxy0, nxy1);

    return std::min(1.F, std::max(-1.F, nxyz));
}

float BaseNoise::grad(int hash, float x, float y, float z) const {
    switch (hash & 0xF) {
        case 0x0:
            return x + y;
        case 0x1:
            return -x + y;
        case 0x2:
            return x - y;
        case 0x3:
            return -x - y;
        case 0x4:
            return x + z;
        case 0x5:
            return -x + z;
        case 0x6:
            return x - z;
        case 0x7:
            return -x - z;
        case 0x8:
            return y + z;
        case 0x9:
            return -y + z;
        case 0xA:
            return y - z;
        case 0xB:
            return -y - z;
        case 0xC:
            return y + x;
        case 0xD:
            return -y + z;
        case 0xE:
            return y - x;
        case 0xF:
            return -y - z;
        default:
            return 0;  // never happens
    }
}

float BaseNoise::fade(float t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

int BaseNoise::fastFloor(float x) const {
    return x > 0.F ? static_cast<int>(x) : static_cast<int>(x - 1);
}

float BaseNoise::lerp(float t, float a, float b) const {
    return a + t * (b - a);
}

}  // namespace math

}  // namespace engine
