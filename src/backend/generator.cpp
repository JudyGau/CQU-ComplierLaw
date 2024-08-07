#include "backend/generator.h"

#include <assert.h>
#include <set>
#include <vector>
#include <iostream>
#include <unordered_map>

#define TODO assert(0 && "todo")

const std::vector<std::string> returnArgs{"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};

int backend::stackVarMap::add_operand(ir::Operand operand, uint32_t size)
{

    _table[operand.name] = s;
    s += size;
    //_table[operand.name] = _table.size() * 4;
    return _table[operand.name];
}

int backend::stackVarMap::find_operand(ir::Operand operand)
{
    return _table[operand.name];
}

backend::Generator::Generator(ir::Program &p, std::ofstream &f) : program(p), fout(f), label_cnt(0) {}

void backend::Generator::load(ir::Operand operand, std::string reg, int offset)
{
    // if it is a global variable
    if (global_vals.count(operand.name))
    {
        fout << "\tla " << reg << ", " << operand.name << std::endl;
        fout << "\tlw " << reg << ", " << offset << "(" << reg << ")" << std::endl;
        return;
    }

    fout << "\tlw " << reg << ", " << stackVar.find_operand(operand) + offset << "(sp)" << std::endl;
}

void backend::Generator::load(ir::Operand operand, std::string reg, std::string offset)
{
    // if it is a global variable
    if (global_vals.count(operand.name))
    {
        auto temp_reg = get_temp_reg();
        fout << "\tla " << temp_reg << ", " << operand.name << std::endl;
        fout << "\tadd " << temp_reg << ", " << temp_reg << ", " << offset << std::endl;
        fout << "\tlw " << reg << ", "
             << "0(" + temp_reg + ")" << std::endl;
        free_temp_reg(temp_reg);
        return;
    }
    auto temp_reg = get_temp_reg();
    // fout << "\taddi " << temp_reg << ", " << offset << ", " << stackVar.find_operand(operand) << std::endl;
    // fout << "\tadd " << temp_reg << ", " << temp_reg << ", sp" << std::endl;
    fout << "\tlw " << temp_reg << ", " << stackVar.find_operand(operand) << "(sp)" << std::endl;
    fout << "\tadd " << temp_reg << ", " << temp_reg << ", " << offset << std::endl;

    fout << "\tlw " << reg << ", "
         << "0(" + temp_reg + ")" << std::endl;
    free_temp_reg(temp_reg);
}

void backend::Generator::store(ir::Operand operand, std::string reg, int offset)
{
    if (global_vals.count(operand.name))
    {
        auto temp_reg = get_temp_reg();
        fout << "\tla " << temp_reg << ", " << operand.name << std::endl;
        fout << "\tsw " << reg << ", " << offset << "(" + temp_reg + ")" << std::endl;
        free_temp_reg(temp_reg);
        return;
    }

    fout << "\tsw " << reg << ", " << stackVar.find_operand(operand) + offset << "(sp)" << std::endl;
}

void backend::Generator::store(ir::Operand operand, std::string reg, std::string offset)
{
    if (global_vals.count(operand.name))
    {
        auto temp_reg = get_temp_reg();
        fout << "\tla " << temp_reg << ", " << operand.name << std::endl;
        fout << "\tadd " << temp_reg << ", " << temp_reg << ", " << offset << std::endl;
        fout << "\tsw " << reg << ", "
             << "0(" + temp_reg + ")" << std::endl;
        free_temp_reg(temp_reg);
        return;
    }
    auto temp_reg = get_temp_reg();
    // fout << "\taddi " << temp_reg << ", " << offset << ", " << stackVar.find_operand(operand) << std::endl;
    // fout << "\tadd " << temp_reg << ", " << temp_reg << ", sp" << std::endl;
    fout << "\tlw " << temp_reg << ", " << stackVar.find_operand(operand) << "(sp)" << std::endl;
    fout << "\tadd " << temp_reg << ", " << temp_reg << ", " << offset << std::endl;

    fout << "\tsw " << reg << ", "
         << "0(" + temp_reg + ")" << std::endl;
    free_temp_reg(temp_reg);
}

std::map<std::string, bool> temporaies{{"t3", 0}, {"t4", 0}, {"t5", 0}, {"t6", 0}};

std::string backend::Generator::get_temp_reg()
{
    for (auto &reg : temporaies)
    {
        if (!reg.second)
        {
            reg.second = 1;
            return reg.first;
        }
    }
    assert(0 && "no temp reg");
}

void backend::Generator::free_temp_reg(std::string reg)
{
    temporaies[reg] = 0;
}

void backend::Generator::gen()
{

    // 处理全局变量（包括整型/浮点型变量/数组）
    // 汇编中包含数据段定义，用于存储变量和常量的内存分配
    this->fout << "\t.data\n";
    const ir::Function &global_func = this->program.functions.back(); // 语义分析中最后一个是最大的全局函数

    // 1.处理整型与浮点型变量声明
    for (const ir::Instruction *instr : global_func.InstVec)
    {
        auto instr_des = instr->des;
        auto instr_op1 = instr->op1;
        auto instr_op2 = instr->op2;
        auto instr_op = instr->op;
        if (instr_op == ir::Operator::def || instr_op == ir::Operator::fdef)
        {
            global_vals.insert(instr_des.name);
            this->fout << "\t.globl\t" << instr_des.name << "\n";
            this->fout << "\t.type\t" << instr_des.name << ", @object\n";
            this->fout << "\t.size\t" << instr_des.name << ", 4\n";
            this->fout << "\t.align\t" << "4\n";

            this->fout << instr_des.name << ":\n";
            // set_label(instr_des.name);
            assert(instr_op1.type != ir::Type::Int && instr_op1.type != ir::Type::Float);
            if (instr_des.type == ir::Type::Int)
            {
                assert(instr_op1.type == ir::Type::IntLiteral);
                this->fout << "\t.word\t" << instr_op1.name << "\n";
            }
        }
    }

    // 2.处理有初始值的数组
    std::unordered_map<std::string, ir::GlobalVal> global_array_vals;
    for (const ir::GlobalVal &array_globalval : this->program.globalVal)
    {
        if (array_globalval.val.type == ir::Type::IntPtr)
            global_array_vals.emplace(array_globalval.val.name, array_globalval);
    }
    for (const ir::Instruction *instr : global_func.InstVec)
    {
        auto instr_des = instr->des;
        auto instr_op1 = instr->op1;
        auto instr_op2 = instr->op2;
        auto instr_op = instr->op;
        if (instr_op == ir::Operator::store)
        {
            if (global_array_vals.count(instr_op1.name))
            {
                auto &array_globalval = global_array_vals.find(instr_op1.name)->second;
                int arr_len = array_globalval.maxlen;

                global_vals.insert(instr_op1.name);
                this->fout << "\t.globl\t" << instr_op1.name << "\n";
                this->fout << "\t.type\t" << instr_op1.name << ", @object\n";
                this->fout << "\t.size\t" << instr_op1.name << ", " << arr_len * 4 << "\n";
                this->fout << "\t.align\t" << "4\n";

                this->fout << instr_op1.name << ":\n";
                // set_label(instr_op1.name);
                global_array_vals.erase(instr_op1.name);
            }
            std::cout << "\n"
                      << instr_des.name << " " << "\n";
            assert(instr_des.type != ir::Type::Int && instr_des.type != ir::Type::Float);
            if (instr_des.type == ir::Type::IntLiteral)
            {
                this->fout << "\t.word\t" << instr_des.name << "\n";
            }
        }
    }

    // 烦烦烦
    //  3.将所有没有进行初始化的数组放入bss中
    if (global_array_vals.size() > 0)
    {
        // 汇编中包含未初始化的全局变量或静态变量
        this->fout << "\t.bss\n";
        for (const auto &array_globalval : global_array_vals)
        {
            global_vals.insert(array_globalval.first);
            this->fout << "\t.globl\t" << array_globalval.first << "\n";
            this->fout << "\t.type\t" << array_globalval.first << ", @object\n";
            this->fout << "\t.size\t" << array_globalval.first << ", " << array_globalval.second.maxlen * 4 << "\n";
            this->fout << "\t.align\t" << "4\n";

            this->fout << array_globalval.first << ":\n";
            // set_label(array_globalval.first);
            this->fout << "\t.space\t" << array_globalval.second.maxlen * 4 << "\n";
        }
    }

    // generate functions
    // program.functions.pop_back();
    for (auto &func : program.functions)
    {
        if (func.name == "global")
        {
            continue;
        }

        gen_func(func);
    }
    fout.close();
}

int get_frame_size(const ir::Function &func, backend::stackVarMap &stackVar)
{
    std::cout << std::endl;
    auto &params = func.ParameterList;
    auto &inst_vec = func.InstVec;
    int frame_size = 0;
    std::set<std::string> var_set;
    std::set<std::string> params_set;

    for (auto &param : params)
    {
        params_set.insert(param.name);
    }

    // local variable
    for (auto &ins : inst_vec)
    {
        if (var_set.count(ins->des.name) || params_set.count(ins->des.name))
            continue; // avoid duplicate (struct)

        if (ins->op == ir::Operator::alloc)
        {
            var_set.insert(ins->des.name);
            stackVar.add_operand(ins->des, std::stoi(ins->op1.name) * 4 + 4);
            // stackVar.add_operand(ins->des, 4);
            frame_size += std::stoi(ins->op1.name) * 4 + 4;
            std::cout << ins->des.name << " : " << stackVar._table[ins->des.name] << std::endl;
            continue;
        }
        if (ins->des.type != ir::Type::null && ins->des.type != ir::Type::IntLiteral)
        {
            var_set.insert(ins->des.name);
            stackVar.add_operand(ins->des, 4);
            frame_size += 4;
            std::cout << ins->des.name << " : " << stackVar._table[ins->des.name] << std::endl;
        }
    }

    // parameters
    for (int i = params.size() - 1; params.size() && i >= 0; i--)
    {
        auto &param = params[i];
        if (var_set.count(param.name))
            continue; // avoid duplicate (struct)
        var_set.insert(param.name);
        stackVar.add_operand(param, 4);
        frame_size += 4;
        std::cout << param.name << " : " << stackVar._table[param.name] << std::endl;
    }

    // special register
    frame_size += 4; // ra
    std::cout << "ra : " << frame_size - 4 << std::endl;
    return frame_size;
}

void backend::Generator::init_label(const ir::Function &func)
{
    label_map.clear();
    int index = 0;
    for (auto &ins : func.InstVec)
    {
        if (ins->op == ir::Operator::_goto)
        {
            int tidx = index + std::stoi(ins->des.name);
            // if(tidx >= func.InstVec.size())
            //     continue;
            label_map[tidx] = ".L" + std::to_string(label_cnt);
            label_cnt++;
        }
        index++;
    }
}

void backend::Generator::gen_func(const ir::Function &function)
{
    // header
    fout << std::endl;
    fout << "\t.text" << std::endl;
    fout << "\t.global " << function.name << std::endl;
    fout << "\t.type " << function.name << ", @function" << std::endl;
    fout << function.name << ":" << std::endl;
    // body
    // entry
    stackVar._table.clear();
    stackVar.s = 0;
    uint32_t frame_size = get_frame_size(function, stackVar);
    init_label(function);
    fout << "\taddi sp, sp, -" << frame_size << std::endl;
    fout << "\tsw ra, " << frame_size - 4 << "(sp)" << std::endl;
    // generate
    if (function.ParameterList.size() <= returnArgs.size())
    {
        for (size_t i = 0; i < function.ParameterList.size(); i++)
        {
            auto &param = function.ParameterList[i];
            auto temp_reg = get_temp_reg();
            fout << "\tmv " << temp_reg << ", a" << i << std::endl;
            fout << "\tsw " << temp_reg << ", " << stackVar.find_operand(param) << "(sp)" << std::endl;
            free_temp_reg(temp_reg);
        }
    }
    int index = 0;
    for (auto &ins : function.InstVec)
    {
        if (label_map.count(index))
        {
            fout << label_map[index] << ":" << std::endl;
        }
        gen_instr(*ins, index);
        index++;
        if (ins->op == ir::Operator::_return)
        {
            fout << "\tlw ra, " << frame_size - 4 << "(sp)" << std::endl;
            fout << "\taddi sp, sp, " << frame_size << std::endl;
            fout << "\tjr ra" << std::endl;
        }
    }
    while (label_map.count(index))
    {
        fout << label_map[index] << ":" << std::endl;
        index++;
    }
    // exit
    fout << "\tlw ra, " << frame_size - 4 << "(sp)" << std::endl;
    fout << "\taddi sp, sp, " << frame_size << std::endl;
    fout << "\tjr ra" << std::endl;
    // footer
    fout << "\t.size " << function.name << ", .-" << function.name << std::endl;
}

void backend::Generator::gen_instr(const ir::Instruction &instruction, int index)
{
    auto &op = instruction.op;
    auto &des = instruction.des;
    auto &op1 = instruction.op1;
    auto &op2 = instruction.op2;
    switch (op)
    {
    case ir::Operator::call: // call des, op1(arg1, arg2, ...) : des = op1(arg1, arg2, ...)
    {
        // global函数不执行，因为全局变量已经在.data和.bss段声明了
        if (op1.name == "global")
        {
            break;
        }
        fout << "# call" << std::endl;
        auto call_inst = dynamic_cast<const ir::CallInst *>(&instruction);
        // arguments
        auto arguments_size = call_inst->argumentList.size();
        if (arguments_size > 8)
        {
            for (size_t i = 0; i < arguments_size; ++i)
            {
                auto &arg = call_inst->argumentList[i];
                // auto &arg_reg = returnArgs[i];
                switch (arg.type)
                {
                case ir::Type::Int:
                {
                    auto reg = "t1";
                    load(arg, reg);
                    fout << "\tsw " << reg << ", -" << (i + 2) * 4 << "(sp)" << std::endl;
                    break;
                }
                case ir::Type::IntPtr:
                {
                    auto reg = "t1";
                    if (global_vals.count(arg.name))
                    {
                        fout << "\tla " << reg << ", " << arg.name << std::endl;
                    }
                    else
                    {
                        load(arg, reg);
                    }
                    fout << "\tsw " << reg << ", -" << (i + 2) * 4 << "(sp)" << std::endl;
                    break;
                }
                case ir::Type::IntLiteral:
                {
                    auto temp_reg = get_temp_reg();
                    // fout << "\tli " << arg_reg << ", " << arg.name << std::endl;
                    fout << "\tli " << temp_reg << ", " << arg.name << std::endl;
                    fout << "\tsw " << temp_reg << ", -" << (i + 2) * 4 << "(sp)" << std::endl;
                    free_temp_reg(temp_reg);
                    break;
                }
                default:
                    assert(0 && "wrong type");
                    break;
                }
            }
        }
        else
        {
            for (size_t i = 0; i < arguments_size; ++i)
            {
                auto &arg = call_inst->argumentList[i];
                auto &arg_reg = returnArgs[i];
                switch (arg.type)
                {
                case ir::Type::Int:
                {
                    auto reg = "t1";
                    load(arg, reg);
                    fout << "\tmv " << arg_reg << ", " << reg << std::endl;
                    // fout << "\tsw " << reg << ", -" << (i + 2) * 4 << "(sp)" << std::endl;
                    break;
                }
                case ir::Type::IntPtr:
                {
                    auto reg = "t1";
                    if (global_vals.count(arg.name))
                    {
                        fout << "\tla " << reg << ", " << arg.name << std::endl;
                    }
                    else
                    {
                        load(arg, reg);
                    }
                    fout << "\tmv " << arg_reg << ", " << reg << std::endl;
                    // fout << "\tsw " << reg << ", -" << (i + 2) * 4 << "(sp)" << std::endl;
                    break;
                }
                case ir::Type::IntLiteral:
                {
                    // auto temp_reg = get_temp_reg();
                    fout << "\tli " << arg_reg << ", " << arg.name << std::endl;
                    break;
                }
                default:
                    assert(0 && "wrong type");
                    break;
                }
            }
        }
        // call
        fout << "\tcall " << call_inst->op1.name << std::endl;
        // save return value
        switch (des.type)
        {
        case ir::Type::Int:
        case ir::Type::IntPtr:
        {
            auto rd = "t0";
            fout << "\tmv " << rd << ", "
                 << "a0" << std::endl;
            store(des, rd);
            break;
        }
        case ir::Type::null:
            break;
        default:
            assert(0 && "wrong type");
            break;
        }
        break;
    }
    case ir::Operator::_return: // return op1 : return op1
    {

        fout << "# return" << std::endl;
        switch (op1.type)
        {
        case ir::Type::Int:
        case ir::Type::IntPtr:
        {
            auto rs1 = "t1";
            load(op1, rs1);
            fout << "\tmv "
                 << "a0"
                 << ", " << rs1 << std::endl;
            break;
        }
        case ir::Type::IntLiteral:
            fout << "\tli "
                 << "a0"
                 << ", " << op1.name << std::endl;
            break;
        case ir::Type::null:
            break;
        default:
            assert(0 && "wrong type");
            break;
        }
        break;
    }
    case ir::Operator::def: // def des, op1 : des = op1;
    case ir::Operator::mov: // mov des, op1 : des = op1;
    {
        if (op == ir::Operator::def)
            fout << "# def" << std::endl;
        else
            fout << "# mov" << std::endl;

        auto rd = "t0";
        auto rs = "t1";

        switch (op1.type)
        {
        case ir::Type::Int:
        {
            load(op1, rs);
            fout << "\tmv " << rd << ", " << rs << std::endl;
            break;
        }
        case ir::Type::IntLiteral:
            fout << "\tli " << rd << ", " << op1.name << std::endl;
            break;
        default:
            assert(0 && "wrong type");
            break;
        }
        store(des, rd);
        break;
    }
    case ir::Operator::add: // add des, op1, op2 : des = op1 + op2
    {
        fout << "# add" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir ::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
            fout << "\tadd " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        }
        else if (op1.type == ir ::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            load(op2, rs1);
            fout << "\taddi " << rd << ", " << rs1 << ", " << op1.name << std::endl;
        }
        else
        {
            load(op1, rs1);
            fout << "\taddi " << rd << ", " << rs1 << ", " << op2.name << std::endl;
        }
        store(des, rd);
        break;
    }
    case ir::Operator::sub: // sub des, op1, op2 : des = op1 - op2
    {
        fout << "# sub" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir ::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
        }
        else if (op1.type == ir ::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            load(op2, rs2);
        }
        else
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        fout << "\tsub " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::mul: // mul des, op1, op2 : des = op1 * op2
    {
        fout << "# mul" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir ::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
        }
        else if (op1.type == ir ::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            load(op2, rs1);
            fout << "\tli " << rs2 << ", " << op1.name << std::endl;
        }
        else if (op1.type == ir ::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        else
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        fout << "\tmul " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::div: // div des, op1, op2 : des = op1 / op2
    {
        fout << "# div" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
        }
        else if (op1.type == ir::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            load(op2, rs1);
            fout << "\tli " << rs2 << ", " << op1.name << std::endl;
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        else
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        fout << "\tdiv " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::mod: // mod des, op1, op2 : des = op1 % op2
    {
        fout << "# mod" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
        }
        else if (op1.type == ir::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            load(op2, rs1);
            fout << "\tli " << rs2 << ", " << op1.name << std::endl;
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        else
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        fout << "\trem " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::eq: // eq des, op1, op2 : des = (op1 == op2)
    {
        fout << "# eq" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
        }
        else if (op1.type == ir::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            load(op2, rs2);
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        else
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        fout << "\txor " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        fout << "\tseqz " << rd << ", " << rd << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::neq:
    {
        fout << "# neq" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
        }
        else if (op1.type == ir::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            load(op2, rs2);
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        else
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        fout << "\txor " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        fout << "\tsnez " << rd << ", " << rd << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::_and: // IR中的_and指令不是按位与，而是逻辑与  if(3&&4)
    {
        fout << "# and" << std::endl;
        auto rd = "t0";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            auto rs1 = "t1";
            load(op1, rs1);
            auto rs2 = "t2";
            load(op2, rs2);

            fout << "\tsnez " << rs1 << ", " << rs1 << std::endl;
            fout << "\tsnez " << rs2 << ", " << rs2 << std::endl;
            fout << "\tand " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        }
        else if (op1.type == ir::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            auto rs1 = "t1";
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;

            auto rs2 = "t2";
            load(op2, rs2);

            fout << "\tsnez " << rs1 << ", " << rs1 << std::endl;
            fout << "\tsnez " << rs2 << ", " << rs2 << std::endl;
            fout << "\tand " << rd << ", " << rs1 << ", " << rs2 << std::endl;

            // fout << "\tandi " << rd << ", " << rs << ", " << op1.name << std::endl;
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            auto rs1 = "t1";
            load(op1, rs1);

            auto rs2 = "t2";
            fout << "\tli" << rs2 << ", " << op2.name << std::endl;

            fout << "\tsnez " << rs1 << ", " << rs1 << std::endl;
            fout << "\tsnez " << rs2 << ", " << rs2 << std::endl;
            fout << "\tand " << rd << ", " << rs1 << ", " << rs2 << std::endl;
            // fout << "\tandi " << rd << ", " << rs << ", " << op2.name << std::endl;
        }
        else
        {
            auto rs1 = "t1";
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            auto rs2 = "t2";
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;

            fout << "\tsnez " << rs1 << ", " << rs1 << std::endl;
            fout << "\tsnez " << rs2 << ", " << rs2 << std::endl;
            fout << "\tand " << rs1 << ", " << rs1 << ", " << rs2 << std::endl;
        }
        store(des, rd);
        break;
    }
    case ir::Operator::_or: //_or是逻辑或
    {
        fout << "# or" << std::endl;
        auto rd = "t0";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            auto rs1 = "t1";
            load(op1, rs1);
            auto rs2 = "t2";
            load(op2, rs2);
            fout << "\tor " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        }
        else if (op1.type == ir::Type::IntLiteral && op2.type == ir::Type::Int)
        {
            auto rs = "t1";
            load(op2, rs);
            fout << "\tori " << rd << ", " << rs << ", " << op1.name << std::endl;
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            auto rs = "t1";
            load(op1, rs);
            fout << "\tori " << rd << ", " << rs << ", " << op2.name << std::endl;
        }
        else
        {
            auto rs1 = "t1";
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            auto rs2 = "t2";
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
            fout << "\tor " << rs1 << ", " << rs1 << ", " << rs2 << std::endl;
        }
        store(des, rd);
        break;
    }
    case ir::Operator::_not:
    {
        fout << "# not" << std::endl;
        auto rd = "t0";
        auto rs = "t1";
        if (op1.type == ir::Type::Int)
        {
            load(op1, rs);
        }
        else if (op1.type == ir::Type::IntLiteral)
        {
            fout << "\tli " << rs << ", " << op1.name << std::endl;
        }
        else
        {
            assert(0);
        }
        fout << "\tseqz " << rd << ", " << rs << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::lss:
    {
        fout << "# lss" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
            fout << "\tslt " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
            fout << "\tslt " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        }
        else
        {
            assert(0);
        }
        store(des, rd);
        break;
    }
    case ir::Operator::leq: // if(a<=b)
    {
        fout << "# leq" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        else
        {
            assert(0);
        }
        fout << "\tslt " << rd << ", " << rs2 << ", " << rs1 << std::endl;
        fout << "\tseqz " << rd << ", " << rd << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::gtr:
    {
        fout << "# gtr" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tli " << rs2 << ", " << op2.name << std::endl;
        }
        else
        {
            assert(0);
        }
        fout << "\tslt " << rd << ", " << rs2 << ", " << rs1 << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::geq: // if(a>=b)
    {
        fout << "# geq" << std::endl;
        auto rd = "t0";
        auto rs1 = "t1";
        auto rs2 = "t2";
        if (op1.type == ir::Type::Int && op2.type == ir::Type::Int)
        {
            load(op1, rs1);
            load(op2, rs2);
            fout << "\tslt " << rd << ", " << rs1 << ", " << rs2 << std::endl;
        }
        else if (op1.type == ir::Type::Int && op2.type == ir::Type::IntLiteral)
        {
            load(op1, rs1);
            fout << "\tslti " << rd << ", " << rs1 << ", " << op2.name << std::endl;
        }
        else
        {
            assert(0);
        }
        fout << "\tseqz " << rd << ", " << rd << std::endl;
        store(des, rd);
        break;
    }
    case ir::Operator::_goto: // goto op1, des : if op1 goto des
    {
        fout << "# goto" << std::endl;
        auto rs1 = "t0";
        if (op1.type == ir::Type::null)
        {
            fout << "\tj " << label_map[index + std::stoi(des.name)] << std::endl;
        }
        else if (op1.type == ir::Type::Int)
        {
            load(op1, rs1);
            fout << "\tbnez " << rs1 << ", " << label_map[index + std::stoi(des.name)] << std::endl;
        }
        else if (op1.type == ir::Type::IntLiteral)
        {
            fout << "\tli " << rs1 << ", " << op1.name << std::endl;
            fout << "\tbnez " << rs1 << ", " << label_map[index + std::stoi(des.name)] << std::endl;
        }
        else
        {
            assert(0 && "wrong type");
        }
        break;
    }
    case ir::Operator::store: // store des, op1, op2 : op1[op2] = des
    {
        fout << "# store" << std::endl;
        auto value = "t0";

        if (op2.type == ir::Type::IntLiteral)
        {
            auto offset = "t2";
            fout << "\tli " << offset << ", " << op2.name << std::endl;
            // fout << "\taddi " << offset << ", " << offset << ", 1" << std::endl;
            fout << "\tslli " << offset << ", " << offset << ", 2" << std::endl;

            if (des.type == ir::Type::IntLiteral)
            {
                fout << "\tli " << value << ", " << des.name << std::endl;
            }
            else if (des.type == ir::Type::Int)
            {
                load(des, value);
            }
            else
            {
                assert(0 && "wrong type");
            }
            store(op1, value, offset);
            // store(op1, value, std::stoi(op2.name) * 4 );
        }
        else if (op2.type == ir::Type::Int)
        {
            auto offset = "t2";
            load(op2, offset);
            // fout << "\taddi " << offset << ", " << offset << ", 1" << std::endl;
            fout << "\tslli " << offset << ", " << offset << ", 2" << std::endl;
            if (des.type == ir::Type::IntLiteral)
            {
                fout << "\tli " << value << ", " << des.name << std::endl;
            }
            else if (des.type == ir::Type::Int)
            {
                load(des, value);
            }
            else
            {
                assert(0 && "wrong type");
            }
            store(op1, value, offset);
        }
        else
        {
            assert(0 && "wrong type");
        }
        break;
    }
    case ir::Operator::load: // load des, op1, op2 : des = op1[op2]
    {
        fout << "# load" << std::endl;
        auto rd = "t0";
        if (op2.type == ir::Type::IntLiteral)
        {
            auto offset = "t2";
            fout << "\tli " << offset << ", " << op2.name << std::endl;
            // fout << "\taddi " << offset << ", " << offset << ", 1" << std::endl;
            fout << "\tslli " << offset << ", " << offset << ", 2" << std::endl;
            load(op1, rd, offset);

            // load(op1, rd, std::stoi(op2.name) * 4 );
        }
        else if (op2.type == ir::Type::Int)
        {
            auto offset = "t2";
            load(op2, offset);
            // fout << "\taddi " << offset << ", " << offset << ", 1" << std::endl;
            fout << "\tslli " << offset << ", " << offset << ", 2" << std::endl;
            load(op1, rd, offset);
        }
        else
        {
            assert(0 && "wrong type");
        }

        store(des, rd);
        break;
    }
    case ir::Operator::alloc:
    {
        fout << "# alloc" << std::endl;
        fout << "\taddi " << "t0" << ", " << "sp" << ", " << stackVar.find_operand(des) + 4 << std::endl;
        store(des, "t0");
        break;
    }
    default:
        std::cout << "! op: " << ir::toString(op) << std::endl;
        // assert(0 && "wrong operator");
        break;
    }
}