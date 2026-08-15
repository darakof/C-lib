#include <criterion/criterion.h>
#include <criterion/internal/assert.h>

#define HASHTABLE_INITCAP 7
#include "hashtable.h"

uint64_t hashtable_hashu64(void* key) {
	// just return the key since its 64 bit
	return *(uint64_t*)key;
}

bool hashtable_cmpu64(void* lhs, void* rhs) {
	// simple 64 bit comparison
	return *(uint64_t*)lhs == *(uint64_t*)rhs;
}

Test(hashtable, initalization) {
	HashTable* hashtableu64 = ht_new(sizeof(uint64_t));
	ht_set_hashfn(hashtableu64, hashtable_hashu64);
	ht_set_cmpfn(hashtableu64, hashtable_cmpu64);

	// test the correct size initialization

	// we round up the capacity to be a multiple of 8
	cr_assert(hashtableu64->capacity == 8, "hash table capacity isnt rounded up correctly expected: %llu received: %llu\n", (unsigned long long)8, (unsigned long long)hashtableu64->capacity);
	// 8 / 0.75
	cr_assert(hashtableu64->useCap == 6, "hash table load preference isnt calculated correctly expected: %llu received: %llu\n", (unsigned long long)6, (unsigned long long)hashtableu64->useCap);
	ht_destroy(hashtableu64);
}

Test(hashtable, getset) {
	HashTable* hashtableu64 = ht_new(sizeof(uint64_t));
	ht_set_hashfn(hashtableu64, hashtable_hashu64);
	ht_set_cmpfn(hashtableu64, hashtable_cmpu64);
	uint64_t tempKey = 39;
	uint64_t tempVal = 13;
	// test correct key insertion and lookup
	ht_addKey(hashtableu64, &tempKey, tempVal);
	cr_assert(hashtableu64->used == 1, "hash table entry count isnt incremented correctly expected: %llu received: %llu ", (unsigned long long)1, (unsigned long long)hashtableu64->used);
	tempVal = 0;
	cr_assert(ht_getVal(hashtableu64, &tempKey, &tempVal), "hash table unable to find entry");
	cr_assert(tempVal != 0, "hash table not able to lookup an inserted entry actual value: %llu", (unsigned long long)tempVal);
	cr_assert(tempVal == 13, "hash table returning incorrect entry values");
	ht_destroy(hashtableu64);
}

Test(hashtable, growth) {
	HashTable* hashtableu64 = ht_new(sizeof(uint64_t));
	ht_set_hashfn(hashtableu64, hashtable_hashu64);
	ht_set_cmpfn(hashtableu64, hashtable_cmpu64);
	uint64_t tempKey = 39;
	uint64_t tempVal = 13;
	// test growth and rehashing (technically doesnt actually rehash)
	ht_addKey(hashtableu64, &tempKey, tempVal);
	ht_resize(hashtableu64, 15);
	cr_assert(hashtableu64->capacity == 16, "hash table incorectly rounding up capacity when growing expected: %llu received: %llu", (unsigned long long)16, (unsigned long long)hashtableu64->capacity);
	cr_assert(hashtableu64->useCap == 12, "hash table incorectly recalculating load prference expected: %llu received: %llu", (unsigned long long)12, (unsigned long long)hashtableu64->useCap);
	cr_assert(hashtableu64->used == 1, "hash table get incorrect entry count after growing expected %llu received: %llu", (unsigned long long)1, (unsigned long long)hashtableu64->used);
	tempVal = 0;
	ht_getVal(hashtableu64, &tempKey, &tempVal);
	cr_assert(tempVal != 0, "hash table not correctly rehashing values actual tempVal: %llu", (unsigned long long)tempVal);
	cr_assert(tempVal == 13, "hash table incorectly writing entry value after growing");
	ht_destroy(hashtableu64);
}

Test(hashtable, deletion) {
	HashTable* hashtableu64 = ht_new(sizeof(uint64_t));
	ht_set_hashfn(hashtableu64, hashtable_hashu64);
	ht_set_cmpfn(hashtableu64, hashtable_cmpu64);
	uint64_t tempKey = 39;
	uint64_t tempVal = 0;
	ht_addKey(hashtableu64, &tempKey, tempVal);
	// test deletion
	ht_delete(hashtableu64, &tempKey);
	ht_getVal(hashtableu64, &tempKey, &tempVal);
	cr_assert(hashtableu64->used == 0, "hash table incorrectly changes entry count after deletion expected: %llu received: %llu", (unsigned long long)0, (unsigned long long)hashtableu64->used);
	cr_assert(tempVal == 0, "hash table unable to delete entry");
	ht_destroy(hashtableu64);
}
