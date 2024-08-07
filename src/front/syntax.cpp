#include "front/syntax.h"

#include <iostream>
#include <cassert>

using frontend::AstNode;
using frontend::Parser;
using frontend::TokenType;

// #define DEBUG_PARSER
#define TODO assert(0 && "todo")
#define CUR_TOKEN_IS(tk_type) (token_stream[index].type == TokenType::tk_type)
#define PARSE_TOKEN(tk_type) root->children.push_back(parseTerm(root, TokenType::tk_type))
#define PARSE(name, type)       \
    auto name = new type(root); \
    assert(parse##type(name));  \
    root->children.push_back(name);

Parser::Parser(const std::vector<frontend::Token> &tokens) : index(0), token_stream(tokens) {}

Parser::~Parser() {}

bool Parser::parseCompUnit(CompUnit *root)
{
    log(root);

    if (CUR_TOKEN_IS(CONSTTK))
    {
        PARSE(decl, Decl);
    }
    else if (CUR_TOKEN_IS(VOIDTK))
    {
        PARSE(funcdef, FuncDef);
    }
    else if (token_stream[index + 2].type == TokenType::LPARENT)
    {
        PARSE(funcdef, FuncDef);
    }
    else
    {
        PARSE(decl, Decl);
    }

    if (index < token_stream.size())
    {

        PARSE(compunit, CompUnit);
    }

    // while (index < token_stream.size())
    // {
    //     if (CUR_TOKEN_IS(CONSTTK))
    //     {
    //         PARSE(decl, Decl);
    //     }
    //     else if (CUR_TOKEN_IS(VOIDTK))
    //     {
    //         PARSE(funcdef, FuncDef);
    //     }
    //     else if (token_stream[index + 2].type == TokenType::LPARENT)
    //     {
    //         PARSE(funcdef, FuncDef);
    //     }
    //     else
    //     {
    //         PARSE(decl, Decl);
    //     }
    // }

    return true;
}

bool Parser::parseDecl(Decl *root)
{
    log(root);
    if (CUR_TOKEN_IS(CONSTTK))
    {
        PARSE(constdecl, ConstDecl);
    }
    else
    {

        PARSE(vardecl, VarDecl);
    }

    return true;
}

bool Parser::parseFuncDef(FuncDef *root)
{
    log(root);
    PARSE(functype, FuncType);
    PARSE_TOKEN(IDENFR);
    PARSE_TOKEN(LPARENT);
    //	no	[FuncFParams],	FuncType	Ident	'('	')'	Block
    if (CUR_TOKEN_IS(RPARENT))
    {
        PARSE_TOKEN(RPARENT);
    }
    //	FuncType	Ident	'('	FuncFParams	')'	Block
    else
    {
        PARSE(node, FuncFParams);
        PARSE_TOKEN(RPARENT);
    }

    PARSE(block, Block);
    return true;
}

bool Parser::parseConstDecl(ConstDecl *root)
{
    log(root);
    PARSE_TOKEN(CONSTTK);
    PARSE(btype, BType);
    PARSE(constdef, ConstDef);

    while (index < token_stream.size() && CUR_TOKEN_IS(COMMA))
    {
        PARSE_TOKEN(COMMA);
        PARSE(constdef, ConstDef);
    }
    PARSE_TOKEN(SEMICN);
    return true;
}

bool Parser::parseBType(BType *root)
{
    log(root);
    if (CUR_TOKEN_IS(INTTK))
    {
        PARSE_TOKEN(INTTK);
    }
    else if (CUR_TOKEN_IS(FLOATTK))
    {
        PARSE_TOKEN(FLOATTK);
    }
    return true;
}

bool Parser::parseConstDef(ConstDef *root)
{
    log(root);
    PARSE_TOKEN(IDENFR);
    while (index < token_stream.size() && CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE(constexp, ConstExp);
        PARSE_TOKEN(RBRACK);
    }

    PARSE_TOKEN(ASSIGN);
    PARSE(constinitval, ConstInitVal);
    return true;
}

bool Parser::parseConstInitVal(ConstInitVal *root)
{
    log(root);
    if (CUR_TOKEN_IS(LBRACE))
    {
        PARSE_TOKEN(LBRACE);
        if (!CUR_TOKEN_IS(RBRACE))
        {
            PARSE(constinitval, ConstInitVal);
            while (index < token_stream.size() && CUR_TOKEN_IS(COMMA))
            {
                PARSE_TOKEN(COMMA);
                PARSE(constinitval, ConstInitVal);
            }
        }

        PARSE_TOKEN(RBRACE);
    }
    else
    {
        PARSE(constexp, ConstExp);
    }
    return true;
}

bool Parser::parseVarDecl(frontend::VarDecl *root)
{
    log(root);
    PARSE(btype, BType);
    PARSE(vardef, VarDef);

    while (index < token_stream.size() && CUR_TOKEN_IS(COMMA))
    {
        PARSE_TOKEN(COMMA);
        PARSE(vardef, VarDef);
    }
    PARSE_TOKEN(SEMICN);
    return true;
}

bool Parser::parseVarDef(frontend::VarDef *root)
{
    log(root);
    PARSE_TOKEN(IDENFR);
    while (index < token_stream.size() && CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE(constexp, ConstExp);
        PARSE_TOKEN(RBRACK);
    }

    if (CUR_TOKEN_IS(ASSIGN))
    {
        PARSE_TOKEN(ASSIGN);
        PARSE(initval, InitVal);
    }

    return true;
}

bool Parser::parseInitVal(InitVal *root)
{
    log(root);
    if (CUR_TOKEN_IS(LBRACE))
    {
        PARSE_TOKEN(LBRACE);
        if (!CUR_TOKEN_IS(RBRACE))
        {
            PARSE(initval, InitVal);
            while (index < token_stream.size() && CUR_TOKEN_IS(COMMA))
            {
                PARSE_TOKEN(COMMA);
                PARSE(initval, InitVal);
            }
        }

        PARSE_TOKEN(RBRACE);
    }
    else
    {
        PARSE(exp, Exp);
    }
    return true;
}

bool Parser::parseFuncType(FuncType *root)
{
    log(root);
    if (CUR_TOKEN_IS(VOIDTK))
    {
        PARSE_TOKEN(VOIDTK);
    }
    else if (CUR_TOKEN_IS(INTTK))
    {
        PARSE_TOKEN(INTTK);
    }
    else if (CUR_TOKEN_IS(FLOATTK))
    {
        PARSE_TOKEN(FLOATTK);
    }
    return true;
}

bool Parser::parseFuncFParam(FuncFParam *root)
{
    log(root);
    PARSE(btype, BType);
    PARSE_TOKEN(IDENFR);
    if (CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE_TOKEN(RBRACK);
        while (index < token_stream.size() && CUR_TOKEN_IS(LBRACK))
        {
            PARSE_TOKEN(LBRACK);
            PARSE(exp, Exp);
            PARSE_TOKEN(RBRACK);
        }
    }

    return true;
}

bool Parser::parseFuncFParams(FuncFParams *root)
{
    log(root);
    PARSE(funcfparam, FuncFParam);
    while (index < token_stream.size() && CUR_TOKEN_IS(COMMA))
    {
        PARSE_TOKEN(COMMA);
        PARSE(funcparam, FuncFParam);
    }

    return true;
}

bool Parser::parseBlock(Block *root)
{
    log(root);
    PARSE_TOKEN(LBRACE);
    while (index < token_stream.size() && !CUR_TOKEN_IS(RBRACE))
    {
        PARSE(blockitem, BlockItem);
    }

    PARSE_TOKEN(RBRACE);
    return true;
}

bool Parser::parseBlockItem(BlockItem *root)
{
    log(root);
    if (CUR_TOKEN_IS(CONSTTK) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK))
    {

        PARSE(decl, Decl);
    }
    else
    {
        PARSE(stmt, Stmt);
    }
    return true;
}

