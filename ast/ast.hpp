#pragma once
#include<iostream>
#include<cstdio>
#include<memory>
#include<unordered_map>

static int now = 0;

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual std::string generate_Koopa_IR() const = 0;
  virtual int Calc() const = 0;
};

static std::unordered_map<uintptr_t, std::string> IR_reg; // 每个表达式对应的寄存器

static std::unordered_map<std::string, int> const_val; // 常量表 常量->值
static std::unordered_map<std::string, std::string> const_reg; // 常量表 常量->寄存器

static std::unordered_map<std::string, int> var_type; 

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override{
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout<<" }";
  }

  std::string generate_Koopa_IR() const override{
    return func_def->generate_Koopa_IR();
  } 

  int Calc() const override{
    return func_def->Calc();
  }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override{
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout<<" }\n";
  }

  std::string generate_Koopa_IR() const override{
    std::string Koopa_IR = "";
    Koopa_IR+="fun ";
    Koopa_IR+="@"+ident+"(): ";
    Koopa_IR+= func_type->generate_Koopa_IR();
    Koopa_IR+="{\n";
    Koopa_IR+= block->generate_Koopa_IR();
    Koopa_IR+="}\n";
    return Koopa_IR;
  }
  int Calc() const override{
    return 0;
  }
};

class FuncTypeAST : public BaseAST {
 public:
  std::string INT;

  void Dump() const override{
    std::cout<<"FuncTypeAST { ";
    std::cout<<INT;
    std::cout<<" }";
  }

  std::string generate_Koopa_IR() const override{
    return "i32 ";
  }

    int Calc() const override{
        return 0;
    }
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> lval;

  void Dump() const override{
    if(lval){
        std::cout << "StmtAST { ";
        lval->Dump();
        exp->Dump();
        std::cout<<" }";
    } 
    else {
        std::cout<< "StmtAST { ";
        exp->Dump();
        std::cout<<" }";
    }
  } 

  std::string generate_Koopa_IR() const override{
    std::string Koopa_IR = "";
    if(lval){
        Koopa_IR += lval->generate_Koopa_IR();
        auto lval_addr = reinterpret_cast<uintptr_t>(lval.get());
        IR_reg[lval_addr] = "%"+std::to_string(now-1);
        Koopa_IR += exp->generate_Koopa_IR();
        auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
        Koopa_IR += "  store " + IR_reg[exp_addr] + ", " + IR_reg[lval_addr] + "\n";
    } else {
        Koopa_IR += exp->generate_Koopa_IR();
        auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
        IR_reg[exp_addr] = "%"+std::to_string(now-1);
        Koopa_IR += "  ret " + IR_reg[exp_addr] + "\n";
    }
    return Koopa_IR;
  }

    int Calc() const override{
        return exp->Calc();
    }
};

class ExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> lorexp;
    void Dump() const override{
        std::cout << "ExpAST { ";
        lorexp->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        Koopa_IR += lorexp->generate_Koopa_IR();
        IR_reg[reinterpret_cast<uintptr_t>(lorexp.get())] = "%"+std::to_string(now-1);
        return Koopa_IR;
    }

    int Calc() const override{
        return lorexp->Calc();
    }
};

class LOrExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> lorexp;
    std::unique_ptr<BaseAST> landexp;
    std::string lorexpop;

    void Dump() const override{
        if(lorexp){
            std::cout << "LorexpAST { ";
            lorexp->Dump();
            std::cout<<" "<<lorexpop<<" ";
            landexp->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "LorexpAST { ";
            landexp->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(lorexp){
            Koopa_IR += lorexp->generate_Koopa_IR();
            IR_reg[reinterpret_cast<uintptr_t>(lorexp.get())] = "%"+std::to_string(now-1);
            Koopa_IR += landexp->generate_Koopa_IR();
            IR_reg[reinterpret_cast<uintptr_t>(landexp.get())] = "%"+std::to_string(now-1);
            if(lorexpop == "||"){
                Koopa_IR += "  %" + std::to_string(now) + " = or " + IR_reg[reinterpret_cast<uintptr_t>(lorexp.get())] + ", " + IR_reg[reinterpret_cast<uintptr_t>(landexp.get())] + "\n" ;
                Koopa_IR += "  %" + std::to_string(now+1) + " = ne " + "%"+std::to_string(now) + ", 0" + "\n";
                now+=2;
            }
        }
        else{
            Koopa_IR +=  landexp->generate_Koopa_IR();
            IR_reg[reinterpret_cast<uintptr_t>(landexp.get())] = "%"+std::to_string(now-1);
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(lorexp){
            if(lorexpop == "||"){
                return lorexp->Calc() || landexp->Calc();
            }
        }
        return landexp->Calc();
    }
};

class LAndExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> landexp;
    std::unique_ptr<BaseAST> eqexp;
    std::string landexpop;

    void Dump() const override{
        if(landexp){
            std::cout << "LandExpAST { ";
            landexp->Dump();
            std::cout<<" "<<landexpop<<" ";
            eqexp->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "LandExpAST { ";
            eqexp->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(landexp){
            Koopa_IR += landexp->generate_Koopa_IR();
            auto landexp_addr = reinterpret_cast<uintptr_t>(landexp.get());
            IR_reg[landexp_addr] = "%"+std::to_string(now-1);
            Koopa_IR += eqexp->generate_Koopa_IR();
            auto eqexp_addr = reinterpret_cast<uintptr_t>(eqexp.get());
            IR_reg[eqexp_addr] = "%"+std::to_string(now-1);
            if(landexpop == "&&"){
                Koopa_IR += "  %" + std::to_string(now++) + " = ne " + IR_reg[landexp_addr] + ", 0" + "\n";
                Koopa_IR += "  %" + std::to_string(now++) + " = ne " + IR_reg[eqexp_addr] + ", 0" + "\n";
                Koopa_IR += "  %" + std::to_string(now) + " = and " + "%"+std::to_string(now-1) + ", " + "%"+std::to_string(now-2) + "\n";
                now++;
            }
        }
        else{
            Koopa_IR +=  eqexp->generate_Koopa_IR();
            IR_reg[reinterpret_cast<uintptr_t>(eqexp.get())] = "%"+std::to_string(now-1);
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(landexp){
            if(landexpop == "&&"){
                return landexp->Calc() && eqexp->Calc();
            }
        }
        return eqexp->Calc();
    }
};

class EqExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> eqexp;
    std::unique_ptr<BaseAST> relexp;
    std::string eqexpop;

    void Dump() const override{
        if(eqexp){
            std::cout << "EqExpAST { ";
            eqexp->Dump();
            std::cout<<" "<<eqexpop<<" ";
            relexp->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "EqExpAST { ";
            relexp->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(eqexp){
            Koopa_IR = eqexp->generate_Koopa_IR();
            auto eqexp_addr = reinterpret_cast<uintptr_t>(eqexp.get());
            IR_reg[eqexp_addr] = "%"+std::to_string(now-1);

            auto relexp_addr = reinterpret_cast<uintptr_t>(relexp.get());
            Koopa_IR += relexp->generate_Koopa_IR();
            IR_reg[relexp_addr] = "%"+std::to_string(now-1);
            if(eqexpop == "=="){
                Koopa_IR += "  %" + std::to_string(now) + " = eq " + IR_reg[eqexp_addr] + ", " + IR_reg[relexp_addr] + "\n";
                now++;
            }else if(eqexpop == "!="){
                Koopa_IR += "  %" + std::to_string(now) + " = ne " + IR_reg[eqexp_addr] + ", " + IR_reg[relexp_addr] + "\n";
                now++;
            }
        }
        else{
            Koopa_IR +=  relexp->generate_Koopa_IR();
            auto relexp_addr = reinterpret_cast<uintptr_t>(relexp.get());
            IR_reg[relexp_addr] = "%"+std::to_string(now-1);
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(eqexp){
            if(eqexpop == "=="){
                return eqexp->Calc() == relexp->Calc();
            }else if(eqexpop == "!="){
                return eqexp->Calc() != relexp->Calc();
            }
        }
        return relexp->Calc();
    }
};

class RelExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> relexp;
    std::unique_ptr<BaseAST> addexp;
    std::string relexpop;

    void Dump() const override{
        if(relexp){
            std::cout << "RelExpAST { ";
            relexp->Dump();
            std::cout<<" "<<relexpop<<" ";
            addexp->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "RelExpAST { ";
            addexp->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(relexp){
            Koopa_IR += relexp->generate_Koopa_IR();
            auto relexp_addr = reinterpret_cast<uintptr_t>(relexp.get());
            IR_reg[relexp_addr] = "%"+std::to_string(now-1);


            Koopa_IR += addexp->generate_Koopa_IR();
            auto addexp_addr = reinterpret_cast<uintptr_t>(addexp.get());
            IR_reg[addexp_addr] = "%"+std::to_string(now-1);
            if(relexpop == "<"){
                Koopa_IR += "  %" + std::to_string(now) + " = lt " + IR_reg[relexp_addr] + ", " + IR_reg[addexp_addr] + "\n";
                now++;
            }else if(relexpop == "<="){
                Koopa_IR += "  %" + std::to_string(now) + " = le " + IR_reg[relexp_addr] + ", " + IR_reg[addexp_addr] + "\n";
                now++;
            }else if(relexpop == ">"){
                Koopa_IR += "  %" + std::to_string(now) + " = gt " + IR_reg[relexp_addr] + ", " + IR_reg[addexp_addr] + "\n";
                now++;
            }else if(relexpop == ">="){
                Koopa_IR += "  %" + std::to_string(now) + " = ge " + IR_reg[relexp_addr] + ", " + IR_reg[addexp_addr] + "\n";
                now++;
            }
        }
        else{
            Koopa_IR +=  addexp->generate_Koopa_IR();
            auto addexp_addr = reinterpret_cast<uintptr_t>(addexp.get());
            IR_reg[addexp_addr] = "%"+std::to_string(now-1);
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(relexp){
            if(relexpop == "<"){
                return relexp->Calc() < addexp->Calc();
            }else if(relexpop == "<="){
                return relexp->Calc() <= addexp->Calc();
            }else if(relexpop == ">"){
                return relexp->Calc() > addexp->Calc();
            }else if(relexpop == ">="){
                return relexp->Calc() >= addexp->Calc();
            }
        }
        return addexp->Calc();
    }
};

class AddExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> addexp;
    std::unique_ptr<BaseAST> mulexp;
    std::string addexpop;

    void Dump() const override{
        if(addexp){
            std::cout << "AddExpAST { ";
            mulexp->Dump();
            std::cout<<" "<<addexpop<<" ";
            addexp->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "AddExpAST { ";
            mulexp->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(addexp){
            Koopa_IR += mulexp->generate_Koopa_IR();
            auto mulexp_addr = reinterpret_cast<uintptr_t>(mulexp.get());
            IR_reg[mulexp_addr] = "%"+std::to_string(now-1);

            Koopa_IR += addexp->generate_Koopa_IR();
            auto addexp_addr = reinterpret_cast<uintptr_t>(addexp.get());
            IR_reg[addexp_addr] = "%"+std::to_string(now-1);

            if(addexpop == "+"){
                Koopa_IR += "  %" + std::to_string(now) + " = add " + IR_reg[addexp_addr] + ", " + IR_reg[mulexp_addr] + "\n";
                now++;
            }else if(addexpop == "-"){
                Koopa_IR += "  %" + std::to_string(now) + " = sub " + IR_reg[addexp_addr] + ", " + IR_reg[mulexp_addr] + "\n";
                now++;
            }
        }
        else{
            Koopa_IR +=  mulexp->generate_Koopa_IR();
            auto mulexp_addr = reinterpret_cast<uintptr_t>(mulexp.get());
            IR_reg[mulexp_addr] = "%"+std::to_string(now-1);
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(addexp){
            if(addexpop == "+"){
                return addexp->Calc() + mulexp->Calc();
            }else if(addexpop == "-"){
                return addexp->Calc() - mulexp->Calc();
            }
        }
        return mulexp->Calc();
    }
};

class MulExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> mulexp;
    std::unique_ptr<BaseAST> unaryexp;
    std::string mulexpop;

    void Dump() const override{
        if(mulexp){
            std::cout << "MulExpAST { ";
            mulexp->Dump();
            std::cout<<" "<<mulexpop<<" ";
            unaryexp->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "MulExpAST { ";
            unaryexp->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(mulexp){
            Koopa_IR += mulexp->generate_Koopa_IR();
            auto mulexp_addr = reinterpret_cast<uintptr_t>(mulexp.get());
            IR_reg[mulexp_addr] = "%"+std::to_string(now-1);

            Koopa_IR += unaryexp->generate_Koopa_IR();
            auto unaryexp_addr = reinterpret_cast<uintptr_t>(unaryexp.get());
            IR_reg[unaryexp_addr] = "%"+std::to_string(now-1);
            if(mulexpop == "*"){
                Koopa_IR +="  %" + std::to_string(now) + " = mul " + IR_reg[mulexp_addr] + ", " + IR_reg[unaryexp_addr] + "\n";
                now++;
            }else if(mulexpop == "/"){
                Koopa_IR += "  %" + std::to_string(now) + " = div " + IR_reg[mulexp_addr] + ", " + IR_reg[unaryexp_addr] + "\n";
                now++;
            }
            else if(mulexpop == "%"){
                Koopa_IR += "  %" + std::to_string(now) + " = mod " + IR_reg[mulexp_addr] + ", " + IR_reg[unaryexp_addr] + "\n";
                now++;
            }
        }
        else{
            Koopa_IR +=  unaryexp->generate_Koopa_IR();
            auto unaryexp_addr = reinterpret_cast<uintptr_t>(unaryexp.get());
            IR_reg[unaryexp_addr] = "%"+std::to_string(now-1);
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(mulexp){
            if(mulexpop == "*"){
                return mulexp->Calc() * unaryexp->Calc();
            }else if(mulexpop == "/"){
                return mulexp->Calc() / unaryexp->Calc();
            }else if(mulexpop == "%"){
                return mulexp->Calc() % unaryexp->Calc();
            }
        }
        return unaryexp->Calc();
    }
};


class UnaryExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> primaryexp;
    std::string unaryop;
    std::unique_ptr<BaseAST> unaryexp;

    void Dump() const override{
        if(unaryexp){
            std::cout << "UnaryExpAST { "<< unaryop <<" ";
            unaryexp->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "UnaryExpAST { ";
            primaryexp->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(unaryexp){
            Koopa_IR += unaryexp->generate_Koopa_IR();
            auto unaryexp_addr = reinterpret_cast<uintptr_t>(unaryexp.get());
            IR_reg[unaryexp_addr] = "%"+std::to_string(now-1);
            if(unaryop == "-"){
                Koopa_IR += "  %" + std::to_string(now) + " = sub 0, " + IR_reg[unaryexp_addr] +" \n";
                now++;
            }else if(unaryop == "!"){
                Koopa_IR += "  %" + std::to_string(now) + " = eq 0, " + IR_reg[unaryexp_addr] +" \n";
                now++;
            }else if(unaryop == "+"){
                int x = 1;
            }
        }
        else{
            Koopa_IR += primaryexp->generate_Koopa_IR();
            auto primaryexp_addr = reinterpret_cast<uintptr_t>(primaryexp.get());
            IR_reg[primaryexp_addr] = "%"+std::to_string(now-1);
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(unaryexp){
            if(unaryop == "-"){
                return -unaryexp->Calc();
            }else if(unaryop == "!"){
                return !unaryexp->Calc();
            }
        }
        return primaryexp->Calc();
    }
};

class PrimaryExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;
    int number;

    void Dump() const override{
        if(exp){
            std::cout << "PrimaryExpAST { ( ";
            exp->Dump();
            std::cout<<" ) }";
        } else if(lval){
            std::cout << "PrimaryExpAST { ";
            lval->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "PrimaryExpAST { ";
            std::cout<<number;
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(exp){
            Koopa_IR += exp->generate_Koopa_IR();
            auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
            IR_reg[exp_addr] = "%"+std::to_string(now-1);
        } else if(lval){
            std::string reg = lval->generate_Koopa_IR();
            auto lval_addr = reinterpret_cast<uintptr_t>(lval.get());
            Koopa_IR += "  %" + std::to_string(now) + " = add 0, " + reg + "\n";
            IR_reg[lval_addr] = "%"+std::to_string(now);
            now++;
        } else {
            Koopa_IR = "  %" + std::to_string(now) + " = add 0, " + std::to_string(number) +" \n";
            now++;
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(exp){
            return exp->Calc();
        } else if(lval){
            return lval->Calc();
        } else {
            return number;
        }
    }
};

class DeclAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> constdecl;
    std::unique_ptr<BaseAST> vardecl;

    void Dump() const override{
        std::cout << "DeclAST { ";
        if(constdecl){
            constdecl->Dump();
        }else{
            vardecl->Dump();
        }
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(constdecl){
            Koopa_IR += constdecl->generate_Koopa_IR();
        }else{
            Koopa_IR += vardecl->generate_Koopa_IR();
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(constdecl){
            return constdecl->Calc();
        }else{
            return vardecl->Calc();
        }
    }
};

class VarDeclAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> vardeflist;

    void Dump() const override{
        std::cout << "VarDeclAST { ";
        btype->Dump();
        std::cout<<", ";
        vardeflist->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        Koopa_IR += btype->generate_Koopa_IR();
        Koopa_IR += vardeflist->generate_Koopa_IR();
        return Koopa_IR;
    }

    int Calc() const override{
        return vardeflist->Calc();
    }
};

class VarDefListAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> vardef;
    std::unique_ptr<BaseAST> vardeflist;

    void Dump() const override{
        if(vardeflist){
            std::cout << "VarDefListAST { ";
            vardef->Dump();
            std::cout<<", ";
            vardeflist->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "VarDefListAST { ";
            vardef->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(vardeflist){
            Koopa_IR += vardeflist->generate_Koopa_IR();
            Koopa_IR += vardef->generate_Koopa_IR();
        }else{
            Koopa_IR += vardef->generate_Koopa_IR();
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(vardeflist){
            //return vardef->Calc() + vardeflist->Calc();
            return 0;
        }else{
            return vardef->Calc();
        }
    }
};

class VarDefAST : public BaseAST{
public:
    std::string ident;
    std::unique_ptr<BaseAST> initval;

    void Dump() const override{
        std::cout << "VarDefAST { ";
        std::cout<<ident;
        std::cout<<", ";
        initval->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        Koopa_IR += initval->generate_Koopa_IR();
        auto exp_addr = reinterpret_cast<uintptr_t>(initval.get());
        IR_reg[exp_addr] = "%"+std::to_string(now-1);
        return Koopa_IR;
    }

    int Calc() const override{
        return initval->Calc();
    }
};

class InitValAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override{
        std::cout << "InitValAST { ";
        exp->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        Koopa_IR += exp->generate_Koopa_IR();
        return Koopa_IR;
    }

    int Calc() const override{
        return exp->Calc();
    }
};

class ConstDeclAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> constdeflist;

    void Dump() const override{
        std::cout << "ConstDeclAST { ";
        btype->Dump();
        std::cout<<", ";
        constdeflist->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        Koopa_IR += btype->generate_Koopa_IR();
        Koopa_IR += constdeflist->generate_Koopa_IR();
        return Koopa_IR;
    }

    int Calc() const override{
        return constdeflist->Calc();
    }
};

class BTypeAST : public BaseAST{
public:
    std::string type;

    void Dump() const override{
        std::cout<<"BTypeAST { ";
        std::cout<<type;
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        return "";
    }

    int Calc() const override{
        return 0;
    }
};

class ConstDefListAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> constdef;
    std::unique_ptr<BaseAST> constdeflist;

    void Dump() const override{
        if(constdeflist){
            std::cout << "ConstDefListAST { ";
            constdef->Dump();
            std::cout<<", ";
            constdeflist->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "ConstDefListAST { ";
            constdef->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(constdeflist){
            Koopa_IR += constdeflist->generate_Koopa_IR();
            Koopa_IR += constdef->generate_Koopa_IR();
        }else{
            Koopa_IR += constdef->generate_Koopa_IR();
        }
        return Koopa_IR;
    }

    int Calc() const override{
        return constdef->Calc();
    }
};

class ConstDefAST : public BaseAST{
public:
    std::string ident;
    std::unique_ptr<BaseAST> constinitval;

    void Dump() const override{
        std::cout << "ConstDefAST { ";
        std::cout<<ident;
        std::cout<<", ";
        constinitval->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        //Koopa_IR += constinitval->generate_Koopa_IR();
        int val = this->Calc();
        auto constinitval_addr = reinterpret_cast<uintptr_t>(constinitval.get());
        IR_reg[constinitval_addr] = "%"+std::to_string(now);
        const_reg[ident] = IR_reg[constinitval_addr]; // 常量放入对应寄存器
        Koopa_IR += "  "+ IR_reg[constinitval_addr] + " = add 0, " + std::to_string(val) + "\n";
        now++;
        return Koopa_IR;
    }

    int Calc() const override{
        const_val[ident] =  constinitval->Calc();
        return const_val[ident];
    }
};

class ConstInitValAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> constexp;

    void Dump() const override{
        std::cout << "ConstInitValAST { ";
        constexp->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        Koopa_IR += constexp->generate_Koopa_IR();
        return Koopa_IR;
    }

    int Calc() const override{
        return constexp->Calc();
    }
};

class BlockAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> blockitemlist;

    void Dump() const override{
        std::cout << "BlockAST { ";
        blockitemlist->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "%entry:\n";
        Koopa_IR += blockitemlist->generate_Koopa_IR();
        return Koopa_IR;
    }

    int Calc() const override{
        return blockitemlist->Calc();
    }
};

class BlockItemListAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> blockitem;
    std::unique_ptr<BaseAST> blockitemlist;

    void Dump() const override{
        if(blockitemlist){
            std::cout << "BlockItemListAST { ";
            blockitem->Dump();
            std::cout<<", ";
            blockitemlist->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "BlockItemListAST { ";
            blockitem->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(blockitemlist){
            Koopa_IR += blockitemlist->generate_Koopa_IR();
            Koopa_IR += blockitem->generate_Koopa_IR();
        }else{
            Koopa_IR += blockitem->generate_Koopa_IR();
        }
        return Koopa_IR;
    }

    int Calc() const override{
        return blockitem->Calc();
    }
};

class BlockItemAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> decl;
    std::unique_ptr<BaseAST> stmt;

    void Dump() const override{
        if(decl){
            std::cout << "BlockItemAST { ";
            decl->Dump();
            std::cout<<" }";
        } else {
            std::cout<< "BlockItemAST { ";
            stmt->Dump();
            std::cout<<" }";
        }
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(decl){
            Koopa_IR += decl->generate_Koopa_IR();
        }else{
            Koopa_IR += stmt->generate_Koopa_IR();
        }
        return Koopa_IR;
    }

    int Calc() const override{
        return 0;
    }
};

class LValAST : public BaseAST{
public:
    std::string ident;

    void Dump() const override{
        std::cout<<"LValAST { ";
        std::cout<<ident;
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        if(const_val.find(ident) != const_val.end()){
            int val = const_val[ident];
            std::string reg = const_reg[ident];
            return reg;
        }else{
            std::cout<<"undefine variable "<<ident<<std::endl;
        }
        return "";
    }

    int Calc() const override{
        if(const_val.find(ident) != const_val.end()){
            return const_val[ident];
        }else{
            std::cout<<"undefine variable "<<ident<<std::endl;
        }
        return 0;
    }
};


class ConstExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;

    void Dump() const override{
        std::cout << "ConstExpAST { ";
        exp->Dump();
        std::cout<<" }";
    }

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        Koopa_IR += exp->generate_Koopa_IR();
        return Koopa_IR;
    }

    int Calc() const override{
        return exp->Calc();
    }
};