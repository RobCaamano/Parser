// Roberto Caamano
// Project Assignment 2

#include "parse.h"
#include <string>
#include <stdio.h>
#include <cctype>
#include<bits/stdc++.h>
using namespace std;
map<string, bool> defVar;
map<string, Token> SymTable;
namespace Parser {
    bool pushed_back = false;
    LexItem	pushed_token;

    static LexItem GetNextToken(istream& in, int& line) {
        if( pushed_back ) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }

    static void PushBackToken(LexItem & t) {
        if( pushed_back ) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }

}
static int errCnt = 0;
int ErrCount()
{
    return errCnt;
}

void ParseError(int line, string msg)
{
    ++errCnt;
    cout << line << ": " << msg << endl;
}

bool Prog(istream& in, int& line) {
    LexItem nex = Parser::GetNextToken(in, line);
    if (nex.GetToken() == PROGRAM) {
        nex= Parser::GetNextToken(in, line);
        if (nex.GetToken() == IDENT){
            nex= Parser::GetNextToken(in, line);
            if (nex.GetToken() == INT || nex.GetToken() == FLOAT || nex.GetToken() == IF || nex.GetToken() == WRITE || nex.GetToken() == IDENT) {
                Parser::PushBackToken(nex);
                bool status = StmtList(in, line);
                if(!status)
                {
                    ParseError(line, "Incorrect Syntax in the Program");
                    return status;
                }
                nex= Parser::GetNextToken(in, line);
                if (nex.GetToken() == END) {
                    nex= Parser::GetNextToken(in, line);
                    if (nex.GetToken() == PROGRAM)
                        return true;
                    else {
                        ParseError(line, "Missing PROGRAM at End");
                        return false;
                    }
                }

            }
        } else {
            ParseError(line, "Missing Program Name.");
            return false;
        }
    }
    return false;
}

