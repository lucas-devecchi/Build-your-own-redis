#pragma once

#include <stddef.h>
#include <stdint.h>

// Phase 1: Fixed-size Hashtable Base Structures
// ------------------------------------------------------------------

// The intrusive "Hook" that lives inside your data
struct HNode
{
    HNode *next = nullptr;
    uint64_t hcode = 0; // Hash value stored to optimize lookups
};

// Fixed-size table (Buckets)
struct HTab
{
    HNode **tab = nullptr; // Array of pointers to hooks
    size_t mask = 0;       // (N - 1) for fast index calculation
    size_t size = 0;       // Number of elements in this table
};

// Phase 2: Resizable Hashtable Structure
// ------------------------------------------------------------------

// Container handling progressive rehashing
struct HMap
{
    HTab newer;             // New table (double the size)
    HTab older;             // Old table from which we migrate data
    size_t migrate_pos = 0; // Pointer for quota-based migration
};

// Functions for the fixed table (HTab)
// ------------------------------------------------------------------
const size_t k_max_load_factor = 8;

void h_init(HTab *htab, size_t n);
void h_insert(HTab *htab, HNode *node);

// Returns the pointer address to facilitate deletion
HNode **h_lookup(HTab *htab, HNode *key, bool (*eq)(HNode *, HNode *));

// Detaches a node from the list
HNode *h_detach(HTab *htab, HNode **from);

// Functions for the Global Map (HMap)
// ------------------------------------------------------------------

HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
void hm_insert(HMap *hmap, HNode *node);
HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));