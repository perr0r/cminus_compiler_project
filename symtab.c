/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

#define MAX_SCOPE 1000

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

// Scope
static Scope scopes[MAX_SCOPE];
static int scopeTotal = 0;
static Scope scopeStack[MAX_SCOPE];
static int scopeStackTop = 0;

Scope sc_top( void ) {
  if (scopeStackTop == 0) return NULL;
  return scopeStack[scopeStackTop-1];
}

void sc_push( Scope scope ) {
  if (scopeStackTop+1 < MAX_SCOPE) scopeStack[scopeStackTop++] = scope;
  //printf("push %s\n", scope->funcName);
}

void sc_pop( void ) {
  //printf("pop %s\n", sc_top()->funcName);
  if (scopeStackTop > 0) scopeStackTop--;
}

Scope sc_create(char *funcName) {
  int i;

  Scope newScope = (Scope)malloc(sizeof(struct ScopeRec));
  newScope->funcName = funcName;
  newScope->nestedLevel = scopeStackTop;
  newScope->parent = sc_top();
  newScope->cntLoc = 0;
  newScope->childCnt = 0;
  for (i = 0; i < SIZE; ++i) newScope->hashTable[i] = NULL;

  scopes[scopeTotal++] = newScope;

  return newScope;
}

// symbol table
/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
BucketList st_bucket( char * name ) {
  int h = hash(name);
  Scope sc = sc_top();

  while(sc) {
    BucketList l = sc->hashTable[h];

    while ((l != NULL) && (strcmp(name,l->name) != 0)) l = l->next;
    if (l != NULL) return l;
    sc = sc->parent;
  }
  return NULL;
}

void st_insert( char * name, int lineno, int loc, TreeNode * treeNode ) { // treeNode: func or cmpstmt
  int h = hash(name);
  Scope top = sc_top();
  BucketList l = top->hashTable[h];

  while ((l != NULL) && (strcmp(name,l->name) != 0)) l = l->next;

  if (l == NULL) { // variable not yet in this scope (= table)
    l = (BucketList)malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->treeNode = treeNode;
    l->memloc = loc;

    l->lines = (LineList)malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->lines->next = NULL;

    l->next = top->hashTable[h];
    top->hashTable[h] = l;
  } else { // found in table, so just add line numebr to lines
    st_add_lineno(name, lineno);
    // LineList t = l->lines;
    // while (t->next != NULL) t = t->next;
    // t->next = (LineList) malloc(sizeof(struct LineListRec));
    // t->next->lineno = lineno;
    // t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup( char * name ) {
  BucketList l = st_bucket(name);
  if (l != NULL) return l->memloc;
  return -1;
}

int st_exist_top (char * name) {
  Scope top = sc_top();
  int h = hash(name);
  
  if (top == NULL) return FALSE;

  BucketList l = top->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0)) l = l->next;
  
  if (l != NULL) return TRUE;
  return FALSE;
}

int st_add_lineno(char * name, int lineno) {
  BucketList l = st_bucket(name);
  if (l == NULL) return 0;

  LineList ll = l->lines;
  while (ll->next != NULL) ll = ll->next;

  ll->next = (LineList) malloc(sizeof(struct LineListRec));
  ll->next->lineno = lineno;
  ll->next->next = NULL;
  return 1;
}

// print
static const char *symKindStr(TreeNode *node) {
  // if (node == NULL) return "Unknown";

  if (node->nodekind == DeclK) {
    switch (node->kind.decl) {
      case FuncK:      return "Function";
      case NonArrVarK:
      case ArrVarK:    return "Variable";
      // default:         return "Variable";
    }
  } else if (node->nodekind == ParamK) {
    switch (node->kind.param) {
      case NonArrParamK:
      case ArrParamK:    return "Variable";
      case VoidParamK:   return ""; // "VoidParam";
      // default:           return // "Param";
    }
  }
  return "ERROR_KIND";
  // return "Variable";
}

static const char *typeStr(Type t) {
  switch (t) {
    case Void:          return "void";
    case Integer:       return "int";
    case IntegerArray:  return "int[]";
    case Undetermined:  return "undetermined";
  }
  return "ERROR_TYPE";
}

