# C-Minus Compiler Project

### Compilation Requirement

- GCC-11
- FLEX 2.6.4


### Language Element

#### Keyword
- if
- else
- while
- return 
- int
- void

#### Symbol
- \+
- \-
- \*
- /
- <
- <=
- \>
- \>=
- ==
- !=
- ;
- ,
- (
- )
- [
- ]
- {
- }

#### Identifier and Number rule
- letter = [a-zA-Z]
- digit = [0-9]
- ID = letter ( letter | digit ) *
- NUM = digit digit *

## Project 1 - Scanner

### How to use

#### Custom c-minus code scanner
```bash
$ make cminus_cimpl
$ ./cminus_cimpl <filename>
```

#### Flex c-minus code scanner
```bash
$ make cminus_lex
$ ./cminus_lex <filename>
```


### How to implement

#### Code Scanner with custom c code
`==`와 `!=`, `<=`와 `<`처럼 분기가 나뉘는 문자에 대해서는 다른 `state`로 전이되도록 설계했다. 이후 전이된 `state`에서 어떤 token으로 scan될지 판단한다.

```c
case START:
    if (isdigit(c))
        state = INNUM;
    else if (isalpha(c))
        state = INID;
    else if (c == '=')
        state = INEQ;
    else if (c == '<')
        state = INLT;
    else if (c == '>')
        state = INGT;
    else if (c == '!')
        state = INNE;
    else if (c == '/') {
        save = FALSE;
        state = INOVER;
    }
    else if ((c == ' ') || (c == '\t') || (c == '\n'))
        save = FALSE;
    else
    {   state = DONE;
        switch (c)
        { case EOF:
            save = FALSE;
            currentToken = ENDFILE;
            break;
        case ',':
            currentToken = COMMA;
            break;
        case '+':
            currentToken = PLUS;
            break;
        case '-':
            currentToken = MINUS;
            break;
        case '*':
            currentToken = TIMES;
            break;
        case '(':
            currentToken = LPAREN;
            break;
        case ')':
            currentToken = RPAREN;
            break;
        case '{':
            currentToken = LCURLY;
            break;
        case '}':
            currentToken = RCURLY;
            break;
        case '[':
            currentToken = LBRACE;
            break;
        case ']':
            currentToken = RBRACE;
            break;
        case ';':
            currentToken = SEMI;
            break;
        default:
            currentToken = ERROR;
            break;
        }
    }
    break;
```

`state`들을 표로 표현하면 아래와 같다.
|`state`|`첫 글자`|`설명`|
|:---|---|---|
|`START`||시작 `state`|
|`INNUM`|`digit`|숫자 토큰을 만드는 중|
|`INID`|`letter`|ID 토큰을 만드는 중|
|`INEQ`|`=`|`==`의 가능성이 있는 상태|
|`INLT`|`<`|`<=`의 가능성이 있는 상태|
|`INGT`|`>`|`>=`의 가능성이 있는 상태|
|`INNE`|`!`|`!=`의 가능성이 있는 상태|
|`INOVER`|`/`|주석 `/* */`의 가능성이 있는 상태|
|`INCOMMENT`||`INOVER`일 때 `*`을 입력받은 상태|
|`INCOMMENT_`||`INCOMMENT`일 때 `*`을 입력받은 상태<br>여기서 `/`을 입력받으면 주석이 끝난것이므로, `START`로 전이된다|
|`DONE`||토큰화가 끝난 상태|

<br>

`INEQ`를 예로 들면, `=`를 입력받은 후, `=`를 다시 입력받으면 해당 토큰은 `==`로 결정된다. 반면 `=`가 아닌 문자를 받으면 `=`로 결정되고, 따라서 입력받았던 문자를 되돌리기 위해 `ungetNextChar()`이 필요하다.

```c
case INEQ:
    if (c == '=')
    {   state = DONE;
        currentToken = EQ;
    }
    else
    {   ungetNextChar();
        save = FALSE;
        state = DONE;
        currentToken = ASSIGN;
    }
    break;
```

