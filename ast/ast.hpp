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
};

static std::unordered_map<uintptr_t, std::string> IR_reg;

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
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override{
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout<<" }";
  }

  std::string generate_Koopa_IR() const override{
    std::string Koopa_IR="";
    Koopa_IR+="%entry:\n";
    Koopa_IR+=stmt->generate_Koopa_IR();
    return Koopa_IR;
  }
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override{
    std::cout << "StmtAST { return ";
    exp->Dump();
    std::cout<<" }";
  } 

  std::string generate_Koopa_IR() const override{
    std::string Koopa_IR = exp->generate_Koopa_IR();
    IR_reg[reinterpret_cast<uintptr_t>(exp.get())] = "%"+std::to_string(now-1);
    std::string reg = IR_reg[reinterpret_cast<uintptr_t>(exp.get())];
    Koopa_IR += "  ret ";
    Koopa_IR += reg + "\n";
    return Koopa_IR;
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
};

class PrimaryExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;
    int number;

    void Dump() const override{
        if(exp){
            std::cout << "PrimaryExpAST { ( ";
            exp->Dump();
            std::cout<<" ) }";
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
        } else {
            Koopa_IR = "  %" + std::to_string(now) + " = add 0, " + std::to_string(number) +" \n";
            now++;
        }
        return Koopa_IR;
    }
};