bool Parser::parseStmt(Stmt *root)
{
    log(root);
    if (CUR_TOKEN_IS(IDENFR) && token_stream[index+1].type!=TokenType::LPARENT)
    {
        PARSE(lval, LVal);

        PARSE_TOKEN(ASSIGN);
        PARSE(exp, Exp);
        PARSE_TOKEN(SEMICN);
    }
    else if (CUR_TOKEN_IS(LBRACE))
    {
        PARSE(block, Block);
    }
    else if (CUR_TOKEN_IS(IFTK))
    {
        PARSE_TOKEN(IFTK);
        PARSE_TOKEN(LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(stmt, Stmt);

        if (CUR_TOKEN_IS(ELSETK))
        {
            PARSE_TOKEN(ELSETK);
            PARSE(stmt, Stmt);
        }
    }
    else if (CUR_TOKEN_IS(WHILETK))
    {
        PARSE_TOKEN(WHILETK);
        PARSE_TOKEN(LPARENT);
        PARSE(cond, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(stmt, Stmt);
    }
    else if (CUR_TOKEN_IS(BREAKTK))
    {
        PARSE_TOKEN(BREAKTK);
        PARSE_TOKEN(SEMICN);
    }
    else if (CUR_TOKEN_IS(CONTINUETK))
    {
        PARSE_TOKEN(CONTINUETK);
        PARSE_TOKEN(SEMICN);
    }
    else if (CUR_TOKEN_IS(RETURNTK))
    {
        PARSE_TOKEN(RETURNTK);
        if (!CUR_TOKEN_IS(SEMICN))
        {
            PARSE(exp, Exp);
        }

        PARSE_TOKEN(SEMICN);
    }
    else
    {
        if (!CUR_TOKEN_IS(SEMICN))
        {
            PARSE(exp, Exp);
        }

        PARSE_TOKEN(SEMICN);
    }
    return true;
}

bool Parser::parseExp(Exp *root)
{
    log(root);
    PARSE(addexp, AddExp);
    return true;
}

bool Parser::parseCond(Cond *root)
{
    log(root);
    PARSE(lorexp, LOrExp);
    return true;
}

bool Parser::parseLVal(LVal *root)
{
    log(root);
    PARSE_TOKEN(IDENFR);
    while (index < token_stream.size() && CUR_TOKEN_IS(LBRACK))
    {
        PARSE_TOKEN(LBRACK);
        PARSE(exp, Exp);
        PARSE_TOKEN(RBRACK);
    }

    return true;
}

bool Parser::parseNumber(Number *root)
{
    log(root);
    if (CUR_TOKEN_IS(INTLTR))
    {
        PARSE_TOKEN(INTLTR);
    }

    if (CUR_TOKEN_IS(FLOATLTR))
    {
        PARSE_TOKEN(FLOATLTR);
    }

    return true;
}

bool Parser::parsePrimaryExp(PrimaryExp *root)
{
    log(root);
    if (CUR_TOKEN_IS(LPARENT))
    {
        PARSE_TOKEN(LPARENT);
        PARSE(exp, Exp);
        PARSE_TOKEN(RPARENT);
    }
    else if (CUR_TOKEN_IS(IDENFR))
    {
        PARSE(lval, LVal);
    }
    else
    {
        PARSE(number, Number);
    }

    return true;
}

bool Parser::parseUnaryExp(UnaryExp *root)
{
    log(root);
    if (CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU) || CUR_TOKEN_IS(NOT))
    {
        PARSE(unaryop, UnaryOp);
        PARSE(unaryexp, UnaryExp);
    }
    else if (CUR_TOKEN_IS(IDENFR) && token_stream[index + 1].type == TokenType::LPARENT)
    {
        PARSE_TOKEN(IDENFR);
        PARSE_TOKEN(LPARENT);
        if (!CUR_TOKEN_IS(RPARENT))
        {
            PARSE(funcrparams, FuncRParams);
        }

        PARSE_TOKEN(RPARENT);
    }
    else
    {
        PARSE(primaryexp, PrimaryExp);
    }
    return true;
}

bool Parser::parseUnaryOp(UnaryOp *root)
{
    log(root);
    if (CUR_TOKEN_IS(PLUS))
    {
        PARSE_TOKEN(PLUS);
    }
    else if (CUR_TOKEN_IS(MINU))
    {
        PARSE_TOKEN(MINU);
    }
    else if (CUR_TOKEN_IS(NOT))
    {
        PARSE_TOKEN(NOT);
    }
    return true;
}

bool Parser::parseFuncRParams(FuncRParams *root)
{
    log(root);
    PARSE(exp, Exp);
    while (index < token_stream.size() && CUR_TOKEN_IS(COMMA))
    {
        PARSE_TOKEN(COMMA);
        PARSE(exp, Exp);
    }

    return true;
}

bool Parser::parseMulExp(MulExp *root)
{
    log(root);
    PARSE(unaryexp, UnaryExp);

    while (index < token_stream.size() && (CUR_TOKEN_IS(MULT) || CUR_TOKEN_IS(DIV) || CUR_TOKEN_IS(MOD)))
    {
        if (CUR_TOKEN_IS(MULT))
        {
            PARSE_TOKEN(MULT);
        }
        else if (CUR_TOKEN_IS(DIV))
        {
            PARSE_TOKEN(DIV);
        }
        else if (CUR_TOKEN_IS(MOD))
        {
            PARSE_TOKEN(MOD);
        }

        PARSE(unary, UnaryExp);
    }
    return true;
}

bool Parser::parseAddExp(AddExp *root)
{
    log(root);
    PARSE(mulexp, MulExp);

    while (index < token_stream.size() && (CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU)))
    {
        if (CUR_TOKEN_IS(PLUS))
        {
            PARSE_TOKEN(PLUS);
        }
        else if (CUR_TOKEN_IS(MINU))
        {
            PARSE_TOKEN(MINU);
        }

        PARSE(mulexp, MulExp);
    }
    return true;
}

