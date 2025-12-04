/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

static Scope globalScope = NULL;
// static char * currFuncName = NULL;
// static Type currFuncType = Void;
static int preserveLastScope = FALSE;
static int skipRedfinedFuncDecl = FALSE;
static int skipAll = FALSE;

/* counter for variable memory locations */
// static int location;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */

/*
static void swap(int *a, int *b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}
*/
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) ) {
  if (t != NULL)
  { preProc(t);
    { int i;
      for (i = 0; i < MAXCHILDREN; i++) {
        traverse(t->child[i],preProc,postProc);
      }
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

static void insertIOFunc(void) {
  TreeNode *func;
  TreeNode *params;
  TreeNode *compStmt;

  // input
  func = newDeclNode(FuncK);
  func->type = Integer;
  func->lineno = 0;

  params = newParamNode(VoidParamK);
  params->type = Void;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;      // no local var
  compStmt->child[1] = NULL;      // no stmt

  func->lineno = 0;
  func->attr.name = "input";
  func->child[0] = params;
  func->child[1] = compStmt;
  func->scope = globalScope;
  // st_insert(func->attr.name, func->lineno, location++, func);
  st_insert(func->attr.name, func->lineno, sc_top()->cntLoc++, func);

  // output
  func = newDeclNode(FuncK);
  func->type = Void;
  func->lineno = 0;
  
  params = newParamNode(NonArrParamK);
  params->type = Integer;
  params->attr.name = "value";

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;      // no local var
  compStmt->child[1] = NULL;      // no stmt

  func->lineno = 0;
  func->attr.name = "output";
  func->child[0] = params;
  func->child[1] = compStmt;
  func->scope = globalScope;
  // st_insert(func->attr.name, func->lineno, location++, func);
  st_insert(func->attr.name, func->lineno, sc_top()->cntLoc++, func);


  Scope outputsc = sc_create(func->attr.name);
  params->scope = outputsc;
  sc_push(outputsc);

  // swap(&location, &(params->scope->cntLoc));
  // st_insert(params->attr.name, func->lineno, location++, params);
  st_insert(params->attr.name, func->lineno, sc_top()->cntLoc++, params);
  // swap(&location, &(params->scope->cntLoc));

  sc_pop();
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
// static void nullProc(TreeNode * t) {
//   if (t==NULL) return;
//   else return;
// }

// static void symbolError(TreeNode * t, char * message) {
//   fprintf(listing,"Symbol error at line %d: %s\n",t->lineno,message);
//   Error = TRUE;
// }

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static char * newStmtScopeName(char *curr, int childCnt) {
  int logChildCnt = 2; int temp = childCnt;
  while (temp) { logChildCnt++; temp /= 10; }
  int len = sizeof(char) * strlen(curr) + logChildCnt;
  char *childCntStr = (char*)malloc(sizeof(char) * logChildCnt);
  snprintf(childCntStr, len, "%d", childCnt);

  char *newFuncName = (char*)malloc(len);
  strcpy(newFuncName, curr);
  strcat(newFuncName, ".");
  strcat(newFuncName, childCntStr);
  return newFuncName;
}

static void insertNode(TreeNode *t) {
  if (t == NULL) return;

  switch (t->nodekind) {
    case StmtK:
    switch (t->kind.stmt) {
      case IterK:
      // currFuncName = newStmtScopeName(sc_top()->funcName, "while");
      // newFuncName = (char*)malloc(sizeof(char) * strlen(currFuncName)+(6));
      // strcpy(newFuncName, currFuncName);
      // strcat(newFuncName, ".while");
      // currFuncName = newFuncName;
      case IfK:
      case IfElseK:
      // currFuncName = newStmtScopeName(sc_top()->funcName, "if");
      break;
      
      case CompK:
      if (preserveLastScope) {
        t->scope = sc_top();
        preserveLastScope = FALSE;
      } else if (skipRedfinedFuncDecl) {
        skipRedfinedFuncDecl = FALSE; // sex
        // skipAll = TRUE;
      } else {
        char *newScopeName = newStmtScopeName(sc_top()->funcName, sc_top()->childCnt);
        Scope sc = sc_create(newScopeName);
        sc_push(sc);
        t->scope = sc;
        // swap(&location, &(t->scope->cntLoc));
      }
      // int location_bak = location;
      // location = t->scope->cntLoc;
      // t->scope->cntLoc = location_bak;
      break;
      default:
      break;
    }
    break;
    
    case ExpK:
    switch (t->kind.exp) {
      case NonArrIdK:
      case ArrIdK:
      if (st_lookup(t->attr.name) == -1) {
        // symbolError(t, "undeclared symbol");
        fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", t->attr.name, t->lineno);

        TreeNode *implicitDecl;
        if (t->kind.exp == NonArrIdK) implicitDecl = newDeclNode(NonArrVarK);
        else if (t->kind.exp == ArrIdK) implicitDecl = newDeclNode(ArrVarK);
        implicitDecl->attr.name = t->attr.name;
        implicitDecl->type = Undetermined;
        implicitDecl->lineno = t->lineno;

        t->type = Undetermined;
        st_insert(implicitDecl->attr.name, implicitDecl->lineno, sc_top()->cntLoc++, implicitDecl);
        Error = TRUE; // implicitly declaration
      } else {
        st_add_lineno(t->attr.name, t->lineno);
      }
      break;
      case CallK:
      if (st_lookup(t->attr.name) == -1) {
        // symbolError(t, "undeclared symbol");
        fprintf(listing, "Error: undeclared function \"%s\" is used at line %d\n", t->attr.name, t->lineno);

        TreeNode *implicitDecl = newDeclNode(FuncK);
        implicitDecl->attr.name = t->attr.name;
        implicitDecl->type = Undetermined;
        implicitDecl->lineno = t->lineno;

        t->type = Undetermined;
        st_insert(implicitDecl->attr.name, implicitDecl->lineno, sc_top()->cntLoc++, implicitDecl);
        Error = TRUE; // implicitly declaration
      } else {
        st_add_lineno(t->attr.name, t->lineno);
      }
      break;

      default:
      break;
    }
    break;

    case DeclK:
    switch (t->kind.decl) {
      case FuncK:
      char *funcName = t->attr.name;

      if (st_exist_top(funcName)) {
      // if (st_lookup(funcName) == -1) {
        // symbolError(t, "function already declared");
        fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line ", funcName, t->lineno);
        BucketList b = st_bucket(funcName);
        fprintf(listing, "%d)\n", b->treeNode->lineno);

        skipRedfinedFuncDecl = TRUE;
        Error = TRUE;
      } else {
        // currFuncName = funcName;
        Scope funcScope = sc_create(funcName);
        t->scope = funcScope;

        // st_insert(funcName, t->lineno, location++, t);
        st_insert(funcName, t->lineno, sc_top()->cntLoc++, t);
        sc_push(t->scope);
        // swap(&location, &(t->scope->cntLoc));

        preserveLastScope = TRUE;
      }
      break;

      case NonArrVarK:
      case ArrVarK:
      char *name = t->attr.name;

      if (t->type == Void) {
        // symbolError(t, "variable should have non-void type");
        fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
        Error = TRUE;
      } else {
        if (t->kind.decl == NonArrVarK) {
          name = t->attr.name;
          t->type = Integer;
        } else {
          name = t->attr.name;
          t->type = IntegerArray;
        }

        if (st_exist_top(name)) {
          // symbolError(t, "symbol already declared in this scope");
          fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line ", name, t->lineno);
          BucketList b = st_bucket(name);
          fprintf(listing, "%d)\n", b->treeNode->lineno);
          Error = TRUE;
        } else {
          // st_insert(name, t->lineno, location++, t);
          st_insert(name, t->lineno, sc_top()->cntLoc++, t);
        }
        break;
      }

      default:
      break;
    }
    break;

    case ParamK:
    if (skipRedfinedFuncDecl) break;
    switch (t->kind.param) {
      case VoidParamK:
        t->type = Void;
        /* void parameter */
      break;

      case NonArrParamK:
      case ArrParamK:
      // if (t->type == Void) {
      //   symbolError(t, "void type parameter is not allowed");
      // }
      if (st_lookup(t->attr.name) == -1) {
        // st_insert(t->attr.name,t->lineno,location++,t);
        st_insert(t->attr.name,t->lineno,sc_top()->cntLoc++,t);
        if (t->type == Integer) {
          if (t->kind.param == NonArrParamK)
            t->type = Integer;
          else if (t->kind.param == ArrParamK)
            t->type = IntegerArray;
        } else {
          fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
          Error = TRUE;
        }
      } else { // goodd
        fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line ...)", t->attr.name, t->lineno);
        Error = TRUE;
      }
      break;
    }
    break;

    default:
    break;
  }
}

static void postInsertNode(TreeNode *t) {
  switch (t->nodekind) {
    case StmtK:
    switch (t->kind.stmt) {
      case CompK:
        sc_pop();
        // swap(&location, &(t->scope->cntLoc));
        break;
      default:
        break;
    }
    break;
  default:
    break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree) {
  globalScope = sc_create(NULL);
  globalScope->cntLoc = 0;
  sc_push(globalScope);
  // swap(&location, &(globalScope->cntLoc));
  // int location_bak = location;
  // location = globalScope->cntLoc;
  // globalScope->cntLoc = location_bak;

  insertIOFunc();

  traverse(syntaxTree,insertNode,postInsertNode);
  sc_pop();
  // swap(&location, &(globalScope->cntLoc));
  // location_bak = globalScope->cntLoc;
  // globalScope->cntLoc = location;
  // location = location_bak;

  if (TraceAnalyze) {
    // fprintf(listing,"\nSymbol table:\n\n");
    fprintf(listing, "\n\n");
    printSymTab(listing);
  }
}

// static void typeError(TreeNode * t, char * message) {
//   fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
//   Error = TRUE;
// }

static void preTypeCheck(TreeNode *t) {
  if (t == NULL) return;

  switch (t->nodekind) {
    case DeclK:
    switch (t->kind.decl) {
      case FuncK:
      // currFuncName = t->attr.name;
      break;
      default:
      break;
    }
    break;
    case StmtK:
    switch (t->kind.stmt) {
      case CompK:
      sc_push(t->scope);
      break;
      default:
      break;
    }
    default:
    break;
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode *t) {
  if (t == NULL) return;

  BucketList b;

  switch (t->nodekind) {
    case DeclK:
    // if (t->kind.decl == FuncK) currFuncName = t->attr.name;
    break;
    
    case StmtK:
    switch (t->kind.stmt) {
      case CompK:
      sc_pop();
      break;

      case IterK:
      case IfK:
      case IfElseK:
      /* if (t->child[0]->type == Void)
        // typeError(t->child[0], "if condition has void value"); */
      if (t->child[0]->type != Integer) {
        fprintf(listing, "Error: invalid condition at line %d\n", t->child[0]->lineno);
        Error = TRUE;
      }
      break;

      case RetK:
      // const TreeNode * funcDecl = st_bucket(currFuncName)->treeNode;
      const TreeNode * funcDecl = st_bucket(sc_top()->funcName)->treeNode;
      const Type funcType = funcDecl->type;
      // TreeNode *expr = t->child[0]; // return value

      if (funcType == Void) {
        if (t->child[0] == NULL /*|| t->child[0]->type == Void*/) break;
      } else if (funcType == Integer) {
        if (t->child[0] != NULL && t->child[0]->type == Integer) break;
      }
      fprintf(listing, "Error: Invalid return at line %d\n", t->lineno);
      Error = TRUE;
      break;

      /* if (funcType == Void) {
        if (expr == NULL || expr->type == Void) ;
        // else typeError(t, "expected no return value");
        else fprintf(listing, "Error: Invalid return at line %d\n", t->lineno);
      } else if (funcType == Integer) {
        if (expr != NULL && expr->type == Integer) ;
        // else typeError(t, "expected return value of type int");
        else fprintf(listing, "Error: Invalid return at line %d\n", t->lineno);
      } */
      break;
      
      default:
        break;
      }
    break;

    case ExpK:
      switch (t->kind.exp) {
      case AssignK:
        TreeNode *lhs = t->child[0]; // var
        TreeNode *rhs = t->child[1];

        if (lhs->type == Integer) {
          if (rhs->type == Integer) {
            t->type = Integer;
          }
        } else if (lhs->type == IntegerArray) {
          if (rhs->type == IntegerArray) {
            t->type = IntegerArray;
          }
        } else {
          fprintf(listing, "Error: invalid assignment at line %d\n", t->lineno);
          Error = TRUE;
        }
        break;

      case OpK:
        Type leftType = t->child[0]->type;
        Type rightType = t->child[1]->type;
        if (leftType == Integer && rightType == Integer) {
          t->type = Integer;
        } else {
          fprintf(listing, "Error: invalid operation at line %d\n", t->lineno);
          Error = TRUE;
        }
        break;

      case ConstK:
        t->type = Integer;
        break;

      case NonArrIdK:
        b = st_bucket(t->attr.name);
        if (b == NULL) {
          t->type = Void;
          break; // seolbim
        }

        if (b->treeNode->type == IntegerArray) {
          t->type = IntegerArray;
        } else if (b->treeNode->type == Integer) {
          t->type = Integer;
        } else {
          // NonArrIdK error message
        }
        break;

      case ArrIdK:
        b = st_bucket(t->attr.name);
        if (b == NULL) break;

        if (b->treeNode->type == IntegerArray) {
          if (t->child[0] != NULL && t->child[0]->type == Integer) {

          } else {
            fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indices should be integer\n", t->lineno, t->attr.name);
            Error = TRUE;
          }
        } else {
          fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", t->lineno, t->attr.name);
          Error = TRUE;
        }
        t->type = Integer;
        break;

      case CallK:
        b = st_bucket(t->attr.name);
        if (b == NULL) break;

        if (b->treeNode->nodekind == DeclK &&
            b->treeNode->kind.decl == FuncK) {
          int status = 0;
          TreeNode *declParam = b->treeNode->child[0];
          TreeNode *useParam = t->child[0];
          int declParamCnt = 0;
          int useParamCnt = 0;

          while (declParam != NULL) {
            declParamCnt++; declParam = declParam->sibling;}
          while (useParam != NULL) {
            useParamCnt++; useParam = useParam->sibling;}

          if (useParamCnt == 0) {
            if (declParamCnt == 1 && b->treeNode->child[0]->type == Void);
            else status = 1;
          } else {
            if (declParamCnt != useParamCnt) status = 1;
            else {
              while (declParam != NULL && useParam != NULL) {
                if (useParam->type != Void) {
                  if (useParam->type == declParam->type) ;
                  else status = 1;
                } else status = 1;

                declParam = declParam->sibling;
                useParam = useParam->sibling;
              }
              if (declParam != NULL || useParam != NULL) status = 1;
            }
          }

          if (status) {
            fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
            Error = TRUE;
          }
          t->type = b->treeNode->type;
        } else {
          fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
          Error = TRUE;
        }
        break;

      default:
        break;
      }
      break;

    default:
    break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *syntaxTree) {
  sc_push(globalScope);
  // swap(&location, &(globalScope->cntLoc));
  traverse(syntaxTree, preTypeCheck, checkNode);
  sc_pop();
  // swap(&location, &(globalScope->cntLoc));
}
