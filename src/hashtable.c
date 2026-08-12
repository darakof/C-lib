// SPDX-License-Identifier: MIT

#include "hashtable.h"

#include <stdlib.h>
#include <stdint.h>

// return the fingerprint based on the hash
// i gotta test it for whether the higher or lower bits are better for collisions
inline uint8_t getfprint(uint64_t hash) {
	return (hash>>56);
}

HashTable* ht_new(size_t KeySize) {
	HashTable* hashtable = (HashTable*)malloc(sizeof(HashTable));
	if (hashtable == NULL) return NULL;
	hashtable->hash = NULL;
	hashtable->cmp = NULL;
	hashtable->KeySize = KeySize;
	hashtable->capacity = HASHTABLE_INITCAP;
	hashtable->indexMask = (HASHTABLE_INITCAP)-1;
	hashtable->used = 0;
	// precompute the 75% load needed to double capacity
	hashtable->useCap = HASHTABLE_LOADPREF * HASHTABLE_INITCAP;
	// calloc since it sets the memory to 0 and we require that otherwise our lookup will see the values as occupied
	// calculate the total amount of storage needed for one entry then multiply that by the amount of entries needed
	uint8_t* base = (uint8_t*)calloc(1, (sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t) + KeySize)*(HASHTABLE_INITCAP));
	if (base == NULL) {
		free(hashtable);
		return NULL;
	}

	hashtable->fingerprints = base; // offset = 0
	hashtable->distances = (uint32_t*)base + (sizeof(uint8_t) * HASHTABLE_INITCAP);
	hashtable->hashes = (uint64_t*)base + (sizeof(uint8_t)+sizeof(uint32_t)) * HASHTABLE_INITCAP;
	hashtable->keys = (uint64_t*)base + (sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint64_t)) * HASHTABLE_INITCAP;
	
	return hashtable;
}

void ht_destroy(HashTable* hashtable) {
	if (hashtable == NULL) return;
	free(hashtable->fingerprints);
	free(hashtable);
}

// configure the hashtable
void ht_set_cmpfn(HashTable* hashtable, ht_cmp_fn compareFunction) {
	if (hashtable == NULL) return;
	if (compareFunction == NULL) return;

	hashtable->cmp = compareFunction;
}

void ht_set_hashfn(HashTable* hashtable, ht_hash_fn hashFunction) {
	if (hashtable == NULL) return;
	if (hashFunction == NULL) return;

	hashtable->hash = hashFunction;
}

bool ht_addKey(HashTable* hashtable, void* key, uint64_t valueIndex);

bool ht_getVal(HashTable* hashtable, void* key, uint64_t* retVal);

