#include <assert.h>
#include <stdlib.h>
#include "hashtable.h" // Ensure you create this header with the structs

// Step 2: Fixed-size table initialization
void h_init(HTab *htab, size_t n)
{
    // n must be a power of 2 for the mask to work
    assert(n > 0 && ((n - 1) & n) == 0);

    // Use calloc for lazy initialization (initial O(1))
    htab->tab = (HNode **)calloc(n, sizeof(HNode *));
    htab->mask = n - 1; // Create the mask to avoid using '%'
    htab->size = 0;
}

// Step 3: Linked list insertion (O(1))
void h_insert(HTab *htab, HNode *node)
{
    // Calculate the position using bitwise AND
    size_t pos = node->hcode & htab->mask;

    // Insert at the front of the list (Front insertion)
    HNode *next = htab->tab[pos];
    node->next = next;
    htab->tab[pos] = node;
    htab->size++;
}

// Step 6: Generic lookup
HNode **h_lookup(HTab *htab, HNode *key, bool (*eq)(HNode *, HNode *))
{
    if (!htab->tab)
    {
        return NULL;
    }

    size_t pos = key->hcode & htab->mask;
    HNode **from = &htab->tab[pos]; // Pointer to the bucket pointer

    // Traverse the list of hooks
    for (HNode *cur; (cur = *from) != NULL; from = &cur->next)
    {
        // Optimization: compare hcode before calling the 'eq' callback
        if (cur->hcode == key->hcode && eq(cur, key))
        {
            return from; // Return the address of the pointer (useful for deletion)
        }
    }
    return NULL;
}

// Step 7: Detach a node
HNode *h_detach(HTab *htab, HNode **from)
{
    HNode *node = *from; // The node we found

    // Update the "parent" pointer to skip to the next node
    // This works thanks to the double pointer from h_lookup
    *from = node->next;

    htab->size--;
    return node;
}

static void hm_trigger_rehashing(HMap *hmap)
{
    assert(hmap->older.tab == NULL);
    // (newer, older) <- (new_table, newer)
    hmap->older = hmap->newer;
    h_init(&hmap->newer, (hmap->newer.mask + 1) * 2); // Double the size
    hmap->migrate_pos = 0;
}