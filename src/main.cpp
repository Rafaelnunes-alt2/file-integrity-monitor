// feito por: maquinzz
#include "fim.hpp"
#include "sha256.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
namespace fs = std::filesystem;
namespace {
void usage() {
    std::cerr << "usage:\n";
    std::cerr << "  fim --hash <arquivo>\n";
    std::cerr << "  fim --init <dir> --db <baseline.txt>\n";
    std::cerr << "  fim --verify --db <baseline.txt> --root <dir>\n";
    std::cerr << "  fim --selftest\n";
}
bool getArg(int argc, char **argv, const std::string &k, std::string &v) {
    for (int i = 1; i + 1 < argc; ++i) {
        if (k == argv[i]) { v = argv[i + 1]; return true; }
    }
    return false;
}
bool hasArg(int argc, char **argv, const std::string &k) {
    for (int i = 1; i < argc; ++i) {
        if (k == argv[i]) return true;
    }
    return false;
}
int selftest() {
    const char *msg = "abc";
    fimsha::Sha256 s;
    s.update(reinterpret_cast<const std::uint8_t*>(msg), 3);
    std::string got = fimsha::hexOf(s.final());
    std::string want = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    if (got != want) {
        std::cout << "SELFTEST_FAIL sha256=" << got << "\n";
        return 2;
    }
    std::cout << "SELFTEST_OK sha256\n";
    return 0;
}
}
int main(int argc, char **argv) {
    if (hasArg(argc, argv, "--selftest")) return selftest();
    std::string hp;
    if (getArg(argc, argv, "--hash", hp)) {
        std::array<std::uint8_t, 32> d;
        std::string err;
        if (!fimsha::hashFile(hp, d, err)) { std::cerr << "hash_failed: " << err << "\n"; return 2; }
        std::cout << fimsha::hexOf(d) << "  " << hp << "\n";
        return 0;
    }
    std::string dir, db, root;
    if (hasArg(argc, argv, "--init") && getArg(argc, argv, "--init", dir) && getArg(argc, argv, "--db", db)) {
        std::vector<fim::Entry> v;
        std::string err;
        if (!fim::buildBaseline(dir, v, err)) { std::cerr << "init_failed: " << err << "\n"; return 2; }
        if (!fim::saveDb(db, v, err)) { std::cerr << "save_failed: " << err << "\n"; return 2; }
        std::cout << "files=" << v.size() << "\n";
        return 0;
    }
    if (hasArg(argc, argv, "--verify") && getArg(argc, argv, "--db", db) && getArg(argc, argv, "--root", root)) {
        std::vector<fim::Entry> base;
        std::string err;
        if (!fim::loadDb(db, base, err)) { std::cerr << "load_failed: " << err << "\n"; return 2; }
        std::vector<fim::Entry> cur;
        std::string dir2 = root;
        if (!fim::buildBaseline(dir2, cur, err)) { std::cerr << "scan_failed: " << err << "\n"; return 2; }
        std::map<std::string, fim::Entry> m;
        for (auto &e : cur) m[e.rel] = e;
        std::size_t ok = 0, changed = 0, missing = 0;
        for (auto &b : base) {
            auto it = m.find(b.rel);
            if (it == m.end()) {
                std::cout << "MISSING " << b.rel << "\n";
                ++missing;
                continue;
            }
            if (it->second.sha != b.sha) {
                std::cout << "CHANGED " << b.rel << "\n";
                ++changed;
            } else ++ok;
            m.erase(it);
        }
        for (auto &kv : m) std::cout << "NEW " << kv.first << "\n";
        std::size_t added = m.size();
        std::cout << "ok=" << ok << " changed=" << changed << " missing=" << missing << " new=" << added << "\n";
        return (changed == 0 && missing == 0 && added == 0) ? 0 : 3;
    }
    usage();
    return 1;
}
