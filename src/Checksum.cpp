#include "Checksum.h"
#include <array>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstring>

namespace {

class SHA256 {
public:
    SHA256() { reset(); }

    void update(const unsigned char* data, std::size_t len) {
        for (std::size_t i = 0; i < len; ++i) {
            buffer_[bufferLen_++] = data[i];

            if (bufferLen_ == 64) {
                transform();
                bitLen_ += 512;
                bufferLen_ = 0;
            }
        }
    }

    std::string finalHex() {
        std::size_t i = bufferLen_;

        if (bufferLen_ < 56) {
            buffer_[i++] = 0x80;
            while (i < 56) buffer_[i++] = 0x00;
        } else {
            buffer_[i++] = 0x80;
            while (i < 64) buffer_[i++] = 0x00;
            transform();
            std::memset(buffer_.data(), 0, 56);
        }

        bitLen_ += static_cast<uint64_t>(bufferLen_) * 8;

        buffer_[63] = static_cast<unsigned char>(bitLen_);
        buffer_[62] = static_cast<unsigned char>(bitLen_ >> 8);
        buffer_[61] = static_cast<unsigned char>(bitLen_ >> 16);
        buffer_[60] = static_cast<unsigned char>(bitLen_ >> 24);
        buffer_[59] = static_cast<unsigned char>(bitLen_ >> 32);
        buffer_[58] = static_cast<unsigned char>(bitLen_ >> 40);
        buffer_[57] = static_cast<unsigned char>(bitLen_ >> 48);
        buffer_[56] = static_cast<unsigned char>(bitLen_ >> 56);
        transform();

        std::ostringstream oss;
        for (int j = 0; j < 8; ++j) {
            oss << std::hex << std::setw(8) << std::setfill('0') << state_[j];
        }
        return oss.str();
    }

private:
    std::array<unsigned char, 64> buffer_{};
    std::array<uint32_t, 8> state_{};
    std::size_t bufferLen_ = 0;
    uint64_t bitLen_ = 0;

    static uint32_t rotr(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }

    static uint32_t choose(uint32_t e, uint32_t f, uint32_t g) {
        return (e & f) ^ (~e & g);
    }

    static uint32_t majority(uint32_t a, uint32_t b, uint32_t c) {
        return (a & b) ^ (a & c) ^ (b & c);
    }

    static uint32_t sig0(uint32_t x) {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }

    static uint32_t sig1(uint32_t x) {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    static uint32_t ep0(uint32_t x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }

    static uint32_t ep1(uint32_t x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }

    void reset() {
        state_ = {
            0x6a09e667,
            0xbb67ae85,
            0x3c6ef372,
            0xa54ff53a,
            0x510e527f,
            0x9b05688c,
            0x1f83d9ab,
            0x5be0cd19
        };
        bufferLen_ = 0;
        bitLen_ = 0;
        std::memset(buffer_.data(), 0, buffer_.size());
    }

    void transform() {
        static const uint32_t k[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        uint32_t m[64];
        for (int i = 0, j = 0; i < 16; ++i, j += 4) {
            m[i] = (static_cast<uint32_t>(buffer_[j]) << 24) |
                   (static_cast<uint32_t>(buffer_[j + 1]) << 16) |
                   (static_cast<uint32_t>(buffer_[j + 2]) << 8) |
                   (static_cast<uint32_t>(buffer_[j + 3]));
        }

        for (int i = 16; i < 64; ++i) {
            m[i] = sig1(m[i - 2]) + m[i - 7] + sig0(m[i - 15]) + m[i - 16];
        }

        uint32_t a = state_[0];
        uint32_t b = state_[1];
        uint32_t c = state_[2];
        uint32_t d = state_[3];
        uint32_t e = state_[4];
        uint32_t f = state_[5];
        uint32_t g = state_[6];
        uint32_t h = state_[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t t1 = h + ep1(e) + choose(e, f, g) + k[i] + m[i];
            uint32_t t2 = ep0(a) + majority(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + t1;
            d = c;
            c = b;
            b = a;
            a = t1 + t2;
        }

        state_[0] += a;
        state_[1] += b;
        state_[2] += c;
        state_[3] += d;
        state_[4] += e;
        state_[5] += f;
        state_[6] += g;
        state_[7] += h;
    }
};

} // namespace

std::string Checksum::sha256File(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    SHA256 sha;
    std::vector<unsigned char> buffer(8 * 1024 * 1024);

    while (file) {
        file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
        std::streamsize got = file.gcount();
        if (got > 0) {
            sha.update(buffer.data(), static_cast<std::size_t>(got));
        }
    }

    if (file.bad()) {
        return "";
    }

    return sha.finalHex();
}