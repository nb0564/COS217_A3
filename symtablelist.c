/* symtablelist.c - Implementation of the SymTable ADT using a linked list */

#include <assert.h>  /* For assert() */
#include <stdlib.h>  /* For malloc(), free() */
#include <string.h>  /* For strcmp(), strcpy(), strlen() */
#include "symtable.h"  /* For SymTable ADT interface */

/* 
 * A Binding structure represents a single key-value binding in the table.
 * Each node in the linked list is a Binding.
 */
typedef struct Binding {
    char *pcKey;             /* Defensive copy of the key string */
    const void *pvValue;     /* Value associated with the key (client-owned) */
    struct Binding *pNext;   /* Pointer to the next binding in the list */
} Binding;

/* 
 * The SymTable structure represents the entire symbol table.
 * It maintains a pointer to the first binding and a count of bindings.
 */
struct SymTable {
    Binding *pHead;          /* Head of the linked list of bindings */
    size_t uLength;          /* Number of bindings in the table */
};

/* 
 * Creates a new empty symbol table.
 * Returns NULL if insufficient memory is available.
 */
SymTable_T SymTable_new(void) {
    /* Allocate memory for the SymTable structure */
    SymTable_T oSymTable = malloc(sizeof(struct SymTable));
    
    /* Return NULL if memory allocation failed */
    if (oSymTable == NULL)
        return NULL;
    
    /* Initialize the empty table with no bindings */
    oSymTable->pHead = NULL;  /* No bindings yet */
    oSymTable->uLength = 0;   /* Length starts at 0 */
    
    return oSymTable;
}

/* 
 * Frees all memory occupied by oSymTable, including all keys.
 * Does not free memory occupied by the values.
 */
void SymTable_free(SymTable_T oSymTable) {
    Binding *pCurr;  /* Current binding being processed */
    Binding *pTemp;  /* Temporary pointer to hold binding before freeing */
    
    /* Validate input parameter */
    assert(oSymTable != NULL);
    
    /* Start traversal at the head of the list */
    pCurr = oSymTable->pHead;
    
    /* Traverse the entire list, freeing each binding */
    while (pCurr != NULL) {
        /* Save current binding to free after advancing pointer */
        pTemp = pCurr;
        
        /* Advance to next binding */
        pCurr = pCurr->pNext;
        
        /* Free the key string (which we own) */
        free(pTemp->pcKey);
        
        /* Free the binding structure itself */
        free(pTemp);
    }
    
    /* Finally, free the SymTable structure */
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
    Binding *pNew;   /* Will point to the new binding to be added */
    Binding *pCurr;  /* For traversing the list to check for duplicates */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Check if the key already exists (duplicate keys not allowed) */
    for (pCurr = oSymTable->pHead; pCurr != NULL; pCurr = pCurr->pNext) {
        if (strcmp(pCurr->pcKey, pcKey) == 0)
            return 0;  /* Key already exists, return FALSE */
    }
    
    /* Allocate memory for new binding */
    pNew = malloc(sizeof(Binding));
    if (pNew == NULL)
        return 0;  /* Memory allocation failed, return FALSE */
    
    /* Allocate memory for the defensive copy of the key */
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
    
    /* Insert at the beginning of the list (prepend) for O(1) insertion */
    pNew->pNext = oSymTable->pHead;
    oSymTable->pHead = pNew;
    
    /* Increment the binding count */
    oSymTable->uLength++;
    
    return 1;  /* Success, return TRUE */
}

/* 
 * Replaces the value of an existing binding.
 * Returns the old value if successful, NULL if key doesn't exist.
 */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    Binding *pCurr;      /* For traversing the list */
    const void *pvOld;   /* To store the old value */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Search for the key in the list */
    for (pCurr = oSymTable->pHead; pCurr != NULL; pCurr = pCurr->pNext) {
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
    Binding *pCurr;  /* For traversing the list */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Search for the key in the list */
    for (pCurr = oSymTable->pHead; pCurr != NULL; pCurr = pCurr->pNext) {
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
    Binding *pCurr;  /* For traversing the list */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Search for the key in the list */
    for (pCurr = oSymTable->pHead; pCurr != NULL; pCurr = pCurr->pNext) {
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
    Binding *pCurr;      /* Current binding in traversal */
    Binding *pPrev = NULL;  /* Previous binding (initially NULL) */
    const void *pvValue;  /* Value to return */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Search for the key in the list */
    for (pCurr = oSymTable->pHead; pCurr != NULL; pCurr = pCurr->pNext) {
        if (strcmp(pCurr->pcKey, pcKey) == 0) {
            /* Key found, remove the binding */
            
            /* Handle case where binding is at the head */
            if (pPrev == NULL)
                oSymTable->pHead = pCurr->pNext;
            else
                /* Handle case where binding is in the middle/end */
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
    Binding *pCurr;  /* For traversing the list */
    
    /* Validate input parameters */
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    
    /* Traverse the list, applying the function to each binding */
    for (pCurr = oSymTable->pHead; pCurr != NULL; pCurr = pCurr->pNext)
        /* Call the client-provided function with key, value, and extra data */
        pfApply(pCurr->pcKey, (void *)pCurr->pvValue, (void *)pvExtra);
}