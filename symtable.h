/* Author: Nicholas Budny */

/* symtable.h - declaration of the SymTable Abstract Data Type (ADT) */

#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <stddef.h>

/* SymTable_T is an opaque pointer to a symbol table. 
 * A symbol table is a collection of bindings where each binding 
 * associates a string key with an arbitrary value. 
 */
typedef struct SymTable *SymTable_T;

/* Creates and returns a new empty symbol table.
 * Returns NULL if insufficient memory is available.
 */
SymTable_T SymTable_new(void);

/* Frees all memory occupied by oSymTable, including all keys.
 * Does not free memory occupied by the values stored in the table.
 * oSymTable must not be NULL.
 */
void SymTable_free(SymTable_T oSymTable);

/* Returns the number of bindings in oSymTable.
 * oSymTable must not be NULL.
 */
size_t SymTable_getLength(SymTable_T oSymTable);

/* Adds a new binding to oSymTable consisting of key pcKey and value pvValue.
 * Makes a defensive copy of pcKey.
 * Returns 1 (true) if the binding was added successfully.
 * Returns 0 (false) if pcKey already exists in oSymTable or if
 * insufficient memory is available.
 * oSymTable and pcKey must not be NULL.
 */
int SymTable_put(SymTable_T oSymTable, const char *pcKey, const void *pvValue);

/* Replaces the value in the binding in oSymTable with key pcKey
 * with the new value pvValue.
 * Returns the old value if successful.
 * Returns NULL if pcKey does not exist in oSymTable.
 * oSymTable and pcKey must not be NULL.
 */
void *SymTable_replace(SymTable_T oSymTable, const char *pcKey, const void *pvValue);

/* Checks if pcKey exists in oSymTable.
 * Returns 1 (true) if the key exists, 0 (false) otherwise.
 * oSymTable and pcKey must not be NULL.
 */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/* Returns the value associated with key pcKey in oSymTable.
 * Returns NULL if no such binding exists.
 * oSymTable and pcKey must not be NULL.
 */
void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/* Removes the binding with key pcKey from oSymTable.
 * Returns the value of the removed binding if successful.
 * Returns NULL if no such binding exists.
 * oSymTable and pcKey must not be NULL.
 */
void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/* Applies function pfApply to each binding in oSymTable.
 * For each binding, calls pfApply(pcKey, pvValue, pvExtra).
 * oSymTable and pfApply must not be NULL.
 */
void SymTable_map(SymTable_T oSymTable,
     void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
     const void *pvExtra);

#endif