bool StmtList(istream& in, int& line){
    LexItem nex= Parser::GetNextToken(in, line);
    while(!(nex.GetToken() == END || nex.GetToken() == PROGRAM)) {
        if (nex.GetToken() == INT || nex.GetToken() == FLOAT || nex.GetToken() == IF || nex.GetToken() == WRITE || nex.GetToken() == IDENT) {
            Parser::PushBackToken(nex);
            bool status = Stmt(in, line);

            if(!status)
            {
                ParseError(line, "Incorrect Statement");
                return status;
            }
        }
        nex= Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(nex);
    return true;
}

bool Stmt(istream& in, int& line){
    bool status=true;
    LexItem t = Parser::GetNextToken(in, line);

    switch( t.GetToken() ) {
        case INT: case FLOAT:
            Parser::PushBackToken(t);
            status = DeclStmt(in, line);
            if(!status)
            {
                ParseError(line, "Incorrect declaration Statement.");
                return status;
            }
            break;
        case IF: case WRITE: case IDENT:
            Parser::PushBackToken(t);
            status = ControlStmt(in, line);
            if(!status)
            {
                ParseError(line, "Incorrect control Statement.");
                return status;
            }
            break;
        default:
            Parser::PushBackToken(t);
    }
    return true;
}

bool ControlStmt(istream& in, int& line){
    bool status=true;
    LexItem t = Parser::GetNextToken(in, line);

    switch(t.GetToken()) {
        case IDENT:
            Parser::PushBackToken(t);
            status = AssignStmt(in, line);
            if(!status)
            {
                ParseError(line, "Incorrect assign Statement.");
                return status;
            }
            break;
        case IF:
            Parser::PushBackToken(t);
            status = IfStmt(in, line);
            if(!status)
            {
                ParseError(line, "Incorrect of Statement.");
                return status;
            }
            break;
        case WRITE:
            Parser::PushBackToken(t);
            status = WriteStmt(in, line);
            if(!status)
            {
                ParseError(line, "Incorrect write Statement.");
                return status;
            }
            break;
        default:
            Parser::PushBackToken(t);
    }
    return status;
}

bool DeclStmt(istream& in, int& line) {
    LexItem nex= Parser::GetNextToken(in, line);
    if(nex.GetToken() == INT || nex.GetToken() == FLOAT ) {
        bool status = IdentList(in, line,nex);
        if (!status)
        {
            ParseError(line, "Incorrect variable in Declaration Statement.");
            return status;
        }
    }
    else{
        Parser::PushBackToken(nex);
        ParseError(line, "Incorrect Type.");
        return false;
    }
    return true;
}

bool WriteStmt(istream& in, int& line) {
    LexItem t = Parser::GetNextToken(in, line);
    bool ex = ExprList(in, line);
    if( !ex ) {
        ParseError(line, "Missing expression after Write");
        return false;
    }
    return ex;
}

bool IfStmt(istream& in, int& line){
    LexItem nex= Parser::GetNextToken(in, line);
    if(nex.GetToken() == IF){
        nex= Parser::GetNextToken(in, line);
        if (nex.GetToken() == LPAREN){
            nex= Parser::GetNextToken(in, line);
            if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST) {
                Parser::PushBackToken(nex);
                bool status = LogicExpr(in, line);
                if (!status) {
                    ParseError(line, "Incorrect logical expression Statement.");
                    return status;
                }
                nex= Parser::GetNextToken(in, line);
                if (nex.GetToken() == RPAREN) {
                    nex= Parser::GetNextToken(in, line);
                    if (nex.GetToken() == IF || nex.GetToken() == WRITE || nex.GetToken() == IDENT) {
                        Parser::PushBackToken(nex);
                        status = ControlStmt(in, line);
                        if (!status) {
                            ParseError(line, "Incorrect control Statement.");
                            return status;
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool IdentList(istream& in, int& line, LexItem tok) {
    LexItem nex= Parser::GetNextToken(in, line);
    if (nex.GetToken() == IDENT) {
        if (defVar.find(nex.GetLexeme()) != defVar.end()){
            ParseError(line, "Variable Redefined");
            return false;
        }
        defVar.insert({nex.GetLexeme(), false});
        SymTable.insert({nex.GetLexeme(), tok.GetToken()});
        nex= Parser::GetNextToken(in, line);
        
		// One IDENT
        if (nex.GetToken() == SEMICOL)
            return true;
        
        // Mult IDENT
        while (nex.GetToken() != SEMICOL) {
            if (nex.GetToken() == COMMA) {
                nex= Parser::GetNextToken(in, line);
                if (nex.GetToken() != IDENT){
                    ParseError(line, "IDENTList contains double Comma.");
                    return false;
                } else{
                    if (defVar.find(nex.GetLexeme()) != defVar.end()){
                        ParseError(line, "Variable Redefined");
                        return false;
                    }
                    defVar.insert({nex.GetLexeme(), false});
                    SymTable.insert({nex.GetLexeme(), tok.GetToken()});
                }
            }
            else {
                ParseError(line, "IDENTList contains double IDENT.");
                return false;
            }
            nex= Parser::GetNextToken(in, line);
        }
        return true;
    }
    return false;
}

bool Var(istream& in, int& line){
    LexItem nex= Parser::GetNextToken(in, line);
    if(nex.GetToken() == IDENT)
        return true;
    ParseError(line,"Not a Var.");
    return false;
}

bool AssignStmt(istream& in, int& line) {
    LexItem nex= Parser::GetNextToken(in, line);
    if (nex.GetToken() == IDENT) {
        Parser::PushBackToken(nex);
        bool status = Var(in, line);
        if (!status) {
            ParseError(line, "Incorrect variable Statement.");
            return status;
        }
        nex= Parser::GetNextToken(in, line);
        if (nex.GetToken() == ASSOP) {
            nex= Parser::GetNextToken(in, line);
            if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST ||
                nex.GetToken() == RCONST || nex.GetToken() == SCONST || nex.GetToken() == LPAREN) {
                Parser::PushBackToken(nex);
                status = Expr(in, line);
                if (!status) {
                    ParseError(line, "Incorrect expression Statement");
                    return status;
                }
                return true;
            }
        }
    }
    return false;
}

bool ExprList(istream& in, int& line) {
    LexItem nex= Parser::GetNextToken(in, line);
    if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST) {
        Parser::PushBackToken(nex);
        bool status = Expr(in, line);
        if (!status) {
            ParseError(line, "Incorrect expression 2 Statement.");
            return status;
        }
        nex= Parser::GetNextToken(in, line);
        if (nex.GetToken() == SEMICOL) {
            Parser::PushBackToken(nex);
            return true;
        }
        while (nex.GetToken() != SEMICOL) {
            if (nex.GetToken() == COMMA) {
                nex= Parser::GetNextToken(in, line);
                if (!(nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST)){
                    ParseError(line, "ExprList contains double Comma.");
                    return false;
                }
            }
            else {
                ParseError(line, "ExprList contains double Expr.");
                return false;
            }
            nex= Parser::GetNextToken(in, line);
        }
        Parser::PushBackToken(nex);
        return true;
    }
    return false;
}
bool LogicExpr(istream& in, int& line){
    LexItem nex= Parser::GetNextToken(in, line);
    if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST) {
        Parser::PushBackToken(nex);
        bool status = Expr(in, line);
        if (!status) {
            ParseError(line, "Incorrect expression 3 Statement.");
            return status;
        }
        nex= Parser::GetNextToken(in, line);
        if (nex.GetToken() == EQUAL || nex.GetToken() == GTHAN){
            nex= Parser::GetNextToken(in, line);
            if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST){
                Parser::PushBackToken(nex);
                status = Expr(in, line);
                if (!status) {
                    ParseError(line, "Incorrect expression 4 Statement.");
                    return status;
                }
                return true;
            }
        }
    }
    return false;
}
bool Expr(istream& in, int& line){
    LexItem nex= Parser::GetNextToken(in, line);
    if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST || nex.GetToken() == LPAREN) {
        Parser::PushBackToken(nex);
        bool status = Term(in, line);
        if (!status) {
            ParseError(line, "Incorrect term 1 Statement.");
            return status;
        }
        nex= Parser::GetNextToken(in, line);
        if (nex.GetToken() == SEMICOL || nex.GetToken() == GTHAN || nex.GetToken() ==  RPAREN) {
            Parser::PushBackToken(nex);
            return true;
        }

        while (nex.GetToken() != SEMICOL && nex.GetToken() != GTHAN && nex.GetToken() != RPAREN && nex.GetToken() != COMMA) {
            if (nex.GetToken() == PLUS || nex.GetToken() == MINUS) {
                nex= Parser::GetNextToken(in, line);
                if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST || nex.GetToken() == LPAREN) {
                    Parser::PushBackToken(nex);
                    status = Term(in, line);
                    if (!status) {
                        ParseError(line, "Incorrect term 2 Statement.");
                        return status;
                    }
                }
                else {
                    ParseError(line, "Double operators.");
                    return false;
                }
            }
            else {
                ParseError(line, "Double 1 Term.");
                return false;
            }
            nex= Parser::GetNextToken(in, line);
        }
        Parser::PushBackToken(nex);
        return true;
    }
    return false;
}
bool Term(istream& in, int& line){
    LexItem nex= Parser::GetNextToken(in, line);
    if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST || nex.GetToken() == LPAREN) {
        Parser::PushBackToken(nex);
        bool status = SFactor(in, line);
        if (!status) {
            ParseError(line, "Incorrect SFactor Statement.");
            return status;
        }

        nex= Parser::GetNextToken(in, line);
        if (nex.GetToken() == SEMICOL || nex.GetToken() == GTHAN || nex.GetToken() ==  RPAREN) {
            Parser::PushBackToken(nex);
            return true;
        }
        while (nex.GetToken() != SEMICOL && nex.GetToken() != GTHAN && nex.GetToken() != RPAREN && nex.GetToken() != PLUS && nex.GetToken() != MINUS && nex.GetToken() != COMMA) {
            if (nex.GetToken() == MULT || nex.GetToken() == DIV || nex.GetToken() == REM) {
                nex= Parser::GetNextToken(in, line);
                if (nex.GetToken() == PLUS || nex.GetToken() == MINUS || nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST) {
                    Parser::PushBackToken(nex);
                    status = SFactor(in, line);
                    if (!status) {
                        ParseError(line, "Incorrect SFactor Statement.");
                        return status;
                    }
                }
                else {
                    ParseError(line, "Double operators.");
                    return false;
                }
            }
            else {
                ParseError(line, "Double Terms.");
                return false;
            }
            nex= Parser::GetNextToken(in, line);
        }
        Parser::PushBackToken(nex);
        return true;
    }
    return false;
}
bool SFactor(istream& in, int& line){
    LexItem nex= Parser::GetNextToken(in, line);
    if (nex.GetToken() == PLUS || nex.GetToken() == MINUS) {
        nex= Parser::GetNextToken(in, line);
    }
    if (nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST || nex.GetToken() == LPAREN) {
        Parser::PushBackToken(nex);
        bool status = Factor(in, line,0);
        if (!status) {
            ParseError(line, "Incorrect factor Statement");
            return status;
        }
        return true;
    }
    return false;
}
bool Factor(istream& in, int& line, int sign){
    LexItem nex= Parser::GetNextToken(in, line);
    if (nex.GetToken() == IDENT || nex.GetToken() == ICONST || nex.GetToken() == RCONST || nex.GetToken() == SCONST)
        return true;
    if (nex.GetToken() == LPAREN) {
        bool status = Expr(in, line);
        if (!status) {
            ParseError(line, "Incorrect Expression in Parenthesis.");
            return status;
        }
        nex= Parser::GetNextToken(in, line);
        if (nex.GetToken() == RPAREN)
            return true;
    }
    return false;
}