void printSymTab(FILE * listing) {
  int i, j;

  // Symbol Table
  fprintf(listing, "< Symbol Table >\n");
  fprintf(listing," Symbol Name   Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
  fprintf(listing,"-------------  -----------  -------------  ------------  --------  ------------\n");
  for (i = 0; i < scopeTotal; i++) {
    Scope sc = scopes[i];
    const char *scopeName = (sc->funcName == NULL) ? "global" : sc->funcName;

    for (j = 0; j < SIZE; ++j) {
      BucketList l = sc->hashTable[j];
      if (l == NULL) continue;

      while (l != NULL) {
        LineList t = l->lines;
        TreeNode *node = l->treeNode;

        fprintf(listing, "%-13s  %-11s  %-13s  %-12s  %-9d",
                          l->name,
                          symKindStr(node),
                          typeStr(node->type),
                          scopeName,
                          l->memloc);
        while (t != NULL) {
          fprintf(listing, " %3d ", t->lineno);
          t = t->next;
        }
        fprintf(listing, "\n");

        l = l->next;
      }
    }
  }
  fprintf(listing, "\n\n");

  // Functions
  fprintf(listing, "< Functions >\n");
  fprintf(listing,"Function Name   Return Type   Parameter Name  Parameter Type\n");
  fprintf(listing,"-------------  -------------  --------------  --------------\n");
  for (i = 0; i < scopeTotal; i++) {
    Scope sc = scopes[i];

    for (j = 0; j < SIZE; ++j) {
      BucketList l = sc->hashTable[j];
      if (l == NULL) continue;

      while (l != NULL) {
        TreeNode *node = l->treeNode;

        if (node->nodekind == DeclK) {
          if (node->kind.decl == FuncK) {
            if (node->child[0] == NULL) {
              fprintf(listing, "%-13s  %-14s  %-13s  %-12s\n", 
                                l->name == NULL ? "" : l->name,
                                typeStr(node->type),
                                "",
                                "undetermined");
            } else if (node->child[0]->type == Void && node->child[0]->attr.name == NULL) {
              fprintf(listing, "%-13s  %-14s  %-13s  %-12s\n", 
                                l->name == NULL ? "" : l->name,
                                typeStr(node->type),
                                "",
                                "void");
            } else {
              fprintf(listing, "%-13s  %-14s\n", 
                                l->name == NULL ? "" : l->name,
                                typeStr(node->type));

              TreeNode *param = node->child[0];
              while (param != NULL) {
                fprintf(listing, "%-13s  %-13s  %-14s  %-12s\n",
                                  "-",
                                  "-",
                                  param->attr.name,
                                  typeStr(param->type));
                param = param->sibling;
              }
            }

          }
        }
        l = l->next;
      }
    }
  }
  fprintf(listing, "\n\n");

  // Global Symbols
  fprintf(listing, "< Global Symbols >\n");
  fprintf(listing," Symbol Name   Symbol Kind   Symbol Type\n");
  fprintf(listing,"-------------  -----------  -------------\n");
  
  Scope sc = scopes[0];
  for (j = 0; j < SIZE; ++j) {
    BucketList l = sc->hashTable[j];
    if (l == NULL) continue;

    while (l != NULL) {
      TreeNode *node = l->treeNode;
      if (node->nodekind == DeclK) {
        fprintf(listing, "%-13s  %-11s  %-13s",
                          l->name,
                          symKindStr(node),
                          typeStr(node->type));
        fprintf(listing, "\n");
      }

      l = l->next;
    }
  }
  fprintf(listing, "\n\n");

  // Scopes
  fprintf(listing, "< Scopes >\n");
  fprintf(listing," Scope Name   Nested Level   Symbol Name   Symbol Type\n");
  fprintf(listing,"------------  ------------  -------------  -----------\n");
  for (i = 1; i < scopeTotal; i++) {
    Scope sc = scopes[i];

    int status = 0;
    for (j = 0; j < SIZE; ++j) {
      BucketList l = sc->hashTable[j];
      if (l == NULL) continue;

      status = 1;
      while (l != NULL) {
        TreeNode *node = l->treeNode;

        fprintf(listing, "%-12s  %-12d  %-13s  %-11s",
                          sc->funcName,
                          sc->nestedLevel,
                          l->name,
                          typeStr(node->type));
        fprintf(listing, "\n");

        l = l->next;
      }
    }

    if (status) fprintf(listing, "\n");
  }
}