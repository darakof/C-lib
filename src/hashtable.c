// SPDX-License-Identifier: MIT

#include "hashtable.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// allows us to align the key size and capacities so everything is correctly aligned
#define ALIGN_UP(num, align) (((num) + ((align) - 1)) & ~((align) - 1))

static uint64_t nextPowerOfTwo(uint64_t x)
{
    if (x <= 1)
        return 1;

    x--;

    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;

    return x + 1;
}

// return the fingerprint based on the hash
// i gotta test it for whether the higher or lower bits are better for collisions
static inline uint8_t getfprint(uint64_t hash) {
	return (hash>>56);
}

HashTable* ht_new(size_t KeySize) {
	size_t alignedKeySize = ALIGN_UP(KeySize, 8);
	uint64_t alignedInitCap = nextPowerOfTwo(HASHTABLE_INITCAP);
	HashTable* hashtable = (HashTable*)malloc(sizeof(HashTable));
	if (hashtable == NULL) return NULL;
	hashtable->hash = NULL;
	hashtable->cmp = NULL;
	hashtable->KeySize = alignedKeySize;
	hashtable->capacity = alignedInitCap;
	hashtable->indexMask = alignedInitCap-1;
	hashtable->used = 0;
	// precompute the 75% load needed to double capacity
	hashtable->useCap = HASHTABLE_LOADPREF * alignedInitCap;
	// calloc since it sets the memory to 0 and we require that otherwise our lookup will see the values as occupied
	// calculate the total amount of storage needed for one entry then multiply that by the amount of entries needed
	uint8_t* base = (uint8_t*)calloc(1, (sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t) + alignedKeySize + sizeof(uint64_t))*(alignedInitCap));
	if (base == NULL) {
		free(hashtable);
		return NULL;
	}

	hashtable->fingerprints = base; // offset = 0
	hashtable->distances = (uint32_t*)(base + (sizeof(uint8_t) * alignedInitCap));
	hashtable->hashes = (uint64_t*)(base + (sizeof(uint8_t)+sizeof(uint32_t)) * alignedInitCap);
	// they keys will have the value right after them so we dont need to do any more poitner arithmetic
	hashtable->keys = (uint64_t*)(base + (sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint64_t)) * alignedInitCap);
	
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

static inline void writeKey(HashTable* hashtable, void* key, uint64_t value, uint64_t index, uint8_t fingerprint, uint32_t distance, uint64_t hash) {
	// store the key and value
	hashtable->fingerprints[index] = fingerprint;
	hashtable->distances[index] = distance;
	hashtable->hashes[index] = hash;
	// each element is KeySize + value size (uint64_t)
	uint8_t* keyLoc = (uint8_t*)((uint8_t*)hashtable->keys + (index*(hashtable->KeySize+sizeof(uint64_t))));
	// write the key
	memcpy(keyLoc, key, hashtable->KeySize);
	// write the value after the key
	uint64_t* valueLoc = (uint64_t*)((uint8_t*)keyLoc + hashtable->KeySize);
	*valueLoc = value;
}

static inline uint64_t* getValAtIndex(uint64_t* keys, uint64_t index, size_t keySize) {	
	// (keys size + value size) * index
	return (uint64_t*)((uint8_t*)keys + ((keySize+sizeof(uint64_t))*index));
}

static inline uint64_t getIndexFromVal(uint64_t* keys, uint64_t* key, size_t KeySize) {
	// key offset / size of key entry
	return (((uint8_t*)key - (uint8_t*)keys) / (KeySize + sizeof(uint64_t)));
}

// allows us to switch the algorithm for probing the table
static inline uint64_t increaseIndex(uint64_t index, uint64_t mask) {
	// simple linear probing
	return (index+1) & mask;
}

