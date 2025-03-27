/* Author: Nicholas Budny */

/* symtablehash.c - Implementation of the SymTable ADT using a hash table */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/* Array of primes close to powers of 2 for bucket counts */
static const size_t primes[] = {509, 1021, 2039, 4093, 8191, 16381, 32749, 65521};

/* Number of elements in the primes array */
static const size_t numPrimes = sizeof(primes) / sizeof(primes[0]);

typedef struct Binding {
    /* Defensive copy of the key string */
    char *pcKey;
    /* Value associated with the key (client-owned) */
    const void *pvValue;
    /* Next binding in this hash bucket */
    struct Binding *pNext;
} Binding;

struct SymTable {
    /* Array of bucket pointers (each bucket is a list) */
    Binding **ppBuckets;
    /* Current number of buckets */
    size_t uBucketCount;
    /* Number of bindings (total across all buckets) */
    size_t uLength;
    /* Current index into the primes array */
    size_t uPrimeIndex;
};

static size_t SymTable_hash(const char *pcKey, size_t uBucketCount) {
    const size_t HASH_MULTIPLIER = 65599;
    size_t uHash = 0;
    size_t u;
    
    assert(pcKey != NULL);
    
    /* Compute hash value by multiplying previous value by prime and adding char */
    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];
    
    return uHash % uBucketCount;
}

