// feito por: maquinzz
#include "sha256.hpp"
#include <fstream>
namespace fimsha {
namespace {
constexpr std::uint32_t K[64] = {
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, 0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,     0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
};
std::uint32_t rotr(std::uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}
void block(Sha256 &s, const std::uint8_t *p) {
    std::uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<std::uint32_t>(p[i * 4]) << 24) | (static_cast<std::uint32_t>(p[i * 4 + 1]) << 16) | (static_cast<std::uint32_t>(p[i * 4 + 2]) << 8) | p[i * 4 + 3];
    }
    for (int i = 16; i < 64; ++i) {
        std::uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        std::uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    std::uint32_t a = s.h[0], b = s.h[1], c = s.h[2], d = s.h[3];
    std::uint32_t e = s.h[4], f = s.h[5], g = s.h[6], h = s.h[7];
    for (int i = 0; i < 64; ++i) {
        std::uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        std::uint32_t ch = (e & f) ^ ((~e) & g);
        std::uint32_t t1 = h + S1 + ch + K[i] + w[i];
        std::uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        std::uint32_t mj = (a & b) ^ (a & c) ^ (b & c);
        std::uint32_t t2 = S0 + mj;
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }
    s.h[0] += a;
    s.h[1] += b;
    s.h[2] += c;
    s.h[3] += d;
    s.h[4] += e;
    s.h[5] += f;
    s.h[6] += g;
    s.h[7] += h;
}
}
Sha256::Sha256() {
    h[0] = 0x6a09e667U;
    h[1] = 0xbb67ae85U;
    h[2] = 0x3c6ef372U;
    h[3] = 0xa54ff53aU;
    h[4] = 0x510e527fU;
    h[5] = 0x9b05688cU;
    h[6] = 0x1f83d9abU;
    h[7] = 0x5be0cd19U;
}
void Sha256::update(const std::uint8_t *p, std::size_t n) {
    if (!p && n > 0) return;
    total += n;
    while (n > 0) {
        std::size_t take = 64 - used;
        if (take > n) take = n;
        for (std::size_t i = 0; i < take; ++i) buf[used + i] = p[i];
        used += take;
        p += take;
        n -= take;
        if (used == 64) {
            block(*this, buf);
            used = 0;
        }
    }
}
std::array<std::uint8_t, 32> Sha256::final() {
    std::uint64_t bits = total * 8;
    std::uint8_t one = 0x80;
    update(&one, 1);
    std::uint8_t zero = 0x00;
    while (used != 56) update(&zero, 1);
    std::uint8_t len[8];
    for (int i = 0; i < 8; ++i) len[i] = static_cast<std::uint8_t>((bits >> (56 - i * 8)) & 0xFF);
    update(len, 8);
    std::array<std::uint8_t, 32> d;
    for (int i = 0; i < 8; ++i) {
        d[i * 4] = static_cast<std::uint8_t>((h[i] >> 24) & 0xFF);
        d[i * 4 + 1] = static_cast<std::uint8_t>((h[i] >> 16) & 0xFF);
        d[i * 4 + 2] = static_cast<std::uint8_t>((h[i] >> 8) & 0xFF);
        d[i * 4 + 3] = static_cast<std::uint8_t>(h[i] & 0xFF);
    }
    return d;
}
std::string hexOf(const std::array<std::uint8_t, 32> &d) {
    static const char *H = "0123456789abcdef";
    std::string o;
    o.reserve(64);
    for (auto b : d) {
        o.push_back(H[(b >> 4) & 0xF]);
        o.push_back(H[b & 0xF]);
    }
    return o;
}
bool hashFile(const std::string &path, std::array<std::uint8_t, 32> &out, std::string &err) {
    if (path.empty() || path.size() > 4096) { err = "bad_path"; return false; }
    std::ifstream f(path, std::ios::binary);
    if (!f) { err = "open_failed"; return false; }
    Sha256 s;
    std::uint8_t b[65536];
    std::uint64_t done = 0;
    constexpr std::uint64_t kLim = 512ULL * 1024ULL * 1024ULL;
    while (f) {
        f.read(reinterpret_cast<char*>(b), sizeof(b));
        std::streamsize g = f.gcount();
        if (g <= 0) break;
        done += static_cast<std::uint64_t>(g);
        if (done > kLim) { err = "too_big"; return false; }
        s.update(b, static_cast<std::size_t>(g));
    }
    if (f.bad()) { err = "read_failed"; return false; }
    out = s.final();
    return true;
}
}
