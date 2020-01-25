#include <algorithm>
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
    if (num_slices == 0) return true;

    uint32_t value = dptable[num_slices];
    if (value == BOT) return false; // unsolvable
    if (value == 0) { // trivially solvable
        if (counters[num_slices] == 0) return false; // unsolvable, pizza type not available
        --counters[num_slices];
        return true;
    }

    /* Determine whether the current number of slices is solvable by checking whether the left and right operands are
     * solvable with the available pizza types. */
    uint32_t left = value;
    uint32_t right = num_slices - left;
    return solve(dptable, counters, left) and solve(dptable, counters, right);
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
    auto reset_counters = [=]() {
        std::copy(counters_original, counters_original + biggest_size + 1, counters);
    };

    /* Allocate and initialize dynamic programming table. */
    uint32_t *dptable = new uint32_t[max_slices + 1];
    std::fill(dptable, dptable + max_slices + 1, BOT); // set all entries to BOT
    dptable[0] = 0;
    for (uint32_t t = 0; t != num_types; ++t)
        dptable[types[t]] = 0; // set all slots that can be trivially solved by a single pizza type

#ifdef VERBOSE
    std::cerr << "DP table: [";
    for (uint32_t i = 0; i != max_slices + 1; ++i) {
        if (i) std::cerr << ", ";
        std::cerr << i << ": " << dptable[i];
    }
    std::cerr << "]\n";
#endif

    /* Solve the dynamic programming table. */
    for (uint32_t i = 1; i <= max_slices; ++i) {
        if (dptable[i] == 0) continue; // trivially solved
#ifdef VERBOSE
        std::cerr << "Solve for " << i << " slices.\n";
#endif
        for (uint32_t t = num_types; t --> 0;) {
            reset_counters();
            uint32_t size_of_type = types[t];
#ifdef VERBOSE
            std::cerr << "  Consider type " << t << " of " << size_of_type << " slices.\n";
#endif

            if (size_of_type > i) { // pizza type is larger than required slices
#ifdef VERBOSE
                std::cerr << "    Type is too large, skip.\n";
#endif
                continue;
            }

            uint32_t count = counters[size_of_type];
            if (count == 0) { // pizza type not available
#ifdef VERBOSE
                std::cerr << "    Type is not available, skip.\n";
#endif
                continue;
            }

            uint32_t left = size_of_type;
            uint32_t right = i - left;
#ifdef VERBOSE
            std::cerr << "    Split into " << left << " + " << right << ".\n";
#endif
            bool solvable_left = solve(dptable, counters, left);
            bool solvable_right = solve(dptable, counters, right);
#ifdef VERBOSE
            std::cerr << "      Left is " << (solvable_left ? "" : "not ") << "solvable.\n";
            std::cerr << "      Right is " << (solvable_right ? "" : "not ") << "solvable.\n";
#endif
            if (solvable_left and solvable_right) {
#ifdef VERBOSE
                std::cerr << "  Solution found: " << left << " + " << right << ".\n";
#endif
                /* Left AND right form a valid solution. */
                dptable[i] = right;
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
#ifdef VERBOSE
    std::cerr << "Solution of " << solution << " slices found!\n";
#endif

    std::vector<uint32_t> pizzas;
    reset_counters();
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
