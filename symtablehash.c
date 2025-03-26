/* Author: Nicholas Budny */

/* symtablehash.c - Implementation of the SymTable ADT using a hash table */

#include <assert.h>  
#include <stdlib.h>  
#include <string.h>  
#include "symtable.h"  

/* 
 * Array of primes close to powers of 2 to use as bucket counts during expansion.
 */
static const size_t primes[] = {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521};

/* Number of elements in the primes array */
static const size_t numPrimes = sizeof(primes) / sizeof(primes[0]);

/* 
 * A Binding structure represents a single key-value binding in the table.
 * Each node in the bucket's linked list is a Binding.
 */
typedef struct Binding {
    char *pcKey;             /* Defensive copy of the key string */
    const void *pvValue;     /* Value associated with the key (client-owned) */
    struct Binding *pNext;   /* Next binding in this hash bucket */
} Binding;

/* 
 * The SymTable structure represents the entire hash table.
 * It maintains the array of buckets, counts, and current size info.
 */
struct SymTable {
    Binding **ppBuckets;     /* Array of bucket pointers (each bucket is a list) */
    size_t uBucketCount;     /* Current number of buckets */
    size_t uLength;          /* Number of bindings (total across all buckets) */
    size_t uPrimeIndex;      /* Current index into the primes array */
};

/*
 * Computes a hash value for pcKey, returning a value between 0 and uBucketCount-1.
 * Uses a simple polynomial hash algorithm with a prime multiplier.
 */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount) {
    const size_t HASH_MULTIPLIER = 65599;  /* A prime number for hashing */
    size_t uHash = 0;  /* Starting hash value */
    size_t u;          /* Loop counter */
    
    assert(pcKey != NULL);
    
    /* Compute hash value by multiplying previous value by prime and adding char */
    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];
    
    /* Return hash value within range of bucket count */
    return uHash % uBucketCount;
}

/*
 * Expands the hash table by increasing bucket count and rehashing all bindings.
 * Returns 1 if successful, 0 if memory allocation fails.
 * If already at maximum bucket count, returns 1 without expansion.
 */
static int expandTable(SymTable_T oSymTable) {
    size_t newPrimeIndex;    /* Index of next prime to use */
    size_t newBucketCount;   /* New number of buckets */
    size_t i;                /* Loop counter */
    Binding **ppNewBuckets;  /* New array of buckets */
    Binding *pCurr;          /* Current binding during rehashing */
    Binding *pNext;          /* Next binding to process */
    
    assert(oSymTable != NULL);
    
    /* Check if already at maximum bucket count */
    if (oSymTable->uPrimeIndex + 1 >= numPrimes)
        return 1;  /* Cannot expand further, but not an error */
    
    /* Get next prime bucket count */
    newPrimeIndex = oSymTable->uPrimeIndex + 1;
    newBucketCount = primes[newPrimeIndex];
    
    /* Allocate new array of bucket pointers */
    ppNewBuckets = malloc(newBucketCount * sizeof(Binding *));
    if (ppNewBuckets == NULL)
        return 0;  /* Memory allocation failed */
    
    /* Initialize all new buckets to empty */
    for (i = 0; i < newBucketCount; i++)
        ppNewBuckets[i] = NULL;
    
    /* Rehash each binding into the new bucket array */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        /* Process all bindings in this bucket */
        for (pCurr = oSymTable->ppBuckets[i]; pCurr != NULL; pCurr = pNext) {
            /* Save next binding before changing current's next pointer */
            pNext = pCurr->pNext;
            
            /* Compute new hash index for this key */
            size_t newIndex = SymTable_hash(pCurr->pcKey, newBucketCount);
            
            /* Insert at head of appropriate new bucket */
            pCurr->pNext = ppNewBuckets[newIndex];
            ppNewBuckets[newIndex] = pCurr;
        }
    }
    
    /* Free old bucket array */
    free(oSymTable->ppBuckets);
    
    /* Update symtable with new bucket array and counts */
    oSymTable->ppBuckets = ppNewBuckets;
    oSymTable->uBucketCount = newBucketCount;
    oSymTable->uPrimeIndex = newPrimeIndex;
    
    return 1;  /* Expansion successful */
}

