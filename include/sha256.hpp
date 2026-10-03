// feito por: maquinzz
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
namespace fimsha {
struct Sha256 {
    std::uint32_t h[8];
    std::uint64_t total = 0;
    std::uint8_t buf[64];
    std::size_t used = 0;
    Sha256();
    void update(const std::uint8_t *p, std::size_t n);
    std::array<std::uint8_t, 32> final();
};
std::string hexOf(const std::array<std::uint8_t, 32> &d);
bool hashFile(const std::string &path, std::array<std::uint8_t, 32> &out, std::string &err);
}
