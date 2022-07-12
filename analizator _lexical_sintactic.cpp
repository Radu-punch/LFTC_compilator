#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <stdarg.h>
#include <stdbool.h>
void err(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}

enum {
	ID, END, CT_INT, ASSIGN, SEMICOLON, CT_REAL, CT_CHAR, CT_STRING,
	COMMA, LPAR, RPAR, LBRACKET, RBRACKET, LACC, RACC, ADD, SUB, MUL, DIV, DOT,
	AND, OR, NOT, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ, SPACE, LINECOMMENT, COMMENT,
	BREAK, CHAR, DOUBLE, ELSE, FOR, IF, INT, RETURN, STRUCT, VOID, WHILE
};
int line = 1;
char* pCrtCh;

typedef struct _Token {
	int code; // codul (numele)
	union {
		char *text; // folosit pentru ID, CT_STRING (alocat dinamic)
		long int i; // folosit pentru CT_INT, CT_CHAR
		double r; // folosit pentru CT_REAL
	};
	int line; // linia din fisierul de intrare
	struct _Token *next; // inlantuire la urmatorul AL
}Token;
void tkerr(const Token* tk, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error in line %d: ", tk->line);
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}

Token* tokens = NULL;
Token* lastToken = NULL;

#define SAFEALLOCN(var,Type,n)		if((var=(Type*)malloc(sizeof(Type)*(n)))==NULL)err("not enough memory");
#define SAFEALLOC(var,Type)		if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");



Token *addTk(int code)
{
	Token *tk;
	SAFEALLOC(tk, Token)
	tk->code = code;
	tk->line = line;
	tk->next = NULL;
	if (lastToken) {
		lastToken->next = tk;
	}
	else {
		tokens = tk;
	}
	lastToken = tk;
	return tk;
}






char* createString(const char* begin, const char* after)
{
	int		n = (after - begin);
	char *p;
	SAFEALLOCN(p, char, n + 1);
		strncpy(p, begin, n);
	p[n] = '\0';
	return p;
}


