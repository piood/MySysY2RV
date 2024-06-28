#pragma once
#include<iostream>
#include<cstdio>
#include<memory>
#include<unordered_map>


// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual std::string generate_Koopa_IR() const = 0;
  virtual int Calc() const = 0;
};

static int now = 0;

static std::unordered_map<uintptr_t, std::string> IR_reg; // 每个表达式对应的寄存器

static std::unordered_map<std::string, int> const_val; // 常量表 常量->值
static std::unordered_map<std::string, std::string> const_reg; // 常量表 常量->寄存器

static std::unordered_map<std::string, int> var_type; // 变量表 变量->类型 0:常量 1:变量

static std::unordered_map<std::string, std::string> f; // 嵌套 当前深度->上一深度

static int if_cnt = 0; // if调整指令计数，方便生成if label
static std::unordered_map<std::string, bool> be_end_bl; // 记录当前block是否已经结束，是否需要生成返回指令

static std::string depth_str = ""; // 当前深度的字符串形式
static std::string nowdepth_str = ""; // 当前深度的字符串形式

static std::string Get_Identifier(std::string ident){
    std::string tempdepth_str = depth_str; //获取当前深度
    while(var_type.find("COMPILER__" + ident+"_"+tempdepth_str) == var_type.end()){
        tempdepth_str = f[tempdepth_str];
    } //循环向上获取深度
    return "COMPILER__" + ident + "_" + tempdepth_str;
}

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;

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

  std::string generate_Koopa_IR() const override{
    std::string Koopa_IR = "";
    Koopa_IR+="fun ";
    Koopa_IR+="@"+ident+"(): ";
    Koopa_IR+= func_type->generate_Koopa_IR();
    Koopa_IR+="{\n";
    Koopa_IR += "%entry:\n";
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

  std::string generate_Koopa_IR() const override{
    return "i32 ";
  }

    int Calc() const override{
        return 0;
    }
};

class LValAST : public BaseAST{
public:
    std::string ident;

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        std::string Identifier = Get_Identifier(ident);
        if(var_type[Identifier] == 0){
            Koopa_IR += "  %"+std::to_string(now)+" = add "+"0, "+const_reg[Identifier]+"\n";
        }else{
            Koopa_IR += "  %"+std::to_string(now)+" = load @" + Identifier + "\n";
        }
        now++;
        return Koopa_IR;
    }

    int Calc() const override{
        std::string Identifier = Get_Identifier(ident);
        if(var_type.find(Identifier) != var_type.end()){
            return const_val[Identifier];
        }
        else{
            std::cout<<"undefine variable "<<ident<<std::endl;
        }
        return 0;
    }
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> if_blockitem;
  std::unique_ptr<BaseAST> else_blockitem;
  std::unique_ptr<BaseAST> if_blockitemlist;
  std::unique_ptr<BaseAST> else_blockitemlist;

  int type;

  std::string generate_Koopa_IR() const override{
    std::string Koopa_IR = "";
    if(type==1){
        Koopa_IR += exp->generate_Koopa_IR();
        auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
        IR_reg[exp_addr] = "%"+std::to_string(now-1);
        //Koopa_IR += lval->generate_Koopa_IR();
        int val = exp->Calc();
        LValAST* LVal_ptr = dynamic_cast<LValAST*>(lval.get());
        
        std::string ident = LVal_ptr->ident;
        std::string Identifier = Get_Identifier(ident);

        Koopa_IR += "  store " + IR_reg[exp_addr] + ", @" + Identifier + "\n";
        
    } else if(type==0) {
        if(be_end_bl[depth_str]) return Koopa_IR;
        Koopa_IR += exp->generate_Koopa_IR();
        auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
        IR_reg[exp_addr] = "%"+std::to_string(now-1);
        Koopa_IR += "  ret " + IR_reg[exp_addr] + "\n\n";
        be_end_bl[depth_str] = true;
    } else if(type==2){
        Koopa_IR += exp->generate_Koopa_IR();
    } else if(type==3){
        Koopa_IR += block->generate_Koopa_IR();
    } else if(type==4){
        Koopa_IR += "";
    } else if(type==5){
        Koopa_IR += "";
    } else if(type==6||type==7){ // if block
        if(be_end_bl[depth_str]) return Koopa_IR; // 如果当前block已经结束，不生成if
        if_cnt++;
        int now_if = if_cnt;

        Koopa_IR += exp->generate_Koopa_IR();

        Koopa_IR += "\tbr %" + std::to_string(now-1) + ", %then_" + std::to_string(now_if) + ", %end_" + std::to_string(now_if) + "\n\n";
        Koopa_IR += "%then_" + std::to_string(now_if) + ":\n";

        depth_str += "_1"+std::to_string(now_if);
        f[depth_str] = nowdepth_str;
        nowdepth_str = depth_str;

        std::cout<<"before depth: "<<depth_str<<std::endl;
        Koopa_IR += if_blockitemlist->generate_Koopa_IR(); // 生成if的block
        

        if(!be_end_bl[depth_str+"_0"]) Koopa_IR += "\tjump %end_" + std::to_string(now_if) + "\n\n";

        nowdepth_str = f[depth_str];
        depth_str = nowdepth_str;

        Koopa_IR += "%end_" + std::to_string(now_if) + ":\n";

    } else if(type==8||type==9||type==10||type==11){ // if else block
        if(be_end_bl[depth_str]) return Koopa_IR;

        if_cnt++;
        int now_if=if_cnt;
        Koopa_IR += exp->generate_Koopa_IR();
        Koopa_IR += "\tbr %" + std::to_string(now-1) + ", %then_" + std::to_string(now_if) + ", %else_" + std::to_string(now_if) + "\n\n";
        Koopa_IR += "%then_" + std::to_string(now_if) + ":\n";


        depth_str += "_1"+std::to_string(now_if);
        f[depth_str] = nowdepth_str;
        nowdepth_str = depth_str;
        Koopa_IR += if_blockitemlist->generate_Koopa_IR(); // 生成if的block


        // 此时从if的block中退出，需要用nowdepth+1查看之前if的block中有没有出现return，如果if的block没有结束，跳转到end
        if(!be_end_bl[depth_str+"_0"]) Koopa_IR += "\tjump %end_" + std::to_string(now_if) + "\n\n";

        nowdepth_str = f[depth_str];
        depth_str = nowdepth_str;

        Koopa_IR += "%else_" + std::to_string(now_if) + ":\n";

        depth_str += "_2"+std::to_string(now_if);
        f[depth_str] = nowdepth_str;
        nowdepth_str = depth_str;

        Koopa_IR += else_blockitemlist->generate_Koopa_IR();
        
        if(!be_end_bl[depth_str+"_0"]) Koopa_IR += "\tjump %end_" + std::to_string(now_if) + "\n\n";

        nowdepth_str = f[depth_str];
        depth_str = nowdepth_str;

        Koopa_IR += "%end_" + std::to_string(now_if) + ":\n";
    }
    return Koopa_IR;
  }

    int Calc() const override{
        return 0;
    }
};

class ExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> lorexp;

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

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(exp){
            Koopa_IR += exp->generate_Koopa_IR();
            auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
            IR_reg[exp_addr] = "%"+std::to_string(now-1);
        } else if(lval){
            Koopa_IR += lval->generate_Koopa_IR();
            auto lval_addr = reinterpret_cast<uintptr_t>(lval.get());
            IR_reg[lval_addr] = "%"+std::to_string(now-1);
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


    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(initval){
            std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
            Koopa_IR += "  @" + Identifier + " = alloc i32\n";
            var_type[Identifier] = 1;
            const_val[Identifier] = initval->Calc();
            Koopa_IR += initval->generate_Koopa_IR();
            Koopa_IR += "  store %" + std::to_string(now-1) + ", @" + Identifier + "\n";
        }
        else{
            std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
            Koopa_IR += "  @" + Identifier + " = alloc i32\n";
            var_type[Identifier] = 1;
            const_val[Identifier] = 0;
        }
        return Koopa_IR;
    }

    int Calc() const override{
        return initval->Calc();
    }
};

class InitValAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;

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

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        //Koopa_IR += constinitval->generate_Koopa_IR();
        int val = this->Calc();
        auto constinitval_addr = reinterpret_cast<uintptr_t>(constinitval.get());
        IR_reg[constinitval_addr] = std::to_string(val);
        std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
        const_reg[Identifier] = IR_reg[constinitval_addr]; // 常量放入对应寄存器
        //IR_reg[constinitval_addr] = "%"+std::to_string(now);

        /*
        std::string Identifier = "COMPILER__" + ident + "_"+ std::to_string(nowdepth);

        const_reg[Identifier] = IR_reg[constinitval_addr]; // 常量放入对应寄存器
        Koopa_IR += "  "+ IR_reg[constinitval_addr] + " = add 0, " + std::to_string(val) + "\n";
        now++;
        */
        return Koopa_IR;
    }

    int Calc() const override{
        std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
        const_val[Identifier] =  constinitval->Calc();
        var_type[Identifier] = 0; //常量
        return const_val[Identifier];
    }
};

class ConstInitValAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> constexp;

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
    int type;

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(type==0){
            Koopa_IR += blockitemlist->generate_Koopa_IR();  // 生成block的IR
        }else if(type==1){
            Koopa_IR += "";
        }
        return Koopa_IR;
    }

    int Calc() const override{
        if(type==0)
            return blockitemlist->Calc();
        else return 0;
    }
};

class BlockItemListAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> blockitem;
    std::unique_ptr<BaseAST> blockitemlist;

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

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        if(be_end_bl[depth_str]){
            return "";
        }
        depth_str += "_0";            // 进入block，深度加1q
        f[depth_str]=nowdepth_str;   // 新的block的父节点是之前的block
        nowdepth_str=depth_str;      // nowdepth更新为当前深度
        if(decl){
            Koopa_IR += decl->generate_Koopa_IR();
        }else if(stmt){
            Koopa_IR += stmt->generate_Koopa_IR();
        }
        nowdepth_str=f[nowdepth_str];   // 离开block前，恢复当前深度depth为父节点的深度
        depth_str = nowdepth_str;       // 恢复当前深度
        return Koopa_IR;
    }

    int Calc() const override{
        return 0;
    }
};


class ConstExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;

    std::string generate_Koopa_IR() const override{
        std::string Koopa_IR = "";
        Koopa_IR += exp->generate_Koopa_IR();
        return Koopa_IR;
    }

    int Calc() const override{
        return exp->Calc();
    }
};