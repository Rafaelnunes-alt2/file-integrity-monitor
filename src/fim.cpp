// feito por: maquinzz
#include "fim.hpp"
#include "sha256.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
namespace fim {
namespace fs = std::filesystem;
bool buildBaseline(const std::string &root, std::vector<Entry> &out, std::string &err) {
    out.clear();
    if (root.empty() || root.size() > 4096) { err = "bad_root"; return false; }
    std::error_code ec;
    fs::path rp = fs::weakly_canonical(fs::path(root), ec);
    if (ec) { err = "bad_root"; return false; }
    if (!fs::is_directory(rp, ec) || ec) { err = "not_a_dir"; return false; }
    std::vector<fs::path> files;
    for (auto it = fs::recursive_directory_iterator(rp, fs::directory_options::skip_permission_denied, ec);
         it != fs::recursive_directory_iterator(); ++it) {
        if (ec) break;
        std::error_code e2;
        if (it->is_symlink(e2)) continue;
        if (!it->is_regular_file(e2) || e2) continue;
        files.push_back(it->path());
        if (files.size() > kMaxFiles) { err = "too_many_files"; return false; }
    }
    std::sort(files.begin(), files.end());
    for (auto &p : files) {
        std::error_code e2;
        fs::path rel = fs::relative(p, rp, e2);
        if (e2) continue;
        std::string rs = rel.generic_string();
        if (rs.empty() || rs.size() > 2048) continue;
        if (rs.find("..") != std::string::npos) continue;
        std::array<std::uint8_t, 32> d;
        std::string he;
        if (!fimsha::hashFile(p.string(), d, he)) continue;
        std::uint64_t sz = 0;
        sz = static_cast<std::uint64_t>(fs::file_size(p, e2));
        if (e2) continue;
        Entry e;
        e.rel = rs;
        e.sha = fimsha::hexOf(d);
        e.size = sz;
        out.push_back(e);
    }
    return true;
}
bool saveDb(const std::string &db, const std::vector<Entry> &v, std::string &err) {
    if (db.empty() || db.size() > 4096) { err = "bad_db"; return false; }
    std::ofstream o(db, std::ios::binary | std::ios::trunc);
    if (!o) { err = "open_failed"; return false; }
    for (auto &e : v) {
        if (e.rel.find('\n') != std::string::npos || e.rel.find('\t') != std::string::npos) continue;
        o << e.sha << '\t' << e.size << '\t' << e.rel << '\n';
        if (!o) { err = "write_failed"; return false; }
    }
    return true;
}
bool loadDb(const std::string &db, std::vector<Entry> &v, std::string &err) {
    v.clear();
    if (db.empty() || db.size() > 4096) { err = "bad_db"; return false; }
    std::ifstream f(db, std::ios::binary | std::ios::ate);
    if (!f) { err = "open_failed"; return false; }
    std::streamsize sz = f.tellg();
    if (sz < 0 || sz > 64 * 1024 * 1024) { err = "bad_db_size"; return false; }
    f.seekg(0, std::ios::beg);
    std::string line;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        std::size_t t1 = line.find('\t');
        if (t1 == std::string::npos) continue;
        std::size_t t2 = line.find('\t', t1 + 1);
        if (t2 == std::string::npos) continue;
        std::string sha = line.substr(0, t1);
        std::string szs = line.substr(t1 + 1, t2 - t1 - 1);
        std::string rel = line.substr(t2 + 1);
        if (sha.size() != 64 || rel.empty() || rel.size() > 2048) continue;
        bool hex = true;
        for (char c : sha) {
            bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
            if (!ok) { hex = false; break; }
        }
        if (!hex) continue;
        if (rel.find("..") != std::string::npos) continue;
        Entry e;
        e.sha = sha;
        e.rel = rel;
        try {
            e.size = static_cast<std::uint64_t>(std::stoull(szs));
        } catch (...) {
            continue;
        }
        v.push_back(e);
        if (v.size() > kMaxFiles) { err = "too_many_files"; return false; }
    }
    return true;
}
}