int getNextToken()
{
	int state = 0;
	char ch;
    int octal = 0;
    int hexa = 0;
    int intreg = 0;
    char* start;
		int n;
		Token* tk;
	for (;;) { // bucla infinita
		 ch = *pCrtCh;

		switch (state) {
		    start = 0;
		case 0: // testare tranzitii posibile din starea 0
			if (isalpha(ch) || ch == '_') {
				start = pCrtCh;// memoreaza inceputul ID-ului
				pCrtCh++; // consuma caracterul
				state = 1; // trece la noua stare
			}
			else if (ch == '=') {
				pCrtCh++;
				state = 44;
			}
			else if (ch == ' ' || ch == '\r' || ch == '\t') {
				pCrtCh++; // consuma caracterul si ramane in starea 0
			}
			else if (ch == '\n') { // tratat separat pentru a actualiza linia curenta
				line++;
				pCrtCh++;
			}
			else if (ch == '!') {
				pCrtCh++;
				state = 41;
			}
			else if (ch == '>') {
				pCrtCh++;
				state = 49;
			}
			else if (ch == '<') {
				pCrtCh++;
				state = 47;
			}
			else if (ch == '+') {
				pCrtCh++;
				state = 58;
			}
			else if (ch == '-') {
				pCrtCh++;
				state = 57;
			}
			else if (ch == '*') {
				pCrtCh++;
				state = 56;
			}
			else if (ch == '.') {
				pCrtCh++;
				state = 55;
			}
			else if (ch == '&') {
				pCrtCh++;
				state = 53;
			}
			else if (ch == '|') {
				pCrtCh++;
				state = 51;
			}
			else if (ch == ',') {
				pCrtCh++;
				state = 35;
			}
			else if (ch == ';') {
				pCrtCh++;
				state = 36;
			}
			else if (ch == '}') {
				pCrtCh++;
				state = 34;
			}
			else if (ch == '{') {
				pCrtCh++;
				state = 33;
			}
			else if (ch == ']') {
				pCrtCh++;
				state = 40;
			}
			else if (ch == '[') {
				pCrtCh++;
				state = 39;
			}
			else if (ch == ')') {
				pCrtCh++;
				state = 38;
			}
			else if (ch == '(') {
				pCrtCh++;
				state = 37;
			}
			else if (ch == '/') {
				start = pCrtCh;
				pCrtCh++;
				state = 27;
			}
			else if (ch == '"') {
				start = pCrtCh;
				pCrtCh++;
				state = 9;
			}
			else if (ch == '0') {
				start = pCrtCh;
				pCrtCh++;
				state = 15;
			}
			else if(ch=='\n'||ch=='\r'||ch=='\t')state = 0;
			else if(ch>='1'&&ch<='9'){
                start = pCrtCh;
                pCrtCh++;
                state = 60;
			}
			else if(ch=='\0'||(ch=='E'&&ch=='O'&&ch=='F')){
                pCrtCh++;
                state = 61;
			}
			else if (ch == '\'') {
				start = pCrtCh;
				pCrtCh++;
				state = 3;

			}
			else if (ch == 0) { // sfarsit de sir
				addTk(END);
				return END;
			}
			else err("caracter invalid : %c (%d)", ch, ch);
			break;
		case 1:
			if (isalnum(ch) || ch == '_')pCrtCh++;
			else state = 2;
			break;
		case 2:
			n = pCrtCh - start; // lungimea cuvantului gasit
			// teste cuvinte cheie
			if (n == 5 && !memcmp(start, "break", 5))tk = addTk(BREAK);
			else if (n == 4 && !memcmp(start, "char", 4))tk = addTk(CHAR);
			else if (n == 6 && !memcmp(start, "double", 6))tk = addTk(DOUBLE);
			else if (n == 4 && !memcmp(start, "else", 4))tk = addTk(ELSE);
			else if (n == 3 && !memcmp(start, "for", 3))tk = addTk(FOR);
			else if (n == 2 && !memcmp(start, "if", 2))tk = addTk(IF);
			else if (n == 3 && !memcmp(start, "int", 3))tk = addTk(INT);
			else if (n == 6 && !memcmp(start, "return", 6))tk = addTk(RETURN);
			else if (n == 6 && !memcmp(start, "struct", 6))tk = addTk(STRUCT);
			else if (n == 4 && !memcmp(start, "void", 4))tk = addTk(VOID);
			else if (n == 5 && !memcmp(start, "while", 5))tk = addTk(WHILE);
			// … toate cuvintele cheie …
			else { // daca nu este un cuvant cheie, atunci e un ID
				tk = addTk(ID);
				tk->text = createString(start, pCrtCh);

			}
			return tk->code;

		case 3:
			if (ch == '\\') {
				pCrtCh++;
				state = 5;
			}
			else if (  ch != '\'' && ch != '\\') {
				pCrtCh++;
				state = 6;
			}
			break;
		case 5:
			if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 't'
				|| ch == 'v' || ch == '\'' || ch == '?' || ch == '"' || ch == '\\' || ch == '0')
			{
				pCrtCh++;
				state = 6;
			}
			break;
		case 6:
			if (ch == '\'') {
				pCrtCh++;
				state = 8;
			}
			break;
		case 8:
			addTk(CT_CHAR);
			lastToken->text = createString(start, pCrtCh - 1);
			return CT_CHAR;
		case 9:
			if (ch != '"' && ch != '\\') {
				pCrtCh++;
				state = 13;
				//printf("aic1i");
            }
			else if (ch == '\\') {
				pCrtCh++;
				state = 11;

			}
			else state = 13;
			break;

		case 11:
			if (ch == 'a' || ch == 'b' || ch == 'f' || ch == 'n' || ch == 'r' || ch == 't'
				|| ch == 'v' || ch == '\'' || ch == '?' || ch == '"' || ch == '\\' || ch == '0')
			{
				pCrtCh++;
				state = 12;
			}
			break;
		case 12:
			pCrtCh++;
			state = 13;
		case 13:
			if (ch == '"') {
				pCrtCh++;
				//printf("aici ");
				state = 14;
				}
            else {state = 9;}
			break;
		case 14:
		    addTk(CT_STRING);
			lastToken->text = createString(start, pCrtCh - 1);
            return CT_STRING;
		case 27:
			if (ch == '/') {
				pCrtCh++;
				state = 31;
			}
			else if (ch == '*') {
				pCrtCh++;
				state = 28;
			}
			else state = 32;
			break;
		case 31:
			if ( (ch != '\n') || (ch != '\r') ||( ch != '\0'))pCrtCh++;
			else state = 0;
			break;
		case 28:
			if ( ch != '*')pCrtCh++;
			else if (ch == '*') {
				pCrtCh++;
				state = 29;
			}
			break;
		case 29:
			if (ch == '*')pCrtCh++;
			else if (ch == '/') {
				pCrtCh++;
				state = 30;
			}
			else state = 28;
			break;
		case 30:
			pCrtCh++;
			state = 0;
			break;
		case 32:
			addTk(DIV);
			return DIV;
		case 15:
			/*if (ch>='0'&&ch<='7') {
				pCrtCh++;
				state = 19;

			}*/
			//else
                if (ch == 'x') {
				pCrtCh++;
				state = 16;
			}
			/*else if (ch=='8'||ch=='9') {
				pCrtCh++;
				state = 20;
             //printf("aici");
			}*/
			else if (ch == '.') {
				pCrtCh++;
				state = 21;
			}
			else state = 19;
			break;
		case 16:
			if (isdigit(ch)||(ch>='a'&&ch<='f')||(ch>='A'&&ch<='F')) {
				pCrtCh++;
				state = 17;
			}
			break;
		case 17:
			if (isdigit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))pCrtCh++;
			else {
				state = 18;
				hexa = 1;
			}
			break;
		case 19:
			if (ch>='0'&&ch<='7') pCrtCh++;
            else if (ch=='8' || ch == '9') {
				pCrtCh++;

				state = 20;
			}
			else{state = 18; octal = 1; }

			break;
		case 20:

			 if (isdigit(ch))pCrtCh++;
            else if (ch == '.') {
				pCrtCh++;
				state = 21;
			}
			else if(ch == 'e'||ch == 'E'){
                pCrtCh++;
                state = 23;
			}
			else {state=26;}
			break;
        case 60:
            if(isdigit(ch)){
                pCrtCh++;
            }
            else if(ch == 'e'||ch == 'E'){
                pCrtCh++;
                state = 23;
            }else if(ch == '.'){
                pCrtCh++;
                state = 21;
            }else {

                intreg = 1;
                state = 18;
            }
            break;
		case 21:
			if (isdigit(ch)) {
				pCrtCh++;
				state = 22;
			}
			break;
		case 22:
			if (isdigit(ch))pCrtCh++;
			else if (ch == 'e' || ch == 'E') {
				pCrtCh++;
				state = 23;
			}
			else state = 26;
			break;
		case 23:
			if (ch == '-' || ch == '+') {
				pCrtCh++;
				state = 24;
			} else state= 24;
			break;
		case 24:
			if (isdigit(ch)) {
				pCrtCh++;
				state = 25;
			}
			break;
		case 25:
			if (isdigit(ch))pCrtCh++;
			else state = 26;
			break;
		case 18:
			addTk(CT_INT);
			if (octal == 1) {

				lastToken->i = strtol(createString(start, pCrtCh), NULL, 8);

			}
			else if (intreg == 1) {
				lastToken->i = atoi(createString(start, pCrtCh));

			}
			else if (hexa == 1) {
				lastToken->i = strtol(createString(start, pCrtCh), NULL, 16);
			}

			return CT_INT;
		case 26:
			addTk(CT_REAL);
			lastToken->r = atof(createString(start, pCrtCh));
			return CT_REAL;
		case 44:
			if (ch == '=') {
				pCrtCh++;
				state = 45;
			}
			else state = 46;
			break;
		case 49:
			if (ch == '=') {
				pCrtCh++;
				state = 50;
			}
			else {
				addTk(GREATER);
				return GREATER;
			}
			break;
		case 47:
			if (ch == '=') {
				pCrtCh++;
				state = 48;
			}
			else {
				addTk(LESS);
				return LESS;
			}
			break;
		case 41:
			if (ch == '=') {
				pCrtCh++;
				state = 42;
			}
			else {
				addTk(NOT);
				return NOT;
			}
			break;
		case 42:
			addTk(NOTEQ);
			return NOTEQ;
		case 48:
			addTk(LESSEQ);
			return LESSEQ;
		case 50:
			addTk(GREATEREQ);
			return GREATEREQ;
		case 51:
			if (ch == '|') {
				pCrtCh++;
				state = 52;
			}
			break;
		case 53:
			if (ch == '&') {
				pCrtCh++;
				state = 54;
			}
			break;
        case 61:
            addTk(END);
            return END;
		case 54:
		    addTk(AND);
			return AND;
		case 52:
			addTk(OR);
			return OR;
		case 45:
			addTk(EQUAL);
			return EQUAL;
		case 46:
			addTk(ASSIGN);
			return ASSIGN;
		case 35:
			addTk(COMMA);
			return COMMA;
		case 34:
			addTk(RACC);
			return RACC;
		case 33:
			addTk(LACC);
			return LACC;
		case 36:
			addTk(SEMICOLON);
			return SEMICOLON;
		case 37:
			addTk(LPAR);
			return LPAR;
		case 38:
			addTk(RPAR);
			return RPAR;
		case 39:
			addTk(LBRACKET);
			return LBRACKET;
		case 40:
			addTk(RBRACKET);
			return RBRACKET;
		case 56:
			addTk(MUL);
			return MUL;
		case 57:
			addTk(SUB);
			return SUB;
		case 58:
			addTk(ADD);
			return ADD;
		case 55:
			addTk(DOT);
			return DOT;
		}
	}
}