static int expandTable(SymTable_T oSymTable) {
    size_t uNewPrimeIndex;
    size_t uNewBucketCount;
    size_t i;
    size_t uNewIndex;
    Binding **ppNewBuckets;
    Binding *pCurrent;
    Binding *pNext;
    
    assert(oSymTable != NULL);
    
    /* Check if already at maximum bucket count */
    if (oSymTable->uPrimeIndex + 1 >= numPrimes)
        return 1;
    
    /* Get next prime bucket count */
    uNewPrimeIndex = oSymTable->uPrimeIndex + 1;
    uNewBucketCount = primes[uNewPrimeIndex];
    
    /* Allocate new array of bucket pointers */
    ppNewBuckets = malloc(uNewBucketCount * sizeof(Binding *));
    if (ppNewBuckets == NULL)
        return 0;
    
    /* Initialize all new buckets to empty */
    for (i = 0; i < uNewBucketCount; i++)
        ppNewBuckets[i] = NULL;
    
    /* Rehash each binding into the new bucket array */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        for (pCurrent = oSymTable->ppBuckets[i]; pCurrent != NULL; pCurrent = pNext) {
            /* Save next binding before changing current's next pointer */
            pNext = pCurrent->pNext;
            
            /* Compute new hash index for this key */
            uNewIndex = SymTable_hash(pCurrent->pcKey, uNewBucketCount);
            
            /* Insert at head of appropriate new bucket */
            pCurrent->pNext = ppNewBuckets[uNewIndex];
            ppNewBuckets[uNewIndex] = pCurrent;
        }
    }
    
    /* Free old bucket array */
    free(oSymTable->ppBuckets);
    
    /* Update symtable with new bucket array and counts */
    oSymTable->ppBuckets = ppNewBuckets;
    oSymTable->uBucketCount = uNewBucketCount;
    oSymTable->uPrimeIndex = uNewPrimeIndex;
    
    return 1;
}

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable;
    size_t i;
    
    /* Allocate memory for the SymTable structure */
    oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
        return NULL;
    
    /* Start with the first prime bucket count */
    oSymTable->uPrimeIndex = 0;
    oSymTable->uBucketCount = primes[oSymTable->uPrimeIndex];
    oSymTable->uLength = 0;
    
    /* Allocate the initial bucket array */
    oSymTable->ppBuckets = malloc(oSymTable->uBucketCount * sizeof(Binding *));
    if (oSymTable->ppBuckets == NULL) {
        free(oSymTable);
        return NULL;
    }
    
    /* Initialize all buckets to empty */
    for (i = 0; i < oSymTable->uBucketCount; i++)
        oSymTable->ppBuckets[i] = NULL;
    
    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
    size_t i;
    Binding *pCurrent;
    Binding *pTemp;
    
    assert(oSymTable != NULL);
    
    /* Process each bucket */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        /* Free all bindings in this bucket */
        for (pCurrent = oSymTable->ppBuckets[i]; pCurrent != NULL; pCurrent = pTemp) {
            /* Save next binding before freeing current */
            pTemp = pCurrent->pNext;
            
            /* Free the key string */
            free(pCurrent->pcKey);
            
            /* Free the binding structure */
            free(pCurrent);
        }
    }
    
    /* Free the bucket array */
    free(oSymTable->ppBuckets);
    
    /* Free the SymTable structure */
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    
    return oSymTable->uLength;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    size_t index;
    Binding *pCurrent;
    Binding *pNew;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Check if key already exists in this bucket */
    for (pCurrent = oSymTable->ppBuckets[index]; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0)
            return 0;
    }
    
    /* Allocate memory for new binding */
    pNew = malloc(sizeof(Binding));
    if (pNew == NULL)
        return 0;
    
    /* Allocate memory for defensive copy of key */
    pNew->pcKey = malloc(strlen(pcKey) + 1);
    if (pNew->pcKey == NULL) {
        free(pNew);
        return 0;
    }
    
    /* Create defensive copy of the key */
    strcpy(pNew->pcKey, pcKey);
    
    /* Store the value pointer (no defensive copy) */
    pNew->pvValue = pvValue;
    
    /* Insert at the head of the bucket's list */
    pNew->pNext = oSymTable->ppBuckets[index];
    oSymTable->ppBuckets[index] = pNew;
    
    /* Increment the binding count */
    oSymTable->uLength++;
    
    /* Check if expansion is needed (bindings > buckets) */
    if (oSymTable->uLength > oSymTable->uBucketCount)
        expandTable(oSymTable);
    
    return 1;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    size_t index;
    Binding *pCurrent;
    const void *pvOld;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Search for the key in this bucket */
    for (pCurrent = oSymTable->ppBuckets[index]; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0) {
            /* Key found, save the old value */
            pvOld = pCurrent->pvValue;
            
            /* Replace with new value */
            pCurrent->pvValue = pvValue;
            
            return (void *)pvOld;
        }
    }
    
    return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    size_t index;
    Binding *pCurrent;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Search for the key in this bucket */
    for (pCurrent = oSymTable->ppBuckets[index]; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0)
            return 1;
    }
    
    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    size_t index;
    Binding *pCurrent;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Search for the key in this bucket */
    for (pCurrent = oSymTable->ppBuckets[index]; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0)
            return (void *)pCurrent->pvValue;
    }
    
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    size_t index;
    Binding *pCurrent;
    Binding *pPrev = NULL;
    const void *pvValue;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Compute hash bucket index for this key */
    index = SymTable_hash(pcKey, oSymTable->uBucketCount);
    
    /* Search for the key in this bucket */
    for (pCurrent = oSymTable->ppBuckets[index]; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0) {
            /* Key found, remove the binding */
            
            /* Handle case where binding is at the head of bucket */
            if (pPrev == NULL)
                oSymTable->ppBuckets[index] = pCurrent->pNext;
            else
                pPrev->pNext = pCurrent->pNext;
            
            /* Save the value to return */
            pvValue = pCurrent->pvValue;
            
            /* Free the key string */
            free(pCurrent->pcKey);
            
            /* Free the binding structure */
            free(pCurrent);
            
            /* Decrement the binding count */
            oSymTable->uLength--;
            
            return (void *)pvValue;
        }
        
        /* Keep track of previous binding for list manipulation */
        pPrev = pCurrent;
    }
    
    return NULL;
}

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
                  const void *pvExtra) {
    size_t i;
    Binding *pCurrent;
    
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    
    /* Process each bucket */
    for (i = 0; i < oSymTable->uBucketCount; i++) {
        for (pCurrent = oSymTable->ppBuckets[i]; pCurrent != NULL; pCurrent = pCurrent->pNext)
            pfApply(pCurrent->pcKey, (void *)pCurrent->pvValue, (void *)pvExtra);
    }
}
