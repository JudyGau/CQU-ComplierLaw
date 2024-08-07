#include "backend/rv_def.h"
#include <cassert>

std::string rv::toString(rv::rvREG r)
{
    switch (r)
    {
    case rv::rvREG::X5:
        return "t0";
    case rv::rvREG::X6:
        return "t1";
    case rv::rvREG::X7:
        return "t2";
    case rv::rvREG::X10:
        return "a0";
    case rv::rvREG::X11:
        return "a1";
    case rv::rvREG::X12:
        return "a2";
    case rv::rvREG::X13:
        return "a3";
    case rv::rvREG::X14:
        return "a4";
    case rv::rvREG::X15:
        return "a5";
    case rv::rvREG::X16:
        return "a6";
    case rv::rvREG::X17:
        return "a7";
    case rv::rvREG::X28:
        return "t3";
    case rv::rvREG::X29:
        return "t4";
    case rv::rvREG::X2:
        return "sp";
    default:
        assert(0 && "invalid rvREG");
        break;
    }
}

std::string rv::toString(rvOPCODE r)
{
    switch (r)
    {
    case rvOPCODE::ADD:
        return "add";
        break;
    case rvOPCODE::SUB:
        return "sub";
        break;
    case rvOPCODE::MUL:
        return "mul";
        break;
    case rvOPCODE::DIV:
        return "div";
        break;

    default:
        break;
    }
}