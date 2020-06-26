#include <Math/Noise/BaseNoise.hpp>

#include <numeric>
#include <random>

namespace engine::math {

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

float BaseNoise::coherentNoise3D(float xCoord, float yCoord, float zCoord) const {
    // Find the unit cube that contains the point
    int x = fastFloor(xCoord) & 255;
    int y = fastFloor(yCoord) & 255;
    int z = fastFloor(zCoord) & 255;

    // Find relative x, y, z of point in cube
    xCoord -= static_cast<float>(fastFloor(xCoord));
    yCoord -= static_cast<float>(fastFloor(yCoord));
    zCoord -= static_cast<float>(fastFloor(zCoord));

    // Compute fade curves for each of x, y, z
    float u = fade(xCoord);
    float v = fade(yCoord);
    float w = fade(zCoord);

    // Hash coordinates of the 8 cube corners
    int aaa = m_perm[m_perm[m_perm[x] + y] + z];
    int aba = m_perm[m_perm[m_perm[x] + y + 1] + z];
    int aab = m_perm[m_perm[m_perm[x] + y] + z + 1];
    int abb = m_perm[m_perm[m_perm[x] + y + 1] + z + 1];
    int baa = m_perm[m_perm[m_perm[x + 1] + y] + z];
    int bba = m_perm[m_perm[m_perm[x + 1] + y + 1] + z];
    int bab = m_perm[m_perm[m_perm[x + 1] + y] + z + 1];
    int bbb = m_perm[m_perm[m_perm[x + 1] + y + 1] + z + 1];

    // Calculate noise contributions from each of the eight corners
    float n000 = grad(aaa, xCoord, yCoord, zCoord);
    float n100 = grad(baa, xCoord - 1, yCoord, zCoord);
    float n010 = grad(aba, xCoord, yCoord - 1, zCoord);
    float n110 = grad(bba, xCoord - 1, yCoord - 1, zCoord);
    float n001 = grad(aab, xCoord, yCoord, zCoord - 1);
    float n101 = grad(bab, xCoord - 1, yCoord, zCoord - 1);
    float n011 = grad(abb, xCoord, yCoord - 1, zCoord - 1);
    float n111 = grad(bbb, xCoord - 1, yCoord - 1, zCoord - 1);

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

}  // namespace engine::math
