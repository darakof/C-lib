#include <benchmark/benchmark.h>
#include <cstdint>
#include <unordered_map>
#include <string.h>

extern "C" {
	#include "hashtable.h"
}

static void BM_Cpp_UnorderedMap_Insert(benchmark::State& state) {
	for (auto _ : state) {
		std::unordered_map<uint64_t, uint64_t> map;
		for (int i = 0; i < state.range(0); ++i) {
			map[i] = i;
		}
		benchmark::ClobberMemory();
	}
}

// test 100 1,000 10,000 items
BENCHMARK(BM_Cpp_UnorderedMap_Insert)->Range(100,10000);

// required custom functions for the table
uint64_t hthash(void* key) {
	return *(uint64_t*)key;
}
bool htcmp(void* lhs, void* rhs) {
	return *(uint64_t*)lhs == *(uint64_t*)rhs;
}

static void BM_clib_HashTable_Insert(benchmark::State& state) {
	for (auto _ : state) {
		HashTable* map = ht_new(sizeof(uint64_t));
		ht_set_hashfn(map, hthash);
		ht_set_cmpfn(map, htcmp);
		for (int i = 0; i < state.range(0); ++i) {
			ht_addKey(map, &i, i);
		}
		ht_destroy(map);
		benchmark::ClobberMemory();
	}
}

BENCHMARK(BM_clib_HashTable_Insert)->Range(100, 10000);
