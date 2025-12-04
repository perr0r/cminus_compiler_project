// Undeclared function / variable
// Ex) Error: undeclared variable "x" is used at line 3
fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", name, lineno);
fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", name, lineno);
// Void-type variable
// Ex) Error: The void-type variable is declared at line 1 (name : "x")
fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", lineno, name);
// Array indexing
// Ex) Error: Invalid array indexing at line 7 (name : "x"). indicies should be integer
// Ex) Error: Invalid array indexing at line 5 (name : "x"). indexing can only allowed for int[] variables
fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indicies should be integer\n", lineno, name);
fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", lineno, name);
// Invalid function call
// Ex) Error: Invalid function call at line 7 (name : "func")
fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", lineno, name);
// Invalid return
// Ex) Error: Invalid return at line 3
fprintf(listing, "Error: Invalid return at line %d\n", lineno);
// Invalid assignment
// Ex) Error: invalid assignment at line 5
fprintf(listing, "Error: invalid assignment at line %d\n", lineno);
// Invalid operation
// Ex) Error: invalid operation at line 6
fprintf(listing, "Error: invalid operation at line %d\n", lineno);
// Invalid condition
// Ex) Error: invalid condition at line 5
// fprintf(listing, "Error: invalid condition at line %d\n", lineno);
// Redefinition
// Ex) Error: Symbol "x" is redefined at line 5 (already defined at line 2 3 4 4)
// fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line ", name, lineno);
// Sequentially print all the line numbers using the following macros
fprintf(listing, " ");
fprintf(listing, "%d", symbol->lineList->lineno);
fprintf(listing, ")\n");