static inline void replaceEntry(HashTable* hashtable, uint8_t* fingerprint, uint32_t* distance, uint64_t* hash, uint8_t** key, uint64_t* value, uint64_t index, uint8_t* tmpbuf) {
	// copy the entry[index] values and then write out current values
	uint8_t newFingerprint = hashtable->fingerprints[index];
	uint32_t newDistance = hashtable->distances[index];
	uint64_t newHash = hashtable->hashes[index];
	uint8_t* tmpptr = (uint8_t*)getValAtIndex(hashtable->keys, index, hashtable->KeySize);
	memcpy(tmpbuf, tmpptr, hashtable->KeySize+sizeof(uint64_t));
	writeKey(hashtable, *key, *value, index, *fingerprint, *distance, *hash);
	memcpy(*key, tmpbuf, hashtable->KeySize);
	*fingerprint = newFingerprint;
	*distance = newDistance;
	*hash = newHash;
	*value = *(uint64_t*)((uint8_t*)(tmpbuf) + hashtable->KeySize);
}

static inline bool addEntry(HashTable* hashtable, uint8_t fprint, uint64_t hash, void* key, uint64_t value) {
	uint32_t distance = 0;
	uint64_t index = hash & hashtable->indexMask;

	uint8_t* keybuf = (uint8_t*)malloc(hashtable->KeySize);
	if (keybuf == NULL) return false;
	uint8_t* tmpbuf = (uint8_t*)malloc(hashtable->KeySize + sizeof(uint64_t));
	if (tmpbuf == NULL) return false;

	memcpy(keybuf, key, hashtable->KeySize);
	while (true) {
		if (hashtable->fingerprints[index] == 0) {
			writeKey(hashtable, keybuf, value, index, fprint, distance, hash);
			hashtable->used++;
			free(keybuf);
			free(tmpbuf);
			return true;
		}
		if (hashtable->fingerprints[index] == fprint &&
			hashtable->hashes[index] == hash) {
			
			uint8_t* curKey = (uint8_t*)getValAtIndex(hashtable->keys, index, hashtable->KeySize);
			if (hashtable->cmp(keybuf, curKey)) {
				uint64_t* curVal = (uint64_t*)(curKey + hashtable->KeySize);
				*curVal = value;

				free(keybuf);
				free(tmpbuf);
				return true;
			}
		}

		// robin hood swap
		if (distance > hashtable->distances[index]) {
			replaceEntry(hashtable, &fprint, &distance, &hash, &keybuf, &value, index, tmpbuf);
		}

		index = increaseIndex(index, hashtable->indexMask);
		distance++;
	}
}
// returns the index for the key
static inline uint64_t* getEntry(HashTable* hashtable, uint8_t fprint, uint64_t hash, uint64_t* key) {
	uint32_t distance = 0;
	uint64_t index = hash & hashtable->indexMask;
	// TODO: has a bunch of repeating code so would have to make it its own label instead
	while (true) {
		if (hashtable->fingerprints[index] == 0) {
			// if the entry is empty we know that the entry we're looking for doesnt exist
			return NULL;
		}
		else if (hashtable->distances[index] < distance) {
			// due to the robin hood hash algorithm we know the entry isn't in the table since a lower distance entry can't be in its place
			return NULL;
		}
		else if (hashtable->fingerprints[index] != fprint) {
			index = increaseIndex(index, hashtable->indexMask);
			distance = distance + 1;
			continue;
		}
		else if (hashtable->hashes[index] != hash) {
			index = increaseIndex(index, hashtable->indexMask);
			distance = distance + 1;
			continue;
		}
		else if (hashtable->cmp(getValAtIndex(hashtable->keys, index, hashtable->KeySize), key) == false) {
			index = increaseIndex(index, hashtable->indexMask);
			distance = distance + 1;
			continue;
		}
		else {
			// the entry is found
			return getValAtIndex(hashtable->keys, index, hashtable->KeySize);
		}
	}
}

bool ht_addKey(HashTable* hashtable, void* key, uint64_t value) {
	if (hashtable == NULL) return false;

	// double capacity if needed
	if (hashtable->used >= hashtable->useCap)
		ht_resize(hashtable, hashtable->capacity*2);

	uint64_t hash = hashtable->hash(key);
	// 0 will mean its empty so we always want the value bigger than 0, we OR a 1 so the first bit is always 1 
	// if the fingerprint was 0 it would collide with 1 but since we also check the key it doesnt really matter
	uint8_t fingerprint = getfprint(hash) | 1;
	// store the keys so the compiler figures it out that we want it to be accessible quickly
	
	return addEntry(hashtable, fingerprint, hash, key, value);
}