void getCaractereSpeciale(char* caracter) {
   const char *s ;
    for(s=caracter;*s != '\0';++s)
    {
        int special= (unsigned char) *s;
        if(special != '\\')
            putchar(special);
        else{
            if(special == '\\'){
                special = (unsigned char) *(++s);
                if(special == 'a') printf("\a");
                if(special == 'b') printf("\b");
                if(special == 'f') printf("\f");
                if(special == 'n') printf("\n");
                if(special == 'r') printf("\r");
                if(special == 't') printf("\t");
                if(special == 'v') printf("\v");
                if(special == '\'') printf("\'");
                if(special == '?') printf("\?");
                if(special == '"') printf("\"");
                if(special == '\\') printf("\\");
                if(special == '0') printf("\0");

            }else printf("%c",special);
        }


    }

}
void showTokens(Token* tk) {



		switch (tk->code)
		{
		case ID:printf("ID : %s\n",tk->text); break;
		case BREAK:printf("BREAK\n"); break;
		case CHAR:printf("CHAR\n"); break;
		case DOUBLE:printf("DOUBLE\n"); break;
		case ELSE:printf("ELSE\n"); break;
		case FOR:printf("FOR\n"); break;
		case IF:printf("IF\n"); break;
		case INT:printf("INT\n"); break;
		case RETURN:printf("RETURN\n"); break;
		case STRUCT:printf("STRUCT\n"); break;
		case VOID:printf("VOID\n"); break;
		case WHILE:printf("WHILE\n"); break;
		case LPAR:printf("LPAR\n"); break;
		case COMMA:printf("COMMA\n"); break;
		case SEMICOLON:printf("SEMICOLON\n"); break;
		case RPAR:printf("RPAR\n"); break;
		case LBRACKET:printf("LBRACKET\n"); break;
		case RBRACKET:printf("RBRACKET\n"); break;
		case LACC:printf("LACC\n"); break;
		case ADD:printf("ADD\n"); break;
		case SUB:printf("SUB\n"); break;
		case MUL:printf("MUL\n"); break;
		case DIV:printf("DIV\n"); break;
		case DOT:printf("DOT\n"); break;
		case AND:printf("AND\n"); break;
		case OR:printf("OR\n"); break;
		case RACC:printf("RACC\n"); break;
		case NOT:printf("NOT\n"); break;
		case ASSIGN:printf("ASSIGN\n"); break;
		case EQUAL:printf("EQUAL\n"); break;
		case NOTEQ:printf("NOTEQ\n"); break;
		case LESS:printf("LESS\n"); break;
		case LESSEQ:printf("LESSEQ\n"); break;
		case GREATER:printf("GREATER\n"); break;
		case GREATEREQ:printf("GREATEREQ\n"); break;
		case END:printf("END\n"); break;
		case CT_CHAR:printf("CT_CHAR: "); getCaractereSpeciale(tk->text); printf("\n"); break;
		case CT_STRING:printf("CT_STRING: ");getCaractereSpeciale(tk->text); printf("\n"); break;
		case CT_INT:printf("CT_INT:%ld\n", tk->i); break;
		case CT_REAL:printf("CT_REAL:%f\n", tk->r); break;
		//default: err("shiit caracter invalid : %c (&d)", tk->code, tk->text); break;

		}

	}