주석 처리를 예로 들면, `START`에서 `/`를 받아 `INOVER`로 전이되면, 주석인지 아니면 `/`연산인지 판단이 필요하다. 따라서 `INOVER`에서 `*`가 아닌 문자가 들어왔을 경우에는 `OVER`로 판단하고, 토큰화를 끝낸다. 반면 `*`가 들어왔을 때는 주석으로 판단하고 `INCOMMENT`로 전이한다.

`INCOMMENT`에서 `*`를 입력받으면 주석을 끝내는 심볼일 것으로 예상할 수 있기 때문에 `INCOMMENT_`로 전이한다.

`INCOMMENT_`에서 `/`를 입력받으면 주석을 끝내는 심볼임을 확정지을 수 있기 때문에 `START`로 전이하고, 토큰화를 재개한다. 만약 다른 문자를 받는다면 주석이 끝난다고 예상할 수 없는 상황이므로 `INCOMMENT`로 되돌아간다.

만약 주석이 닫히지 않는다면, EOF를 출력하도록 설계했다.

```c
case INOVER:
    if (c == '*')
    {   save = FALSE;
        state = INCOMMENT;
    }
    else
    {   ungetNextChar();
        state = DONE;
        currentToken = OVER;
    }
    break;
case INCOMMENT:
    if (c == '*')
    {   save = FALSE;
        state = INCOMMENT_;
    } else if (c == EOF)
    {   save = FALSE;
        state = DONE;
        currentToken = ENDFILE;
    } else save = FALSE;
    break;
case INCOMMENT_:
    if (c == '/')
    {   save = FALSE;
        state = START;
    }
    else if (c == EOF)
    {   state = DONE;
        currentToken = ENDFILE;
    }
    else
    {   save = FALSE;
        state = INCOMMENT;
    }
    break;
```

DFA로 표현하면 아래와 같다.

<img src="./img/FSM for COMMENT.png"/>

#### Code Scanner made by FLEX
`Keyword`와 `Symbol`에 따라 `globals.h`에 정의된 `TokenType`을 반환한다.

`/* */`의 경우에는 `/*`을 입력받았을 때 `COMMENT`로 전이하고, `COMMENT`에서 `*/`을 입력받았을 때 초기 `state`인 `INITIAL`로 전이한다.

```cpp
%x COMMENT

%%

"if"            {return IF;}
"else"          {return ELSE;}
"while"         {return WHILE;}
"return"        {return RETURN;}
"int"           {return INT;}
"void"          {return VOID;}
"="             {return ASSIGN;}
"=="            {return EQ;}
"!="            {return NE;}
"<"             {return LT;}
"<="            {return LE;}
">"             {return GT;}
">="            {return GE;}
"+"             {return PLUS;}
"-"             {return MINUS;}
"*"             {return TIMES;}
"/"             {return OVER;}
"("             {return LPAREN;}
")"             {return RPAREN;}
"{"             {return LCURLY;}
"}"             {return RCURLY;}
"["             {return LBRACE;}
"]"             {return RBRACE;}
";"             {return SEMI;}
","             {return COMMA;}
{number}        {return NUM;}
{identifier}    {return ID;}
{newline}       {lineno++;}
{whitespace}    {/* skip whitespace */}
"/*"            {BEGIN(COMMENT);}
<COMMENT>"*/"   {BEGIN(INITIAL);}
<COMMENT>\n     {lineno++;}
<COMMENT>.      {}
.               {return ERROR;}
```

### Test Case

#### Code Scanner with custom c code

Test Case #1

<img src="./img/Custom Test Case #1.png"/>

Test Case #2

<img src="./img/Custom Test Case #2.png"/>

#### Code Scanner made by FLEX

Test Case #1

<img src="./img/FLEX Test Case #1.png"/>

Test Case #2

<img src="./img/FLEX Test Case #2.png"/>