/*
 * Creates a new empty symbol table.
 * Returns NULL if insufficient memory is available.
 */
SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;  /* The new symtable to create */
    
    /* Allocate memory for the SymTable structure */
    oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;  /* Memory allocation failed */
    
    /* Start with the first prime bucket count */
    oSymTable->uPrimeIndex = 0;
    oSymTable->uBucketCount = primes[oSymTable->uPrimeIndex];
    oSymTable->uLength = 0;  /* No bindings yet */
    
    /* Allocate the initial bucket array */
    oSymTable->ppBuckets = malloc(oSymTable->uBucketCount * sizeof(Binding *));
    if (oSymTable->ppBuckets == NULL) {
        /* Bucket array allocation failed, clean up and return NULL */
        free(oSymTable);
        return NULL;
    }
    
    /* Initialize all buckets to empty */
    for (size_t i = 0; i < oSymTable->uBucketCount; i++)
        oSymTable->ppBuckets[i] = NULL;
    
    return oSymTable;
}

/*
 * Frees all memory occupied by oSymTable, including all keys.
 * Does not free memory occupied by the values.
 */
void SymTable_free(SymTable_T oSymTable) {
    size_t i;        /* Loop counter for buckets */
    Binding *pCurr;  /* Current binding being processed */
    Binding *pTemp;  /* Temporary pointer to hold binding before freeing */
    
    /* Validate input parameter */
    assert(oSymTable != NULL);
    
    /* Process each bucket */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        /* Free all bindings in this bucket */
        for (pCurr = oSymTable->ppBuckets[i]; pCurr != NULL; pCurr = pTemp) {
            /* Save next binding before freeing current */
            pTemp = pCurr->pNext;
            
            /* Free the key string (which we own) */
            free(pCurr->pcKey);
            
            /* Free the binding structure */
            free(pCurr);
        }
    }
    
    /* Free the bucket array */
    free(oSymTable->ppBuckets);
    
    /* Free the SymTable structure */
    free(oSymTable);
}

/*
 * Returns the number of bindings in oSymTable.
 * Maintains O(1) time complexity by tracking the count.
 */
size_t SymTable_getLength(SymTable_T oSymTable) {
    /* Validate input parameter */
    assert(oSymTable != NULL);
    
    /* Return the pre-computed length */
    return oSymTable->uLength;
}

/*
 * Adds a new binding to oSymTable.
 * Returns 1 (TRUE) if successful, 0 (FALSE) if key already exists
 * or if insufficient memory is available.
 */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    size_t index;    /* Hash bucket index for this key */
    Binding *pCurr;  /* For traversing the bucket */
    Binding *pNew;   /* New binding to add */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Check if key already exists in this bucket */
    for (pCurr = oSymTable->ppBuckets[index]; pCurr != NULL; pCurr = pCurr->pNext) {
        if (strcmp(pCurr->pcKey, pcKey) == 0)
            return 0;  /* Key already exists, return FALSE */
    }
    
    /* Allocate memory for new binding */
    pNew = malloc(sizeof(Binding));
    if (pNew == NULL)
        return 0;  /* Memory allocation failed */
    
    /* Allocate memory for defensive copy of key */
    pNew->pcKey = malloc(strlen(pcKey) + 1);  /* +1 for null terminator */
    if (pNew->pcKey == NULL) {
        /* Key allocation failed, clean up and return FALSE */
        free(pNew);
        return 0;
    }
    
    /* Create defensive copy of the key */
    strcpy(pNew->pcKey, pcKey);
    
    /* Store the value pointer (no defensive copy) */
    pNew->pvValue = pvValue;
    
    /* Insert at the head of the bucket's list for O(1) insertion */
    pNew->pNext = oSymTable->ppBuckets[index];
    oSymTable->ppBuckets[index] = pNew;
    
    /* Increment the binding count */
    oSymTable->uLength++;
    
    /* Check if expansion is needed (bindings > buckets) */
    if (oSymTable->uLength > oSymTable->uBucketCount)
        expandTable(oSymTable);  /* Ignore return value - keep going even if expansion fails */
    
    return 1;  /* Success, return TRUE */
}