Token* crtTk;
Token* consumedTk;

bool consume(int code)
{
	if (crtTk->code == code) {
		consumedTk = crtTk;
		crtTk = crtTk->next;
		return true;
	}
	return false;
}

bool declStruct();
bool declVar();
bool typeBase();
bool arrayDecl();
bool typeName();
bool funcArg();
bool stmCompound();
bool expr();
bool exprAssign();
bool exprCast();
bool exprUnary();
bool declFunc();
bool stm() ;
bool stmCompound();
bool exprOR();
bool exprORprim();
bool exprAndPrim();
bool exprAnd();
bool exprEqPrim();
bool exprEq();

bool exprReal();
bool exprRealPrim();
bool exprAddPrim();
bool exprAdd();
bool exprMulPrim();
bool exprMul();
bool exprPrimary();
bool exprPostfix();
bool exprPostfixPrimary();

bool unit() {
	Token* start = crtTk;
	while (1) {
		if (declStruct()) { return true; }
		else if (declFunc()) { return true; }
		else if (declVar()) { return true; }
		else break;
	}
	if (consume(END)) { return true; } //else tkerr(crtTk, "missing END or syntax error");
	crtTk = start;
	return false;
}

/*bool declStruct() {
	Token* start = crtTk;
	if (consume(STRUCT)) {
		if (consume(ID)) {
			if (consume(LACC)) {
				while (1) {
					if (declVar()) {}
					break;
				}
				if (consume(RACC)) {
					if (consume(SEMICOLON)) {
						return true;
					}else tkerr(crtTk, "missing ; or sybtax error");
				}else tkerr(crtTk, "missing right braket } or syntax error");
			}else tkerr(crtTk, "missing left braket } or syntax error");
		}else tkerr(crtTk, "incorect declaration of ID or syntax error");
	}else tkerr(crtTk, "incorect declaration of STRUCT ");
	crtTk = start;
	return false;
}*/
bool declStruct(){
    Token *startTk=crtTk;
    if(consume(STRUCT)){
        if(consume(ID)){
            if(consume(LACC)){
                while(declVar()){
                }
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                            return true;
                    }else tkerr(crtTk, "lipseste ; dupa } ");
                }else tkerr(crtTk, "lipseste } dupa declaratia de variabila");
            }else tkerr(crtTk, "lipseste { dupa numele variabilei");
        }else tkerr(crtTk, "lipseste numele variabilei ");
    }
    crtTk=startTk;
    return false;
    }

