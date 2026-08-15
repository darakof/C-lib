// SPDX-License-Identifier: MIT

#ifndef HASHTABLE_H
#define HASHTABLE_H

// USAGE:
// the user has to hold their own array with the actual values
// they can use any data of any size as the key
// they must provide their own functions to compare keys and generate an (almost) unique hash for the keys
// the hash function should have focus on the lower bits being different as that is what is used to comparison first meaning its faster
// we provide default hash functions which allow you to skip writting one yourself
// using a provided hash function removes the potential optimization a user may make knowing the actual data keys hold
// when a hash table is created it default to which ever provided hash function is fastest (benchmarking should be done and priorities should be set in code)


/*	dev note:
 *	this system should probably be the second best optimized system in the whole library
 *	it uses a very lightweight system where the only thing implemented
 *	is the actual hash table algorithms
 *	this library relies on the user providing the hash and comparison functions as to allow better optimization
 *	for better speed we want to allow the user to handle the data and simply return an index to their array
 *	we also dont have to do a bunch of pointer arithmetic to find the key in an array since its very likely to be bigger than 64 bits
 *	and since we're doign the robin hood hash algorithm we would have to move the values across memory a lot so we simply offload that to the user
 */

// default initializing value which the user can potentially change,
// needs to be a power of 2 though so we would need to check that when creating a new hashtable
// we could just remove any lower bits, basically like the custom ALIGN_UP macro inside arenamem.c
// it increments the value until it is a mutliple of a (in this case 8)
// also by similar i mean its exactly the same so we literally just use that macro and rename it so it makes sence

#ifndef HASHTABLE_INITCAP
#define HASHTABLE_INITCAP 8
#endif

#ifndef HASHTABLE_LOADPREF
#define HASHTABLE_LOADPREF 0.75
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t (*ht_hash_fn)(void*); // generate a hash from a user managed block of data
typedef bool (*ht_cmp_fn)(void*, void*); // check if 2 user defined blocks of data are equal, allows for optimizations with the knowledge of what the data holds

typedef struct {
	uint8_t* fingerprints;
	uint32_t* distances;
	uint64_t* hashes;
	uint64_t* keys;

	uint64_t capacity;
	uint64_t used;
	uint64_t indexMask;
	uint64_t useCap; // allows us to precompute prefered load so we dont have to do expensive float division and comparison
	size_t KeySize;

	ht_hash_fn hash;
	ht_cmp_fn cmp;
} HashTable;


HashTable* ht_new(size_t KeySize);
void ht_destroy(HashTable* hashtable);

// configure the hashtable
void ht_set_cmpfn(HashTable* hashtable, ht_cmp_fn compareFunction);
void ht_set_hashfn(HashTable* hashtable, ht_hash_fn hashFunction);

bool ht_addKey(HashTable* hashtable, void* key, uint64_t value);
bool ht_getVal(HashTable* hashtable, void* key, uint64_t* retVal);
bool ht_delete(HashTable* hashtable, void* key);

// resize to fit maxKeys amount of entries, can only shrink up to the amount of entries currently in the table
void ht_resize(HashTable* hashtable, uint64_t maxKeys);

#endif
