/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* SIZE is the size of the hash table */
#define SIZE 211

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec {
    int lineno;
    struct LineListRec * next;
} * LineList;

typedef struct BucketListRec {
    char * name;
    LineList lines;
    TreeNode *treeNode;
    int memloc;
    struct BucketListRec * next;
} * BucketList;

typedef struct ScopeRec {
    char * funcName;
    int nestedLevel;
    struct ScopeRec * parent;
    int cntLoc;
    BucketList hashTable[SIZE];
    int childCnt;
    TreeNode *treeNode;
} * Scope;

// scope
Scope sc_create( char * funcName );
Scope sc_top( void );
void sc_push( Scope scope );
void sc_pop( void );

// symbol table
/* Function st_lookup returns the memory 
* location of a variable or -1 if not found
*/
/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
BucketList st_bucket ( char * name );
void st_insert( char * name, int lineno, int loc, TreeNode * treeNode );
int st_lookup ( char * name );
int st_exist_top ( char * name );
int st_add_lineno ( char * name, int lineno );

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