bool declVar() {
	Token* start = crtTk;
	if (typeBase()) {
		if (consume(ID)) {
			if(arrayDecl()){}

			while (1) {
				if (consume(COMMA)) {
					if (consume(ID)) {
						if (arrayDecl()) {}

					}else tkerr(crtTk, "missing declaration of ID ");
				}else tkerr(crtTk, "missing declaration of , ");
				break;

			}
			if (consume(SEMICOLON)) {
				return true;

			}else tkerr(crtTk, "missing ; or syntax error");
		}else tkerr(crtTk, "invalid declaration of ID ");
	}
	crtTk = start;
	return false;

}

bool typeBase() {
	Token* start = crtTk;
	if (consume(INT)){
		return true;
	}//else tkerr(crtTk, "invalid declaration of INT");
	if (consume(DOUBLE)) {
		return true;
	}//else tkerr(crtTk, "invalid declaration of Double");
	if (consume(CHAR)) {
		return true;
	}//else tkerr(crtTk, "invalid declaration of Char");
	if (consume(STRUCT)) {
		if (consume(ID)) {
			return true;
		}else tkerr(crtTk, "invalid declaration of ID");
	}//else tkerr(crtTk, "invalid declaration of Structure");
	crtTk = start;
	return false;

}

bool arrayDecl() {
	Token* start = crtTk;
	if (consume(LBRACKET)) {
		if(expr()){}
		if (consume(RBRACKET)) {
			return true;
		}else tkerr(crtTk, "missing ] from sintax");
	}else tkerr(crtTk, "missing [ from sintax");
	crtTk = start;
	return false;
}

