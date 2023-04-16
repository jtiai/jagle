grammar Jagle;

// Program is a collection of lines
prog: stmtList + EOF ;

// Statements are single liners or code blocks
statement
    : END
    | variableDeclStmt
    | variableAssignmentStmt
    | forStmt
    | ifStmt
    | printStmt
    | dataStmt
    | readStmt
    | restoreStmt
    | inputStmt
    | funcDefStmt
    | returnStmt
    | funcStmt
    | funcCallStmt
    ;

expression
    : funcCall # funcCallExpression
    | func # funcExpression
    | LPAREN expression RPAREN # parenExpression
    | <assoc=right> expression EXPONENT expression # exponentExpression
    | (NOT | unary) expression # unaryExpression
    | expression (TIMES | DIV | MOD) expression # multiplyingExpression
    | expression (PLUS | MINUS) expression # addingExpression
    | expression relop expression # relationalExpression
    | expression (AND | OR) expression # logicalExpression
    | variableAssignment # variableAssignmentExpression
    | identifier # identifierExpression
    | literal # literalExpression
    ;

stmtList
    : statement+
    ;

funcStmt
    : func
    ;

func
    : VAL LPAREN expression RPAREN # valFunc
    ;

funcDefStmt
    : funcDef
    ;

funcDef
    : FUNC identifier LPAREN argList? RPAREN (COLON variableType)? stmtList ENDFUNC
    ;

argList
    : identifier COLON variableType (COMMA identifier COLON variableType)*
    ;

funcCallStmt
    : funcCall
    ;

funcCall
    : identifier LPAREN paramList? RPAREN
    ;

paramList
    : expression (COMMA expression)*
    ;

returnStmt
    : RETURN expression?
    ;

dataStmt
    : DATA dataList
    ;

dataList
    : literal (COMMA unary? literal)*
    ;

readStmt
    : READ identifier
    ;

restoreStmt
    : RESTORE (NUMBER | identifier)?
    ;

printStmt
    : PRINT printList?
    ;

printList
    : expression (SEMICOLON expression?)*
    ;

ifStmt
    : IF expression THEN stmtList (ELSE stmtList)? ENDIF
    ;

forStmt
    : FOR (variableDecl | variableAssignment) TO expression (STEP expression)? stmtList NEXT
    ;

inputStmt
    : INPUT (prompt=STRINGLITERAL COMMA)? identifier (useDefault=ASSIGN expression)?
    ;

variableDeclStmt
    : variableDecl
    ;

variableDecl
    : identifier COLON variableType ASSIGN expression
    ;

variableAssignmentStmt
    : variableAssignment
    ;

variableAssignment
    : <assoc=right> identifier ASSIGN expression
    ;

identifier
    : ID
    ;

unary
    : PLUS
    | MINUS
    ;

passStmt : PASS ;

relop
    : EQ
    | NEQ
    | GTE
    | LTE
    | GT
    | LT
    ;

literal
    : STRINGLITERAL
    | NUMBER
    | FLOAT
    | NOTHING
    ;

variableType
    : INT_TYPE
    | FLOAT_TYPE
    | STR_TYPE
    ;

// Tokens
LET : 'let' ;
PRINT : 'print' ;
IF : 'if' ;
FOR : 'for' ;
TO : 'to' ;
STEP : 'step' ;
NEXT : 'next' ;
THEN : 'then' ;
ELSE : 'else' ;
ENDIF : 'endif' ;
END : 'end' ;
AND : 'and' ;
OR : 'or' ;
NOT : 'not' ;
PASS : 'pass' ;
NOTHING : 'Nothing' ;
DATA : 'data' ;
READ : 'read' ;
RESTORE : 'restore' ;
INPUT : 'input';
FUNC : 'func' ;
ENDFUNC: 'endfunc' ;
RETURN : 'return' ;

// Built-in functions
VAL : 'val' ;

COMMENT : '\'' ~[\r\n]*;

PLUS : '+' ;
MINUS : '-' ;
TIMES : '*' ;
DIV : '/' ;
MOD : '%' ;
EXPONENT : '^' ;

ASSIGN : '=' ;
STR_TYPE : 'str' ;
INT_TYPE : 'int' ;
FLOAT_TYPE: 'float' ;

EQ : '==' ;
NEQ : '!=' ;
GTE : '>=' ;
LTE : '<=' ;
GT : '>' ;
LT : '<' ;

LPAREN : '(' ;
RPAREN : ')' ;
COLON : ':' ;
SEMICOLON : ';' ;
COMMA : ',' ;
LBRACKET : '[' ;
RBRACKET : ']' ;

STRINGLITERAL : '"' ~ ["\r\n]* '"' ;

ID : [a-zA-Z_][a-zA-Z0-9_]* ;

NUMBER
   : [0-9] + ('E' NUMBER)*
   ;

FLOAT
   : [0-9]* '.' [0-9] + ('E' [0-9] +)*
   ;

LF : ('\r\n' | [\n\r]) -> channel (HIDDEN) ;

WS : [ \r\n\t] + -> channel (HIDDEN) ;
