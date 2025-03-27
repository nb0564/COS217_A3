/* Author: Nicholas Budny */

/* symtablelist.c - Implementation of the SymTable ADT using a linked list */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

typedef struct Binding {
    /* Defensive copy of the key string */
    char *pcKey;
    /* Value associated with the key (client-owned) */
    const void *pvValue;
    /* Pointer to the next binding in the list */
    struct Binding *pNext;
} Binding;

struct SymTable {
    /* Head of the linked list of bindings */
    Binding *pHead;
    /* Number of bindings in the table */
    size_t uLength;
};

SymTable_T SymTable_new(void) {
    /* Allocate memory for the SymTable structure */
    SymTable_T oSymTable = malloc(sizeof(struct SymTable));
    
    if (oSymTable == NULL)
        return NULL;
    
    /* Initialize the empty table with no bindings */
    oSymTable->pHead = NULL;
    oSymTable->uLength = 0;
    
    return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
    Binding *pCurrent;
    Binding *pTemp;
    
    assert(oSymTable != NULL);
    
    /* Start traversal at the head of the list */
    pCurrent = oSymTable->pHead;
    
    /* Traverse the entire list, freeing each binding */
    while (pCurrent != NULL) {
        /* Save current binding before advancing pointer */
        pTemp = pCurrent;
        pCurrent = pCurrent->pNext;
        
        /* Free the key string */
        free(pTemp->pcKey);
        
        /* Free the binding structure itself */
        free(pTemp);
    }
    
    /* Finally, free the SymTable structure */
    free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    
    return oSymTable->uLength;
}

int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    Binding *pNew;
    Binding *pCurrent;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Check if the key already exists (duplicate keys not allowed) */
    for (pCurrent = oSymTable->pHead; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0)
            return 0;
    }
    
    /* Allocate memory for new binding */
    pNew = malloc(sizeof(Binding));
    if (pNew == NULL)
        return 0;
    
    /* Allocate memory for the defensive copy of the key */
    pNew->pcKey = malloc(strlen(pcKey) + 1);
    if (pNew->pcKey == NULL) {
        free(pNew);
        return 0;
    }
    
    /* Create defensive copy of the key */
    strcpy(pNew->pcKey, pcKey);
    
    /* Store the value pointer (no defensive copy) */
    pNew->pvValue = pvValue;
    
    /* Insert at the beginning of the list (prepend) */
    pNew->pNext = oSymTable->pHead;
    oSymTable->pHead = pNew;
    
    /* Increment the binding count */
    oSymTable->uLength++;
    
    return 1;
}

void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue) {
    Binding *pCurrent;
    const void *pvOld;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Search for the key in the list */
    for (pCurrent = oSymTable->pHead; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0) {
            /* Key found, save the old value */
            pvOld = pCurrent->pvValue;
            
            /* Replace with new value */
            pCurrent->pvValue = pvValue;
            
            return (void *)pvOld;
        }
    }
    
    /* Key not found */
    return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    Binding *pCurrent;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Search for the key in the list */
    for (pCurrent = oSymTable->pHead; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0)
            return 1;
    }
    
    return 0;
}

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    Binding *pCurrent;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Search for the key in the list */
    for (pCurrent = oSymTable->pHead; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0)
            return (void *)pCurrent->pvValue;
    }
    
    return NULL;
}

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    Binding *pCurrent;
    Binding *pPrev = NULL;
    const void *pvValue;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    
    /* Search for the key in the list */
    for (pCurrent = oSymTable->pHead; pCurrent != NULL; pCurrent = pCurrent->pNext) {
        if (strcmp(pCurrent->pcKey, pcKey) == 0) {
            
            /* Handle case where binding is at the head */
            if (pPrev == NULL)
                oSymTable->pHead = pCurrent->pNext;
            else
                pPrev->pNext = pCurrent->pNext;
            
            /* Save the value to return */
            pvValue = pCurrent->pvValue;
            
            /* Free the key string */
            free(pCurrent->pcKey);
            
            /* Free the binding structure */
            free(pCurrent);
            
            /* Decrement the count of bindings */
            oSymTable->uLength--;
            
            return (void *)pvValue;
        }
        
        /* Keep track of previous binding */
        pPrev = pCurrent;
    }
    
    return NULL;
}

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
                  const void *pvExtra) {
    Binding *pCurrent;
    
    assert(oSymTable != NULL);
    assert(pfApply != NULL);
    
    /* Traverse the list and apply the function to each binding */
    for (pCurrent = oSymTable->pHead; pCurrent != NULL; pCurrent = pCurrent->pNext)
        pfApply(pCurrent->pcKey, (void *)pCurrent->pvValue, (void *)pvExtra);
}