bool typeName() {
	Token* start = crtTk;
	if (typeBase()) {
		if (arrayDecl()) {}
		return true;
	}
	crtTk = start;
	return false;

}

bool funcArg() {
	Token* start = crtTk;
	if (typeBase()) {
		if (consume(ID)) {
			if(arrayDecl()){}
			return true;
		}else tkerr(crtTk, "invalid declaration or sintax error in ID");
	}
	crtTk = start;
	return false;

}

bool stmCompound() {
	Token* start = crtTk;
	if (consume(LACC)) {
		while (1)
		{
			if (declVar()) {
				return true;
			}else tkerr(crtTk, "invalid declaration of var");
			if (stm()) {
				return true;
			}
		}	if (consume(RACC)) {
			return true;
		}else tkerr(crtTk, "forget to close bracket }");
	}else tkerr(crtTk, "missing left bracket {");
	crtTk = start;
	return false;
}

bool expr() {
	Token* start = crtTk;
	if (exprAssign()) {
		return true;
	}
	crtTk = start;
	return false;
}

bool exprAssign() {
	Token* start = crtTk;
	if (exprUnary()) {
		if (consume(ASSIGN)) {
			if (exprAssign()) {
				return true;
			}
		}else tkerr(crtTk, "missing asign = sign");
		crtTk = start;
	} if (exprOR()) {
		return true;
	}
	crtTk = start;
	return false;
}

bool exprCast() {
	Token* start = crtTk;
	if (consume(LPAR)) {
		if (typeName()) {
			if (consume(RPAR)) {
				if (exprCast()) {
					return true;
				}
			}
			else tkerr(crtTk, "error missing )");
		}
		crtTk = start;
	}
	else tkerr(crtTk, "error missing ( ");
	if (exprPostfix()) {
		return true;
	}
	crtTk = start;
	return false;

}

bool exprUnary() {
	Token* start = crtTk;
	if (consume(SUB) || consume(NOT)) {
		if (exprUnary()) {
			return true;
		}
	}
	else tkerr(crtTk, "missing - or negation !");
	crtTk = start;
	if (exprPostfix()) {
		return true;
	}
	crtTk = start;
	return false;
}

bool declFunc() {
	Token* start = crtTk;
	if (typeBase()) {
		if(consume(MUL)){}
		if (consume(VOID)) {
			return true;
		}
	}
	if (consume(ID)) {
		if (consume(LPAR)) {
			if (funcArg()) {
				while(1) {
					if (consume(COMMA)) {
						if (funcArg()) {
							return true;
						}
					}
				}
			}
			if (consume(RPAR)) {
				if (stmCompound()) {
					return true;
				}
			}
		}
	}
	crtTk = start;
	return false;


}

