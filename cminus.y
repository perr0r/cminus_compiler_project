/****************************************************/
/* File: cminus.y                                   */
/* The CMINUS Yacc/Bison specification file         */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static TreeNode * savedTree;
static char * savedName;
static int savedNumber;
static int savedLineNo;
static Type savedType;
static TokenType savedOp;
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%token INT VOID IF ELSE WHILE RETURN
%token ID NUM
%token PLUS MINUS TIMES OVER LT LE GT GE EQ NE ASSIGN SEMI COMMA
%token LPAREN RPAREN LCURLY RCURLY LBRACE RBRACE
%token ERROR 

%right ASSIGN
%left PLUS MINUS
%left TIMES OVER

%nonassoc THEN
%nonassoc ELSE

%% /* Grammar for CMINUS */

program     : decl_list { savedTree = $1; }
            ;
decl_list   : decl_list decl {
                YYSTYPE t = $1;
                if (t != NULL) {
                  while (t->sibling != NULL) t = t->sibling;
                  t->sibling = $2;
                  $$ = $1;
                } else $$ = $2;
              }
            | decl { $$ = $1; }
            ;
decl        : var_decl { $$ = $1; }
            | fun_decl { $$ = $1; }
            ;
identifier  : ID { 
                savedName = copyString(tokenString);
                savedLineNo = lineno;
              }
number      : NUM { savedNumber = atoi(tokenString); }
type_spec   : INT { savedType = Integer; }
            | VOID { savedType = Void; }
            ;
var_decl    : type_spec identifier SEMI {
                $$ = newDeclNode(NonArrVarK);
                $$->type = savedType;
                $$->attr.name = savedName;
                $$->lineno = savedLineNo;
              }
            | type_spec identifier {
                $$ = newDeclNode(ArrVarK);
                $$->type = savedType;
                $$->attr.name = savedName;
                $$->lineno = savedLineNo;
            } LBRACE number RBRACE SEMI {
                YYSTYPE t = newExpNode(ConstK);
                t->attr.val = savedNumber;
                $3->child[0] = t;
                $$ = $3;
              }
            ;
fun_decl    : type_spec identifier {
                $$ = newDeclNode(FuncK);
                $$->type = savedType;
                $$->attr.name = savedName;
                $$->lineno = savedLineNo;
              } LPAREN params RPAREN comp_stmt {
                $$ = $3;
                $$->child[0] = $5;
                $$->child[1] = $7;
                // $$->lineno = lineno;
              }
            ;
params      : param_list { $$ = $1; }
            | VOID {
                $$ = newParamNode(VoidParamK);
                $$->type = Void;
                $$->lineno = lineno;
              }
            ;
param_list  : param_list COMMA param {
                YYSTYPE t = $1;
                while (t->sibling != NULL) t = t->sibling;
                t->sibling = $3;
                $$ = t;
              }
            | param { $$ = $1; }
            ;
param       : type_spec identifier {
                $$ = newParamNode(NonArrParamK);
                $$->type = savedType;
                $$->attr.name = savedName;
                $$->lineno = savedLineNo;
              }
            | type_spec identifier LBRACE RBRACE {
                $$ = newParamNode(ArrParamK);
                $$->type = savedType;
                $$->attr.name = savedName;
                $$->lineno = savedLineNo;
              }
            ;
comp_stmt   : LCURLY local_decls stmt_list RCURLY {
                $$ = newStmtNode(CompK);
                $$->child[0] = $2;
                $$->child[1] = $3;
                $$->lineno = lineno;
              }
            ;
local_decls : local_decls var_decl {
                YYSTYPE t = $1;
                if (t != NULL) {
                  while (t->sibling != NULL) t = t->sibling;
                  t->sibling = $2;
                  $$ = $1;
                } else $$ = $2;
              }
            | /* empty */ { $$ = NULL; }
            ;
stmt_list   : stmt_list stmt {
                YYSTYPE t = $1;
                if (t != NULL) {
                  while (t->sibling != NULL) t = t->sibling;
                  t->sibling = $2;
                  $$ = $1;
                } else $$ = $2;
              }
            | /* empty */ { $$ = NULL; }
            ;
