#include <algorithm>
#include <cassert>
#include <cstdint>
#include <err.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>


//#define VERBOSE


constexpr uint32_t BOT = std::numeric_limits<uint32_t>::max();


bool solve(uint32_t *dptable, uint32_t *counters, uint32_t num_slices)
{
    assert(num_slices != BOT);
    while (num_slices) {
        const uint32_t prev = dptable[num_slices];
        if (prev == BOT) return false; // unsolvable

        const uint32_t a_pizza = num_slices - prev; // must be the size of a pizza type
        assert(a_pizza != 0);
        if (counters[a_pizza] == 0) return false; // pizza type not available anymore
        --counters[a_pizza];

        num_slices = prev;
    }
    return true; // num_slices == 0  =>  trivially solvable
}


int main(int argc, const char **argv)
{
    if (argc != 2)
        errx(EXIT_FAILURE, "illegal number of arguments");

    const char *in_file_name = argv[1];

    /* Create input file stream. */
    std::ifstream in(in_file_name);
    if (not in)
        err(EXIT_FAILURE, "Failed to open the input file '%s'", in_file_name);

    /* Read problem description: maximum number of pizza slices and number of pizza types. */
    uint32_t max_slices, num_types;
    in >> max_slices >> num_types;
#ifdef VERBOSE
    std::cerr << "Maximum " << max_slices << " slices with " << num_types << " types.\n";
#endif

    /* Allocate array for pizza types. */
    uint32_t *types = new uint32_t[num_types];

    /* Read pizza types. */
    for (uint32_t i = 0; i != num_types; ++i)
        in >> types[i];
#ifdef VERBOSE
    std::cerr << "Pizza types: [";
    for (uint32_t i = 0; i != num_types; ++i) {
        if (i) std::cerr << ", ";
        std::cerr << i << ": " << types[i];
    }
    std::cerr << "]\n";
#endif

    /* Allocate and initialize table of type counters. */
    const uint32_t biggest_size = types[num_types-1];
    uint32_t *counters_original = new uint32_t[biggest_size + 1]();
    for (uint32_t t = 0; t != num_types; ++t)
        ++counters_original[types[t]];
#ifdef VERBOSE
    std::cerr << "counters: [";
    for (uint32_t c = 0; c <= biggest_size; ++c) {
        if (c) std::cerr << ", ";
        std::cerr << counters_original[c];
    }
    std::cerr << "]\n";
#endif

    /* Create a working copy of counters and define a function to reset them to the original state. */
    uint32_t *counters = new uint32_t[biggest_size + 1];
#define RESET_COUNTERS() std::copy(counters_original, counters_original + biggest_size + 1, counters)

    /* Allocate and initialize dynamic programming table. */
    uint32_t *dptable = new uint32_t[max_slices + 1];
    std::fill(dptable, dptable + max_slices + 1, BOT); // set all entries to BOT
    dptable[0] = 0;
    for (uint32_t t = 0; t != num_types and types[t] <= max_slices; ++t)
        dptable[types[t]] = 0; // set all slots that can be trivially solved by a single pizza type

#ifdef VERBOSE
    std::cerr << "DP table: [";
    for (uint32_t i = 0; i != max_slices + 1; ++i) {
        if (i) std::cerr << ", ";
        std::cerr << i << ": " << dptable[i];
    }
    std::cerr << "]\n";
#endif

    const uint32_t milestone = max_slices / 100; // report every 1% progress
    uint32_t next_milestone = milestone;

    /* Solve the dynamic programming table. */
    for (uint32_t i = types[0]; i <= max_slices; ++i) {
        if (i == next_milestone) {
            std::cerr << (next_milestone * 100.f) / max_slices << "%\n";
            next_milestone += milestone;
        }
#ifdef VERBOSE
        std::cerr << "Solve for " << i << " slices.\n";
#endif
        for (uint32_t t = num_types; t --> 0;) {
            uint32_t size_of_type = types[t];
            if (size_of_type > i) continue;            // pizza type is larger than required slices
            RESET_COUNTERS();
            assert(counters[size_of_type] > 0);
            --counters[size_of_type];
            uint32_t prev = i - size_of_type;
            if (solve(dptable, counters, prev)) {
                dptable[i] = prev;
                break;
            }
        }
    }

#ifdef VERBOSE
    std::cerr << "DP table: [";
    for (uint32_t i = 0; i != max_slices + 1; ++i) {
        if (i) std::cerr << ", ";
        std::cerr << i << ": " << dptable[i];
    }
    std::cerr << "]\n";
#endif

    uint32_t solution = max_slices;
    while (dptable[solution] == BOT) --solution;
    std::cerr << "Solution of " << solution << " slices found!\n";

    std::vector<uint32_t> pizzas;
    RESET_COUNTERS();
    while (solution) {
        uint32_t left = dptable[solution];
        uint32_t right = solution - left;
        pizzas.push_back(right);
        solution = left;
    }
#ifdef VERBOSE
    std::cerr << "Use the following pizzas:";
    for (auto p : pizzas)
        std::cerr << ' ' << p;
    std::cerr << '\n';
#endif

    std::sort(pizzas.begin(), pizzas.end());
    uint32_t t = 0;
    auto p = pizzas.begin();

    std::cout << pizzas.size() << '\n';
    while (p != pizzas.end()) {
        if (types[t] == *p) {
            std::cout << t << ' ';
            ++p;
        }
        ++t;
    }
    std::cout << '\n';

    /* Dispose of acquired resources. */
    delete[] counters;
    delete[] counters_original;
    delete[] dptable;
    delete[] types;
}