bool stm() {
	Token* start = crtTk;
	if (stmCompound()) {
		return true;
		crtTk = start;
	}

	if (consume(IF)) {
		if (consume(LPAR)) {
			if (expr()) {
				if (consume(RPAR)) {
					if (stm()) {
						if (consume(ELSE)) {
							if (stm()) {}
						}
						return true;
					}else tkerr(crtTk, "bad statement");
				}else tkerr(crtTk, "missing )");
			}else tkerr(crtTk, "bad expresion");
		}else tkerr(crtTk, "missing (");
		crtTk = start;
	}else tkerr(crtTk, "bad declaration of IF");

	if (consume(WHILE)) {
		if (consume(LPAR)) {
			if (expr()) {
				if (consume(RPAR)) {
					if (stm())
						return true;
				}
				else tkerr(crtTk, "missing )");
			}
			else tkerr(crtTk,"bad expresion");
		}
		else tkerr(crtTk, "missing (");
		crtTk = start;
	}else tkerr(crtTk, "bad declaration of WHILE");

	if (consume(FOR)) {
		if (consume(LPAR)) {
			if(expr()){}
			if (consume(SEMICOLON)) {
				if(expr()){}
				if (consume(SEMICOLON)) {
					if(expr()){}
					if (consume(RPAR)) {
						if (stm()) {
							return true;
						}
					}
					else tkerr(crtTk, "missing RPAR");
				}
				else tkerr(crtTk, "missing semicolon ;");
			}
			else tkerr(crtTk, "missing semicolon ;");
		}
		else tkerr(crtTk, "missing (");
		crtTk = start;
	}
	else tkerr(crtTk, "bad declaration of FOR");

	if (consume(BREAK)) {
		if (consume(SEMICOLON)) {
			return true;
		}
		else tkerr(crtTk, "missing ;");
		crtTk = start;
	}
	else tkerr(crtTk, "bad declaration of break");

	if (consume(RETURN)) {
		if(expr()){}
		if (consume(SEMICOLON)) {
			return true;
		}
		crtTk = start;

	}
	if(expr()){}
	if (consume(SEMICOLON)) {
		return true;
	}
	crtTk = start;
	return false;

}

/*bool stmCompound() {
	Token* start = crtTk;
	if (consume(LACC)) {
		while (1)
		{
			if (declVar()) {
				return true;
			}
			if (stm()) {
				return true;
			}
		}
		if (consume(RACC)) {
			return true;
		}
		else tkerr(crtTk, "missing )");
	}
	else tkerr(crtTk, "missing (");

	crtTk = start;
	return false;
}
*/
bool exprOR() {
	Token* start = crtTk;
	if (exprAnd()) {
		if (exprORprim()) {
			return true;
		}
	}
	crtTk = start;
	return true;
}
bool exprORprim() {
	Token* start = crtTk;
	if (consume(OR)) {
		if (exprEq()) {
			if (exprORprim()) {
				return true;
			}
		}
	}
	crtTk = start;
	return true;
}

bool exprAnd() {
	Token* start = crtTk;
	if (exprEq()) {
		if (exprAndPrim()) {
			return true;
		}
	}
	crtTk = start;
	return false;
}

bool exprAndPrim() {
	Token* start = crtTk;
	if (consume(AND)) {
		if (exprEq()) {
			if (exprAndPrim()) {
				return true;
			}
		}
	}
	crtTk = start;
	return true;
}

bool exprEq() {
	Token* start = crtTk;
	if (exprReal()) {
		if (exprEqPrim()) {
			return true;
		}
	}
	crtTk = start;
	return true;
}

bool exprEqPrim() {
	Token* start = crtTk;
	if (consume(EQUAL) || consume(NOTEQ)) {
		if (exprReal()) {
			if (exprEqPrim()) {
				return true;
			}
		}
	}
	crtTk = start;
	return true;
}

