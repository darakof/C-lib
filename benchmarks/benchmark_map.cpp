#include <benchmark/benchmark.h>
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <string.h>
#include <vector>
#include <random>
#include <algorithm>

// since its a C library we have to stop it from name mangling
extern "C" {
#include "hashtable.h"
}

enum class KeyPattern {
    Sequential,
    Random,
    Adversarial
};

static std::vector<uint64_t> genKeys(size_t n, KeyPattern pattern)
{
    std::vector<uint64_t> keys(n);

    switch (pattern) {
    case KeyPattern::Sequential:
        for (size_t i = 0; i < n; ++i)
            keys[i] = i + 1;
        break;

    case KeyPattern::Random: {
        // Guaranteed unique random keys.
        for (size_t i = 0; i < n; ++i)
            keys[i] = i + 1;

        std::mt19937_64 gen(12345);
        std::shuffle(keys.begin(), keys.end(), gen);
        break;
    }

    case KeyPattern::Adversarial:
        for (size_t i = 0; i < n; ++i)
            keys[i] = (i + 1) << 16;
        break;
    }

    return keys;
}

static void BM_Cpp_UnorderedMap_InsertAutoGrow(benchmark::State& state) {
	const size_t N = state.range(0);
	const KeyPattern pattern = static_cast<KeyPattern>(state.range(1));

	std::vector<uint64_t> keys = genKeys(N, pattern);

	for (auto _ : state) {
		state.PauseTiming();
		std::unordered_map<uint64_t, uint64_t> map;
		state.ResumeTiming();
		for (uint64_t i = 0; i < N; ++i) {
			map[keys[i]] = i;
		}
		benchmark::ClobberMemory();
	}
}

// test 100 1,000 10,000 items
BENCHMARK(BM_Cpp_UnorderedMap_InsertAutoGrow)->Ranges({{100,10000}, {0, 2}});

// required custom functions for the table
/*uint64_t hthash(void* key) {
	uint64_t keyTemp = *(uint64_t*)key;
	return keyTemp; //| (keyTemp << 56);
}
*/

// TEMP splitmix64 HASH

uint64_t hthash(void* key) {
    uint64_t x = *(uint64_t*)key;

    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;

    return x;
}

bool htcmp(void* lhs, void* rhs) {
	return *(uint64_t*)lhs == *(uint64_t*)rhs;
}

static void BM_clib_HashTable_InsertAutoGrow(benchmark::State& state) {
	const size_t N = state.range(0);
	const KeyPattern pattern = static_cast<KeyPattern>(state.range(1));

	std::vector<uint64_t> keys = genKeys(N, pattern);

	for (auto _ : state) {
		state.PauseTiming();
		HashTable* map = ht_new(sizeof(uint64_t));
		ht_set_hashfn(map, hthash);
		ht_set_cmpfn(map, htcmp);
		state.ResumeTiming();
		for (uint64_t i = 0; i < N; ++i) {
			ht_addKey(map, &keys[i], i);
		}
		state.PauseTiming();
		ht_destroy(map);
		state.ResumeTiming();
	}
}

BENCHMARK(BM_clib_HashTable_InsertAutoGrow)->Ranges({{100, 10000}, {0, 2}});

static void BM_Cpp_UnorderedMap_InsertReserve(benchmark::State& state) {
	const size_t N = state.range(0);
	const KeyPattern pattern = static_cast<KeyPattern>(state.range(1));

	std::vector<uint64_t> keys = genKeys(N, pattern);

	for (auto _ : state) {
		state.PauseTiming();
		std::unordered_map<uint64_t, uint64_t> map;
		map.reserve(state.range(0));
		state.ResumeTiming();

		for (uint64_t i = 0; i < N; ++i) {
			map[keys[i]] = i;
		}
		state.PauseTiming();
		state.counters["Bucket Count"] = map.bucket_count();
		state.counters["Load"] = map.load_factor();
		benchmark::ClobberMemory();
		state.ResumeTiming();
	}
}

BENCHMARK(BM_Cpp_UnorderedMap_InsertReserve)->Ranges({{100,10000}, {0, 2}});

static void BM_clib_HashTable_InsertReserve(benchmark::State& state) {
	const size_t N = state.range(0);
	const KeyPattern pattern = static_cast<KeyPattern>(state.range(1));

	std::vector<uint64_t> keys = genKeys(N, pattern);

	for (auto _ : state) {
		state.PauseTiming();
		HashTable* map = ht_new(sizeof(uint64_t));
		ht_set_hashfn(map, hthash);
		ht_set_cmpfn(map, htcmp);
		ht_resize(map, N);
		state.ResumeTiming();
		for (uint64_t i = 0; i < N; ++i) {
			ht_addKey(map, &keys[i], i);
		}
		state.PauseTiming();
		
		state.counters["Capacity"] = map->capacity;
		state.counters["Load"] = static_cast<double>(map->used) / map->capacity;

		ht_destroy(map);
		state.ResumeTiming();
	}
}

BENCHMARK(BM_clib_HashTable_InsertReserve)->Ranges({{100, 10000}, {0, 2}});

