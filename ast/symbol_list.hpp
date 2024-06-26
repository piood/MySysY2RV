#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include<cassert>
#include "koopa.h"

class BaseAST;

struct LValSymbol
{
    enum SymbolType
    {
        Const,
        Var,
        Array,
        Pointer,
        Function
    } type;
    void *number;
    LValSymbol() {}
    LValSymbol(SymbolType _type, void * _number) : type(_type), number(_number) {}
};

class SymbolList
{
    std::vector<std::map<std::string, LValSymbol>> sym_stk;

public:
    void NewEnv()
    {
        sym_stk.push_back(std::map<std::string, LValSymbol>());
    }
    void AddSymbol(const std::string &name, LValSymbol koopa_item)
    {
        auto &list = sym_stk[sym_stk.size() - 1];
        assert(list.count(name) == 0);
        list[name] = koopa_item;
    }
    LValSymbol GetSymbol(const std::string &name)
    {
        LValSymbol res;
        for (size_t i = sym_stk.size() - 1; i >= 0; i--)
            if (sym_stk[i].count(name) != 0)
            {
                res = sym_stk[i][name];
                break;
            }
        return res;
    }
    void DeleteEnv()
    {
        sym_stk.pop_back();
    }
};