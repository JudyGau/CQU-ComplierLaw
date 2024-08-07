/**
 * @file semantic.h
 * @author Yuntao Dai (d1581209858@live.com)
 * @brief
 * @version 0.1
 * @date 2023-01-06
 *
 * a Analyzer should
 * @copyright Copyright (c) 2023
 *
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ir/ir.h"
#include "front/abstract_syntax_tree.h"

#include <map>
#include <string>
#include <vector>
using std::map;
using std::string;
using std::vector;

namespace frontend
{

    // 符号表中的一条记录
    struct STE
    {
        ir::Operand operand;   // 符号的名字和类型
        vector<int> dimension; // for数组
    };

    using map_str_ste = map<string, STE>; // string 是操作数的原始名称
    // 作用域
    struct ScopeInfo
    {
        int cnt;           // 作用域在函数中的唯一编号, 代表是函数中出现的第几个作用域
        string name;       // 分辨作用域的类别, 'b' 代表是一个单独嵌套的作用域, 'i' 'e' 'w' 分别代表由 if else while 产生的新作用域
        map_str_ste table; // 一张存放符号的表
    };

    // surpport lib functions
    map<std::string, ir::Function *> *get_lib_funcs();

    // 符号表，栈式结构
    struct SymbolTable
    {
        vector<ScopeInfo> scope_stack;
        map<std::string, ir::Function *> functions;

        /**
         * @brief 进入新作用域时, 向符号表中添加 ScopeInfo, 相当于压栈
         */
        void add_scope();

        /**
         * @brief 退出时弹栈
         */
        void exit_scope();

        /**
         * @brief 输入一个变量名, 返回其在当前作用域下重命名后的名字 (相当于加后缀)
         * for example, we have these code:
         * "
         * int a;
         * {
         *      int a; ....
         * }
         * "
         * in this case, we have two variable both name 'a', after change they will be 'a' and 'a_block'
         * @param id: origin id
         * @return string: new name with scope infomations
         */
        string get_scoped_name(string id) const;

        /**
         * @brief 输入一个变量名, 在符号表中寻找最近的同名变量, 返回对应的 Operand(注意，此 Operand 的 name 是重命名后的)
         * @param id identifier name 标识符名称
         * @return Operand
         */
        ir::Operand get_operand(string id) const;

        /**
         * @brief  输入一个变量名, 在符号表中寻找最近的同名变量, 返回 STE
         * @param id identifier name
         * @return STE
         */
        STE get_ste(string id) const;

        void add_operand(std::string name, STE ste);
    };

    // singleton class
    struct Analyzer
    {
        int tmp_cnt;
        vector<ir::Instruction *> g_init_inst;
        SymbolTable symbol_table;
        int float_flag=0;
        
        /**
         * @brief constructor
         */
        Analyzer();

        // analysis functions
        ir::Program get_ir_program(CompUnit *);

        // reject copy & assignment
        Analyzer(const Analyzer &) = delete;
        Analyzer &operator=(const Analyzer &) = delete;

        // ir::Operand get_temp(ir::Type);
        std::string get_temp_name();
        void delete_temp_name();

        // analysis functions
        void analysisCompUnit(CompUnit *);

        void analysisDecl(Decl *, vector<ir::Instruction *> &);
        void analysisConstDecl(ConstDecl *, vector<ir::Instruction *> &);
        void analysisConstDef(ConstDef *, vector<ir::Instruction *> &, ir::Type);

        void analysisFuncDef(FuncDef *);
        void analysisFuncType(FuncType *, ir::Type &);
        void analysisFuncFParams(FuncFParams *, vector<ir::Operand> &);
        void analysisFuncFParam(FuncFParam *, vector<ir::Operand> &); // TODO

        void analysisBlock(Block *, vector<ir::Instruction *> &);
        void analysisBlockItem(BlockItem *, vector<ir::Instruction *> &);

        void analysisStmt(Stmt *, vector<ir::Instruction *> &);
        void analysisExp(Exp *, vector<ir::Instruction *> &);

        void analysisAddExp(AddExp *, vector<ir::Instruction *> &);
        void analysisMulExp(MulExp *, vector<ir::Instruction *> &);
        void analysisUnaryExp(UnaryExp *, vector<ir::Instruction *> &);
        void analysisUnaryOp(UnaryOp *, vector<ir::Instruction *> &);
        void analysisFuncRParams(FuncRParams *, vector<ir::Operand> &, vector<ir::Instruction *> &);
        void analysisPrimaryExp(PrimaryExp *, vector<ir::Instruction *> &);
        void analysisNumber(Number *, vector<ir::Instruction *> &);
        void analysisLVal(LVal *, vector<ir::Instruction *> &, bool);

        void analysisCond(Cond *, vector<ir::Instruction *> &);
        void analysisLOrExp(LOrExp *, vector<ir::Instruction *> &);
        void analysisLAndExp(LAndExp *, vector<ir::Instruction *> &);
        void analysisEqExp(EqExp *, vector<ir::Instruction *> &);
        void analysisRelExp(RelExp *, vector<ir::Instruction *> &);

        void analysisConstDef(ConstDef *, vector<ir::Instruction *> &);
        void analysisConstInitVal(ConstInitVal *, vector<ir::Instruction *> &);

        void analysisVarDecl(VarDecl *, vector<ir::Instruction *> &);
        void analysisBType(BType *);
        void analysisVarDef(VarDef *, vector<ir::Instruction *> &, ir::Type);
        void analysisConstExp(ConstExp *, vector<ir::Instruction *> &);
        void analysisInitVal(InitVal *, vector<ir::Instruction *> &);
    };

} // namespace frontend

#endif

