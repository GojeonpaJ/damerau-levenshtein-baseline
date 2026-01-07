#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include <iomanip>


// True Damerau–Levenshtein distance (not OSA).
int damerau_levenshtein_true(const std::string& a, const std::string& b) {
    const int n = static_cast<int>(a.size());
    const int m = static_cast<int>(b.size());
    const int INF = n + m;

    // DP matrix: (n+2) x (m+2)
    std::vector<int> d((n + 2) * (m + 2), 0);
    auto at = [&](int i, int j) -> int& { return d[i * (m + 2) + j]; };

    at(0, 0) = INF;
    for (int i = 0; i <= n; ++i) {
        at(i + 1, 0) = INF;
        at(i + 1, 1) = i;
    }
    for (int j = 0; j <= m; ++j) {
        at(0, j + 1) = INF;
        at(1, j + 1) = j;
    }

    // last row occurrence of each character in a (ASCII 0..255)
    std::vector<int> da(256, 0);

    for (int i = 1; i <= n; ++i) {
        int db = 0;  // last column match in b for a[i-1]

        for (int j = 1; j <= m; ++j) {
            const unsigned char bj = static_cast<unsigned char>(b[j - 1]);
            const unsigned char ai = static_cast<unsigned char>(a[i - 1]);

            const int i1 = da[bj];
            const int j1 = db;

            int cost = 1;
            if (a[i - 1] == b[j - 1]) {
                cost = 0;
                db = j;
            }

            const int del = at(i, j + 1) + 1;
            const int ins = at(i + 1, j) + 1;
            const int sub = at(i, j) + cost;

            int best = std::min({ del, ins, sub });

            // transposition
            const int transp = at(i1, j1) + (i - i1 - 1) + 1 + (j - j1 - 1);
            best = std::min(best, transp);

            at(i + 1, j + 1) = best;
        }

        da[static_cast<unsigned char>(a[i - 1])] = i;
    }

    return at(n + 1, m + 1);
}

static std::string random_dna(size_t len, std::mt19937& rng) {
    static const char alphabet[4] = { 'A', 'C', 'G', 'T' };
    std::uniform_int_distribution<int> dist(0, 3);

    std::string s;
    s.resize(len);
    for (size_t i = 0; i < len; ++i) {
        s[i] = alphabet[dist(rng)];
    }
    return s;
}

static double time_one_pair_us(const std::string& a, const std::string& b, int iters) {
    using clk = std::chrono::high_resolution_clock;

    volatile int sink = 0;
    for (int i = 0; i < 3; ++i)
        sink += damerau_levenshtein_true(a, b);

    auto t0 = clk::now();
    for (int i = 0; i < iters; ++i)
        sink += damerau_levenshtein_true(a, b);
    auto t1 = clk::now();

    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    return static_cast<double>(us) / iters;
}

static void run_batch_benchmark_to_csv(const std::string& csv_path) {
    std::mt19937 rng(123456);

    const std::vector<int> lengths = { 50, 100, 200, 500, 1000, 2000, 4000 };

    std::ofstream out(csv_path, std::ios::out | std::ios::trunc);
    if (!out) {
        std::cerr << "Failed to open CSV file: " << csv_path << "\n";
        std::exit(1);
    }

    out << "length,iters,avg_us\n";
    std::cout << "Writing results to: " << csv_path << "\n";

    for (int L : lengths) {
        std::string a = random_dna(L, rng);
        std::string b = random_dna(L, rng);

        int iters = 200;
        if (L >= 500)  iters = 50;
        if (L >= 1000) iters = 20;
        if (L >= 2000) iters = 10;
        if (L >= 4000) iters = 5;

        double avg_us = time_one_pair_us(a, b, iters);

        std::cout << "L=" << L
            << "  iters=" << iters
            << "  avg_us=" << std::fixed << std::setprecision(3)
            << avg_us << "\n";

        out << L << "," << iters << ","
            << std::fixed << std::setprecision(6)
            << avg_us << "\n";
    }

    out.close();
    std::cout << "Done.\n";
}


static void quick_unit_tests() {
    struct Case { std::string a, b; int expected; };
    std::vector<Case> cases = {
        {"", "", 0},
        {"a", "", 1},
        {"", "abc", 3},
        {"abc", "abc", 0},
        {"ca", "ac", 1},         // transposition
        {"abcd", "abdc", 1},     // transposition
        {"kitten", "sitting", 3}
    };

    for (const auto& c : cases) {
        int got = damerau_levenshtein_true(c.a, c.b);
        if (got != c.expected) {
            std::cerr << "[FAIL] a=\"" << c.a << "\" b=\"" << c.b
                << "\" expected=" << c.expected << " got=" << got << "\n";
            std::exit(1);
        }
    }
    std::cout << "[OK] basic unit tests passed\n";
}


int main() {
    quick_unit_tests();
    run_batch_benchmark_to_csv("results_baseline.csv");
    return 0;
}

