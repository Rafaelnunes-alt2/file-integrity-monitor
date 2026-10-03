// feito por: maquinzz
#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace fim {
struct Entry {
    std::string rel;
    std::string sha;
    std::uint64_t size = 0;
};
constexpr std::size_t kMaxFiles = 100000;
bool buildBaseline(const std::string &root, std::vector<Entry> &out, std::string &err);
bool saveDb(const std::string &db, const std::vector<Entry> &v, std::string &err);
bool loadDb(const std::string &db, std::vector<Entry> &v, std::string &err);
}