bool exprReal() {
	Token* start = crtTk;
	if (exprAdd()) {
		if (exprRealPrim()) {
			return true;
		}
	}
	crtTk = start;
	return false;
}
bool exprRealPrim() {
	Token* start = crtTk;
	if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
		if (exprAdd()) {
			if (exprRealPrim()) {
				return true;
			}
		}
	}
	else tkerr(crtTk, "error on comparing operators");
	crtTk = start;
	return true;
}

bool exprAdd() {
	Token* start = crtTk;
	if (exprMul()) {
		if (exprAddPrim()) {
			return true;
		}
	}
		crtTk = start;
		return false;


}

bool exprAddPrim() {
	Token* start = crtTk;
	if (consume(ADD) || consume(SUB)) {
		if (exprMul()) {
			if (exprAddPrim()) {
				return true;
			}
		}
	}
	else tkerr(crtTk, "error in add and sub operators");
	crtTk = start;
	return true;
}

bool exprMul() {
	Token* start = crtTk;
	if (exprCast()) {
		if (exprMulPrim()) {
			return true;
		}
	}
	crtTk = start;
	return false;

}

bool exprMulPrim() {
	Token* start = crtTk;
	if (consume(MUL) || consume(DIV)) {
		if (exprCast()) {
			if (exprMulPrim()) {
				return true;
			}
		}
	}
	else tkerr(crtTk, "error on * or / expression");
	crtTk = start;
	return true;
}

bool exprPrimary() {
	Token* start = crtTk;
	if (consume(ID)) {
		if (consume(LPAR)) {
			if (expr()) {
				while (1)
				{
					if (consume(COMMA)) {
						if (expr()) {
							return true;
						}
					}
				}
			}
			if (consume(RPAR)) {}
		}
		return true;
	}
	else tkerr(crtTk, "bad type declaration  ");
	if (consume(CT_INT)) {
		return true;
	}
	else tkerr(crtTk, "bad declaration of type int");
	if (consume(CT_REAL)) {
		return true;
	}
	else tkerr(crtTk, "bad declaration of real");

	if (consume(CT_CHAR)) {
		return true;
	}
	else tkerr(crtTk, "bad declaration of char");
	if (consume(CT_STRING)) {
		return true;
	}
	else tkerr(crtTk, "bad declaration of string");
	if (consume(LPAR)) {
		if (expr()) {
			if (consume(RPAR)) {
				return true;
			}
			else tkerr(crtTk, "missing ) in expresion");
		}
	}tkerr(crtTk, "missing ( in expresion");
	crtTk = start;
	return false;
}

bool exprPostfix() {
	Token* start = crtTk;
	if (exprPrimary()) {
		if (exprPostfixPrimary()) {
			return true;
		}
	}
	crtTk = start;
	return false;
}
bool exprPostfixPrimary() {
	Token* start = crtTk;
	if (consume(LBRACKET)) {
		if (expr()) {
			if (consume(RBRACKET)) {
				if (exprPostfixPrimary()) {
					return true;
				}
			}
			else tkerr(crtTk, "missing )");
		}
		else tkerr(crtTk, "bad expresion");
	}
	else tkerr(crtTk, "missing (");
	if (consume(DOT)) {
		if (consume(ID)) {
			if (exprPostfixPrimary()) {
				return true;
			}
		}
		else tkerr(crtTk, "bad ID declaration");
	}
	else tkerr(crtTk, "missing dot . ");
	crtTk = start;
	return true;
}





char inbuf[30001];
int main() {
	FILE* fis;
	if ((fis = fopen("3.c", "r")) == NULL) {
		printf("eroare la deschidere");
		return 1;
	}
	int n = fread(inbuf, 1, 30000, fis);
	inbuf[n] = '\0';
	fclose(fis);
	pCrtCh = inbuf;
    //printf("%s",pCrtCh);
//printf("aici s-a ajuns ");
	while (getNextToken() != END) {}
	//printf("aici s-a ajuns ");
	Token* tk;
	for (tk = tokens; tk; tk = tk->next) {
		printf("%d ", tk->line);
        showTokens(tk);

	}
	crtTk = tokens;
	if(unit()){
        puts("sintaxa ok");
	}else puts("eroare de sintaxa");


return 0;
}