bool ht_getVal(HashTable* hashtable, void* key, uint64_t* retVal) {
	if (hashtable == NULL) return false;

	uint64_t hash = hashtable->hash(key);
	uint64_t* keyLoc = getEntry(hashtable, getfprint(hash) | 1, hash, (uint64_t*)key);
	if (keyLoc == NULL) return false;
	*retVal = *(uint64_t*)((uint8_t*)keyLoc + hashtable->KeySize);
	return true;
}

bool ht_delete(HashTable *hashtable, void *key) {
	if (hashtable == NULL) return false;

	uint64_t hash = hashtable->hash(key);
	
	uint64_t* keyEntry = getEntry(hashtable, getfprint(hash) | 1, hash, (uint64_t*)key);
	if (keyEntry == NULL) return false;

	uint64_t keyIndex = getIndexFromVal(hashtable->keys, keyEntry, hashtable->KeySize);

	while (true) {
		uint64_t next = (keyIndex + 1) & hashtable->indexMask;
		if (hashtable->fingerprints[next] != 0) {
			if (hashtable->distances[next] == 0) {
				// the entry is already as close as possible to its target index
				return true;
			}
			// the entry should be able to be moved back
			hashtable->fingerprints[keyIndex] = hashtable->fingerprints[next];
			hashtable->distances[keyIndex] = hashtable->distances[next];
			hashtable->hashes[keyIndex] = hashtable->hashes[next];
			// copy the key and value into the current entry from the next entry
			memcpy(getValAtIndex(hashtable->keys, keyIndex, hashtable->KeySize), getValAtIndex(hashtable->keys, next, hashtable->KeySize), hashtable->KeySize + sizeof(uint64_t));
			keyIndex = next;
		}
		else {
			// there is no more entries to push back so we clear the current one and exit
			hashtable->fingerprints[keyIndex] = 0;
			hashtable->distances[keyIndex] = 0;
			hashtable->used = hashtable->used - 1;
			return true;
		}
	}
}

void ht_resize(HashTable *hashtable, uint64_t maxKeys) {
	if (hashtable == NULL) return;
	if (hashtable->used > maxKeys) return;
	
	// make sure the size is correctly setup
	maxKeys = nextPowerOfTwo(maxKeys);

	uint8_t* oldFprints = hashtable->fingerprints;
	uint64_t* oldHashes = hashtable->hashes;
	uint64_t* oldKeys = hashtable->keys;

	uint8_t* base = (uint8_t*)calloc(1, (sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t) + hashtable->KeySize + sizeof(uint64_t))*(maxKeys));
	if (base == NULL) {
		return;
	}

	hashtable->fingerprints = base; // offset = 0
	hashtable->distances = (uint32_t*)((uint8_t*)base + (sizeof(uint8_t) * maxKeys));
	hashtable->hashes = (uint64_t*)((uint8_t*)base + (sizeof(uint8_t)+sizeof(uint32_t)) * maxKeys);
	hashtable->keys = (uint64_t*)((uint8_t*)base + (sizeof(uint8_t)+sizeof(uint32_t)+sizeof(uint64_t)) * maxKeys);

	uint64_t oldCap = hashtable->capacity;
	hashtable->used = 0;
	hashtable->capacity = maxKeys;
	hashtable->useCap = maxKeys * HASHTABLE_LOADPREF;
	hashtable->indexMask = maxKeys - 1;
	
	for (uint64_t index = 0; index < oldCap; ++index) {
	    if (oldFprints[index] == 0)
    	    continue;

	    uint64_t* key =
    	    getValAtIndex(oldKeys, index, hashtable->KeySize);

	    addEntry(
    	    hashtable,
        	oldFprints[index],
    	    oldHashes[index],
	        key,
        	*(uint64_t*)((uint8_t*)key + hashtable->KeySize)
    	);
	}

	free(oldFprints);
}