/*
 * Replaces the value of an existing binding.
 * Returns the old value if successful, NULL if key doesn't exist.
 */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    size_t index;      /* Hash bucket index for this key */
    Binding *pCurr;    /* For traversing the bucket */
    const void *pvOld; /* Old value to return */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Search for the key in this bucket */
    for (pCurr = oSymTable->ppBuckets[index]; pCurr != NULL; pCurr = pCurr->pNext) {
        if (strcmp(pCurr->pcKey, pcKey) == 0) {
            /* Key found, save the old value */
            pvOld = pCurr->pvValue;
            
            /* Replace with new value */
            pCurr->pvValue = pvValue;
            
            /* Return the old value */
            return (void *)pvOld;
        }
    }
    
    /* Key not found */
    return NULL;
}

/*
 * Checks if pcKey exists in oSymTable.
 * Returns 1 (TRUE) if found, 0 (FALSE) otherwise.
 */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    size_t index;    /* Hash bucket index for this key */
    Binding *pCurr;  /* For traversing the bucket */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Search for the key in this bucket */
    for (pCurr = oSymTable->ppBuckets[index]; pCurr != NULL; pCurr = pCurr->pNext) {
        if (strcmp(pCurr->pcKey, pcKey) == 0)
            return 1;  /* Key found, return TRUE */
    }
    
    /* Key not found */
    return 0;  /* Return FALSE */
}

/*
 * Returns the value associated with pcKey, or NULL if not found.
 */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    size_t index;    /* Hash bucket index for this key */
    Binding *pCurr;  /* For traversing the bucket */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Search for the key in this bucket */
    for (pCurr = oSymTable->ppBuckets[index]; pCurr != NULL; pCurr = pCurr->pNext) {
        if (strcmp(pCurr->pcKey, pcKey) == 0)
            /* Key found, return its value */
            return (void *)pCurr->pvValue;
    }
    
    /* Key not found */
    return NULL;
}

/*
 * Removes the binding with key pcKey from oSymTable.
 * Returns the value of the removed binding, or NULL if key not found.
 */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    size_t index;      /* Hash bucket index for this key */
    Binding *pCurr;    /* Current binding in traversal */
    Binding *pPrev = NULL;  /* Previous binding (initially NULL) */
    const void *pvValue;  /* Value to return */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Search for the key in this bucket */
    for (pCurr = oSymTable->ppBuckets[index]; pCurr != NULL; pCurr = pCurr->pNext) {
        if (strcmp(pCurr->pcKey, pcKey) == 0) {
            /* Key found, remove the binding */
            
            /* Handle case where binding is at the head of bucket */
            if (pPrev == NULL)
                oSymTable->ppBuckets[index] = pCurr->pNext;
            else
                /* Handle case where binding is in the middle/end of bucket */
                pPrev->pNext = pCurr->pNext;
            
            /* Save the value to return */
            pvValue = pCurr->pvValue;
            
            /* Free the key string (which we own) */
            free(pCurr->pcKey);
            
            /* Free the binding structure */
            free(pCurr);
            
            /* Decrement the count of bindings */
            oSymTable->uLength--;
            
            /* Return the value from the removed binding */
            return (void *)pvValue;
        }
        
        /* Keep track of previous binding for list manipulation */
        pPrev = pCurr;
    }
    
    /* Key not found */
    return NULL;
}

/*
 * Applies function pfApply to each binding in oSymTable.
 * For each binding, calls pfApply(pcKey, pvValue, pvExtra).
 */
void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
                  const void *pvExtra) {
    size_t i;        /* Loop counter for buckets */
    Binding *pCurr;  /* For traversing each bucket */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    
    /* Process each bucket */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        /* Apply function to each binding in this bucket */
        for (pCurr = oSymTable->ppBuckets[i]; pCurr != NULL; pCurr = pCurr->pNext)
            /* Call the client-provided function with key, value, and extra data */
            pfApply(pCurr->pcKey, (void *)pCurr->pvValue, (void *)pvExtra);
    }
}