stmt        : exp_stmt { $$ = $1; }
            | comp_stmt { $$ = $1; }
            | sel_stmt { $$ = $1; }
            | iter_stmt { $$ = $1; }
            | ret_stmt { $$ = $1; }
            ;
sel_stmt    : IF LPAREN exp RPAREN stmt %prec THEN {
                $$ = newStmtNode(IfK);
                $$->child[0] = $3;
                $$->child[1] = $5;
              }
            | IF LPAREN exp RPAREN stmt ELSE stmt {
                $$ = newStmtNode(IfElseK);
                $$->child[0] = $3;
                $$->child[1] = $5;
                $$->child[2] = $7;
            }
exp_stmt    : exp SEMI { $$ = $1; }
            | SEMI { $$ = NULL; }
            ;
iter_stmt   : WHILE LPAREN exp RPAREN stmt {
                $$ = newStmtNode(IterK);
                $$->child[0] = $3;
                $$->child[1] = $5;
                $$->lineno = lineno;
              }
            ;
ret_stmt    : RETURN SEMI {
                $$ = newStmtNode(RetK);
                $$->lineno = lineno;
              }
            | RETURN exp SEMI {
                $$ = newStmtNode(RetK);
                $$->child[0] = $2;
                $$->lineno = lineno;
              }
            ;
exp         : var ASSIGN exp {
                $$ = newExpNode(AssignK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->lineno = lineno;
              }
            | simple_exp { $$ = $1; }
            ;
var         : identifier {
                $$ = newExpNode(NonArrIdK);
                $$->attr.name = savedName;
                $$->lineno = savedLineNo;
              }
            | identifier {
                $$ = newExpNode(ArrIdK);
                $$->attr.name = savedName;
                $$->lineno = savedLineNo;
              } LBRACE exp RBRACE {
                $$ = $2;
                $$->child[0] = $4;
              }
            ;
simple_exp  : add_exp relop {
                $$ = newExpNode(OpK);
                $$->attr.op = savedOp;
              } add_exp {
                $$ = $3;
                $$->child[0] = $1;
                $$->child[1] = $4;
                $$->lineno = lineno;
              }
            | add_exp { $$ = $1; }
            ;
relop       : LE { savedOp = LE; }
            | LT { savedOp = LT; }
            | GT { savedOp = GT; }
            | GE { savedOp = GE; }
            | EQ { savedOp = EQ; }
            | NE { savedOp = NE; }
            ;
add_exp     : add_exp addop {
                $$ = newExpNode(OpK);
                $$->attr.op = savedOp;
              } term {
                $$ = $3;
                $$->child[0] = $1;
                $$->child[1] = $4;
                $$->lineno = lineno;
              }
            | term { $$ = $1; }
            ;
addop       : PLUS { savedOp = PLUS; }
            | MINUS { savedOp = MINUS; }
            ;
term        : term mulop {
                $$ = newExpNode(OpK);
                $$->attr.op = savedOp;
              } factor {
                $$ = $3;
                $$->child[0] = $1;
                $$->child[1] = $4;
                $$->lineno = lineno;
              }
            | factor { $$ = $1; }
            ;
mulop       : TIMES { savedOp = TIMES; }
            | OVER { savedOp = OVER; }
            ;
factor      : LPAREN exp RPAREN { $$ = $2; }
            | var { $$ = $1; }
            | call { $$ = $1; }
            | number { 
                $$ = newExpNode(ConstK);
                $$->attr.val = savedNumber;
                $$->lineno = lineno;
              }
            ;
call        : identifier {
                $$ = newExpNode(CallK);
                $$->attr.name = savedName;
                $$->lineno = savedLineNo;
              } LPAREN args RPAREN {
                $$ = $2;
                $$->child[0] = $4;
              }
            ;
args        : arg_list { $$ = $1; }
            | /* empty */ { $$ = NULL; }
            ;
arg_list    : arg_list COMMA exp {
                YYSTYPE t = $1;
                while (t->sibling != NULL) t = t->sibling;
                t->sibling = $3;
                $$ = $1;
              }
            | exp { $$ = $1; }
            ;

%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