bool Parser::parseRelExp(RelExp *root)
{
    log(root);
    PARSE(addexp, AddExp);
    while (index < token_stream.size() && (CUR_TOKEN_IS(LSS) || CUR_TOKEN_IS(GTR) || CUR_TOKEN_IS(LEQ) || CUR_TOKEN_IS(GEQ)))
    {
        if (CUR_TOKEN_IS(LSS))
        {
            PARSE_TOKEN(LSS);
        }
        else if (CUR_TOKEN_IS(GTR))
        {
            PARSE_TOKEN(GTR);
        }
        else if (CUR_TOKEN_IS(LEQ))
        {
            PARSE_TOKEN(LEQ);
        }
        else if (CUR_TOKEN_IS(GEQ))
        {
            PARSE_TOKEN(GEQ);
        }

        PARSE(addexp, AddExp);
    }

    return true;
}

bool Parser::parseEqExp(EqExp *root)
{
    log(root);
    PARSE(relexp, RelExp);
    while (index < token_stream.size() && (CUR_TOKEN_IS(EQL) || CUR_TOKEN_IS(NEQ)))
    {
        if (CUR_TOKEN_IS(EQL))
        {
            PARSE_TOKEN(EQL);
        }
        else if (CUR_TOKEN_IS(NEQ))
        {
            PARSE_TOKEN(NEQ);
        }

        PARSE(relexp, RelExp);
    }
    return true;
}

