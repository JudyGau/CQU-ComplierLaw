#include "front/lexical.h"

#include <map>
#include <cassert>
#include <string>
#include <cctype>
#include <algorithm>
#include <regex>

#define TODO assert(0 && "todo")

// #define DEBUG_DFA
// #define DEBUG_SCANNER

std::string frontend::toString(State s)
{
    switch (s)
    {
    case State::Empty:
        return "Empty";
    case State::Ident:
        return "Ident";
    case State::IntLiteral:
        return "IntLiteral";
    case State::FloatLiteral:
        return "FloatLiteral";
    case State::op:
        return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

std::set<std::string> frontend::keywords = {
    "const", "int", "float", "if", "else", "while", "continue", "break", "return", "void"};

frontend::DFA::DFA() : cur_state(frontend::State::Empty), cur_str() {}

frontend::DFA::~DFA() {}

//  helper  function,  you  are  not  require  to  implement  these,  but  they  may  be  helpful
bool isoperator(char c)
{
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '<' || c == '>' || c == ':' || c == '=' || c == ';' || c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '!' || c == '&' || c == '|')
    {
        return true;
    }
    else
    {
        return false;
    }
}

frontend::TokenType get_op_type(std::string s)
{
    if (s == "+")
    {
        return frontend::TokenType::PLUS;
    }
    else if (s == "-")
    {
        return frontend::TokenType::MINU;
    }
    else if (s == "*")
    {
        return frontend::TokenType::MULT;
    }
    else if (s == "/")
    {
        return frontend::TokenType::DIV;
    }
    else if (s == "%")
    {
        return frontend::TokenType::MOD;
    }
    else if (s == "<")
    {
        return frontend::TokenType::LSS;
    }
    else if (s == ">")
    {
        return frontend::TokenType::GTR;
    }
    else if (s == ":")
    {
        return frontend::TokenType::COLON;
    }
    else if (s == "=")
    {
        return frontend::TokenType::ASSIGN;
    }
    else if (s == ";")
    {
        return frontend::TokenType::SEMICN;
    }
    else if (s == ",")
    {
        return frontend::TokenType::COMMA;
    }
    else if (s == "(")
    {
        return frontend::TokenType::LPARENT;
    }
    else if (s == ")")
    {
        return frontend::TokenType::RPARENT;
    }
    else if (s == "{")
    {
        return frontend::TokenType::LBRACE;
    }
    else if (s == "}")
    {
        return frontend::TokenType::RBRACE;
    }
    else if (s == "[")
    {
        return frontend::TokenType::LBRACK;
    }
    else if (s == "]")
    {
        return frontend::TokenType::RBRACK;
    }
    else if (s == "!")
    {
        return frontend::TokenType::NOT;
    }
    else if (s == "<=")
    {
        return frontend::TokenType::LEQ;
    }
    else if (s == ">=")
    {
        return frontend::TokenType::GEQ;
    }
    else if (s == "==")
    {
        return frontend::TokenType::EQL;
    }
    else if (s == "!=")
    {
        return frontend::TokenType::NEQ;
    }
    else if (s == "&&")
    {
        return frontend::TokenType::AND;
    }
    else if (s == "||")
    {
        return frontend::TokenType::OR;
    }

    else
    {
        std::cout << s << std::endl;
        assert(0 && ("invalid  token  type" + s).c_str());
    }
}

frontend::TokenType get_ident_type(std::string s)
{
    if (s == "const")
    {
        return frontend::TokenType::CONSTTK;
    }
    else if (s == "void")
    {
        return frontend::TokenType::VOIDTK;
    }
    else if (s == "int")
    {
        return frontend::TokenType::INTTK;
    }
    else if (s == "float")
    {
        return frontend::TokenType::FLOATTK;
    }
    else if (s == "if")
    {
        return frontend::TokenType::IFTK;
    }
    else if (s == "else")
    {
        return frontend::TokenType::ELSETK;
    }
    else if (s == "while")
    {
        return frontend::TokenType::WHILETK;
    }
    else if (s == "continue")
    {
        return frontend::TokenType::CONTINUETK;
    }
    else if (s == "break")
    {
        return frontend::TokenType::BREAKTK;
    }
    else if (s == "return")
    {
        return frontend::TokenType::RETURNTK;
    }
    else
    {
        return frontend::TokenType::IDENFR;
    }
}

bool frontend::DFA::next(char input, Token &buf)
{
#ifdef DEBUG_DFA
#include <iostream>
    std::cout << "in state [" << toString(cur_state) << "], input = \'" << input << "\', str = " << cur_str << "\t";
#endif

    switch (cur_state)
    {
    case State::Empty:
        if (isdigit(input))
        {
            cur_state = State::IntLiteral;
            cur_str += input;
            return false;
        }
        else if (input == '.')
        {
            cur_state = State::FloatLiteral;
            cur_str += input;
            return false;
        }
        else if (isoperator(input))
        {
            cur_state = State::op;
            cur_str += input;
            return false;
        }
        else if (isalpha(input) || input == '_')
        {
            cur_state = State::Ident;
            cur_str += input;
            return false;
        }
        else
        {
            return false;
        }

    case State::IntLiteral:
        if (isdigit(input) || isalpha(input))
        {
            cur_str += input;
            return false;
        }
        else if (isoperator(input))
        {
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;

            reset();
            cur_state = State::op;
            cur_str += input;

            return true;
        }
        else if (input == '.')
        {
            cur_state = State::FloatLiteral;
            cur_str += input;
            return false;
        }
        else
        {
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;

            reset();

            return true;
        }

    case State::FloatLiteral:
        if (isdigit(input))
        {
            cur_str += input;
            return false;
        }
        else if (isalpha(input) || input == '_')
        {
            buf.type = TokenType::FLOATLTR;
            buf.value = cur_str;

            reset();
            cur_state = State::Ident;
            cur_str += input;

            return true;
        }
        else if (isoperator(input))
        {
            buf.type = TokenType::FLOATLTR;
            buf.value = cur_str;

            reset();
            cur_state = State::op;
            cur_str += input;

            return true;
        }
        else
        {
            buf.type = TokenType::FLOATLTR;
            buf.value = cur_str;

            reset();

            return true;
        }

    case State::op:
        if (isdigit(input))
        {
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;

            reset();

            cur_state = State::IntLiteral;
            cur_str += input;

            return true;
        }
        else if (input == '.')
        {
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;

            reset();

            cur_state = State::FloatLiteral;
            cur_str += input;

            return true;
        }
        else if (isalpha(input) || input == '_')
        {
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;

            reset();

            cur_state = State::Ident;
            cur_str += input;

            return true;
        }
        else if (isoperator(input))
        {
            if (cur_str + input == "<=" || cur_str + input == ">=" || cur_str + input == "==" || cur_str + input == "!=" || cur_str + input == "&&" || cur_str + input == "||")
            {
                cur_str += input;
                return false;
            }
            else
            {
                buf.type = get_op_type(cur_str);
                buf.value = cur_str;

                reset();

                cur_state = State::op;
                cur_str += input;

                return true;
            }
        }
        else
        {
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;

            reset();

            return true;
        }

    case State::Ident:
        if (isdigit(input))
        {
            cur_str += input;
            return false;
        }
        else if (isalpha(input) || input == '_')
        {
            cur_str += input;
            return false;
        }
        else if (input == '_')
        {
            cur_str += input;
            return false;
        }
        else if (isoperator(input))
        {
            buf.type = get_ident_type(cur_str);
            buf.value = cur_str;

            reset();
            cur_state = State::op;
            cur_str += input;

            return true;
        }
        else
        {
            buf.type = get_ident_type(cur_str);
            buf.value = cur_str;

            reset();

            return true;
        }
    }

#ifdef DEBUG_DFA
    std::cout << "next state is [" << toString(cur_state) << "], next str = " << cur_str << "\t, ret = " << ret << std::endl;
#endif
}

// 去除单行注释
std::string removeSingleLineComments(const std::string &s)
{
    std::regex pattern(R"((?:\/\/)[^\n\r]*)");
    return std::regex_replace(s, pattern, "");
}

// 去除多行注释
std::string removeMultiLineComments(const std::string &s)
{
    std::regex pattern(R"((\/\*([^*]|[\r\n]|(\*+([^*\/]|[\r\n])))*\*+\/))");
    return std::regex_replace(s, pattern, "");
}

// 去除所有注释
std::string removeComments(const std::string &s)
{
    return removeMultiLineComments(removeSingleLineComments(s));
}

// 预处理，去除注释
std::string preprocess(std::ifstream &fin)
{
    std::string line;
    std::string fileContent;
    while (std::getline(fin, line))
    {
        fileContent += line + "\n";
    }
    fin.close();

    // 去除注释
    std::string contentWithoutComments = removeComments(fileContent);

    // 返回结果

    return contentWithoutComments;
}

void frontend::DFA::reset()
{
    cur_state = State::Empty;
    cur_str = "";
}

frontend::Scanner::Scanner(std::string filename) : fin(filename)
{
    if (!fin.is_open())
    {
        assert(0 && "in Scanner constructor, input file cannot open");
    }
}

frontend::Scanner::~Scanner()
{
    fin.close();
}

std::vector<frontend::Token> frontend::Scanner::run()
{

    // TODO;

    std::string stdin_str = preprocess(fin);
    // std::string stdin_str((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    stdin_str += "\n";
    DFA dfa;
    Token tk;
    std::vector<Token> tokens;
    for (size_t i = 0; i < stdin_str.size(); i++)
    {
        if (dfa.next(stdin_str[i], tk))
        {
            tokens.push_back(tk);
        }
    }
    return tokens;

#ifdef DEBUG_SCANNER
#include <iostream>
    std::cout << "token: " << toString(tk.type) << "\t" << tk.value << std::endl;
#endif
}