bool Parser::parseLAndExp(LAndExp *root)
{
    log(root);
    PARSE(eqexp, EqExp);
    if (index < token_stream.size() && (CUR_TOKEN_IS(AND)))
    {
        PARSE_TOKEN(AND);
        PARSE(landxp, LAndExp);
    }

    return true;
}

bool Parser::parseLOrExp(LOrExp *root)
{
    log(root);

    PARSE(landexp, LAndExp);
    if (index < token_stream.size() && (CUR_TOKEN_IS(OR)))
    {
        PARSE_TOKEN(OR);
        PARSE(lorexp, LOrExp);
    }
    return true;
}

bool Parser::parseConstExp(ConstExp *root)
{
    log(root);
    PARSE(addexp, AddExp);
    return true;
}

frontend::Term *frontend::Parser::parseTerm(frontend::AstNode *parent, frontend::TokenType expected)
{
    log(parent);
    if (token_stream[index].type == expected)
    {
        Term *temp = new Term(token_stream[index], parent);
        index++;
        // parent->children.push_back(temp);
        return temp;
    }
    else
    {
        assert(0);
    }
}

frontend::CompUnit *Parser::get_abstract_syntax_tree()
{

    CompUnit *root = new CompUnit(nullptr);
    PARSE(compunit, CompUnit);

    CompUnit *ans = new CompUnit(nullptr);
    ans->children = root->children[0]->children;

    return ans;
}

void Parser::log(AstNode *node)
{

    //std::cout << "in parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << '\n';
}
