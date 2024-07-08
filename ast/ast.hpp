#pragma once
#include<iostream>
#include<cstdio>
#include<memory>
#include<unordered_map>
#include<unordered_set>
#include<vector>


// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void generate_Koopa_IR() const = 0;
  virtual int Calc() const = 0;
};

static int now = 0;

static std::unordered_map<uintptr_t, std::string> IR_reg; // 每个表达式对应的寄存器

static std::unordered_map<std::string, int> const_val; // 常量表 常量->值
static std::unordered_map<std::string, std::string> const_reg; // 常量表 常量->寄存器

static std::unordered_map<std::string, int> var_type; // 变量表 变量->类型 0:常量 1:变量

static std::unordered_map<std::string, std::string> f; // 嵌套 当前深度->上一深度

static int if_cnt = 0; // if调整指令计数，方便生成if label

static int while_cnt = 0, while_now = 0; // while调整指令计数，方便生成while label
static std::unordered_map<int, int> whf; // 记录当前while是否已经结束，是否需要生成返回指令

static std::unordered_map<std::string, bool> be_end_bl; // 记录当前block是否已经结束，是否需要生成返回指令

static std::unordered_map<std::string, bool> func_ret; // 记录当前函数是需要返回值，如int类型函数
static int be_func_para = 1; // 记录当前函数是否需要参数
static int func_cnt = 0; // 函数计数，方便生成函数label

static std::string depth_str = ""; // 当前深度的字符串形式
static std::string nowdepth_str = ""; // 当前深度的字符串形式

static std::string Koopa_IR = ""; // Koopa_IR
static std::string global_IR = ""; // 全局变量，库函数声明IR
static std::unordered_set<std::string> lib_func_decl_set; // 库函数声明集合

// 寄存器->数值，保存使用的number，来方便直接进行计算，而不是通过%0 = add 0, 1，放入寄存器后再计算
static std::unordered_map<std::string, std::string> reg2number;

static std::string Get_Identifier(std::string ident){
    std::string tempdepth_str = depth_str; //获取当前深度
    while(var_type.find("COMPILER__" + ident+"_"+tempdepth_str) == var_type.end()){
        tempdepth_str = f[tempdepth_str];
    } //循环向上获取深度
    return "COMPILER__" + ident + "_" + tempdepth_str;
}

class ProgramUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> compunit;

  void generate_Koopa_IR() const override{
    compunit->generate_Koopa_IR();
    Koopa_IR = global_IR + "\n" + Koopa_IR;
    std::cout<<Koopa_IR;
    return;
  }

  int Calc() const override{
    return compunit->Calc();
  }
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> funcdef;
  std::unique_ptr<BaseAST> decl;
  std::unique_ptr<BaseAST> compunit;
  int type;

  void generate_Koopa_IR() const override{
    if(type==1) {
        if(compunit) compunit->generate_Koopa_IR();
        funcdef->generate_Koopa_IR();
        return;
    }
    else if(type==2) {
        if(compunit) compunit->generate_Koopa_IR();
        decl->generate_Koopa_IR();
        return;
    }
    else {
        return;
    }
    return;
  } 

  int Calc() const override{
    if(type==1) return funcdef->Calc();
    else if(type==2) return decl->Calc();
    else if(type==3){
        return funcdef->Calc() + compunit->Calc();
    }
    else if(type==4){
        return decl->Calc() + compunit->Calc();
    }
  }
};

class FuncTypeAST : public BaseAST {
 public:
  std::string func_type;

  void generate_Koopa_IR() const override{
    if(func_type == "int") {
        Koopa_IR += ": i32 ";
    }
    return;
  }

    int Calc() const override{
        return 0;
    }
};

class FuncFParamAST : public BaseAST {
 public:
  std::string ident;

  void generate_Koopa_IR() const override{
    std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
    Koopa_IR += "  @" + Identifier + " = alloc i32\n";
    Koopa_IR += "  store %" + Identifier + ", @" + Identifier + "\n";
    var_type[Identifier] = 1;
    return;
  }

  void Show() const {
    Koopa_IR += "%COMPILER__" + ident + "_"+ depth_str + " :i32";
  }

  int Calc() const override{
    return 0;
  }
};

class FuncFParamsAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> funcfparam;
  std::unique_ptr<BaseAST> funcfparams;

  void generate_Koopa_IR() const override{
    if(funcfparams){
        funcfparams->generate_Koopa_IR();
        funcfparam->generate_Koopa_IR();
    }else{
        funcfparam->generate_Koopa_IR();
    }
    return;
  }

  void Show() const {
    if(funcfparams){
        FuncFParamsAST* FuncFParamsAST_ptr = dynamic_cast<FuncFParamsAST*>(funcfparams.get());
        FuncFParamAST* FuncFParamAST_ptr = dynamic_cast<FuncFParamAST*>(funcfparam.get());
        FuncFParamsAST_ptr->Show();
        Koopa_IR += ", ";
        FuncFParamAST_ptr->Show();
    }else{
        FuncFParamAST* FuncFParamAST_ptr = dynamic_cast<FuncFParamAST*>(funcfparam.get());
        FuncFParamAST_ptr->Show();
    }
  }

  int Calc() const override{
    if(funcfparams){
        return funcfparam->Calc() + funcfparams->Calc();
    }else{
        return funcfparam->Calc();
    }
  }
};

class BTypeAST : public BaseAST{
public:
    std::string type;

    void generate_Koopa_IR() const override{
        return ;
    }

    int Calc() const override{
        return 0;
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> funcfparams;

void generate_Koopa_IR() const override{
    func_cnt++;
    depth_str += "_4" + std::to_string(func_cnt);
    f[depth_str] = nowdepth_str;
    nowdepth_str = depth_str;

    Koopa_IR+="fun ";
    Koopa_IR+="@"+ident+"(";
    
    if(funcfparams){
        FuncFParamsAST* FuncFParamsAST_ptr = dynamic_cast<FuncFParamsAST*>(funcfparams.get());
        FuncFParamsAST_ptr->Show();
    }
    
    Koopa_IR+=")";

    BTypeAST* func_type_ptr = dynamic_cast<BTypeAST*>(func_type.get());
    //func_type->generate_Koopa_IR();
    if(func_type_ptr->type == "int") Koopa_IR+=": i32";

    if(func_type_ptr->type == "int") func_ret[ident] = true;
    else func_ret[ident] = false;

    Koopa_IR+="{\n";
    Koopa_IR += "%entry:\n";

    if(funcfparams){
       funcfparams->generate_Koopa_IR();
    }

    block->generate_Koopa_IR();

    if(!be_end_bl[depth_str+"_0"]) {
        if(func_ret[ident]) Koopa_IR += "  ret 0\n";
        else Koopa_IR += "  ret\n";
    }

    nowdepth_str = f[nowdepth_str];
    depth_str = nowdepth_str;
    Koopa_IR+="}\n\n";
    return;
  }
  int Calc() const override{
    return 0;
  }
};

class LValAST : public BaseAST{
public:
    std::string ident;

    void generate_Koopa_IR() const override{
        std::string Identifier = Get_Identifier(ident);
        if(var_type[Identifier] == 0){
            Koopa_IR += "  %"+std::to_string(now)+" = add "+"0, "+const_reg[Identifier]+"\n";
        }else{
            Koopa_IR += "  %"+std::to_string(now)+" = load @" + Identifier + "\n";
        }
        now++;
        return;
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
  std::unique_ptr<BaseAST> while_blockitemlist;

  int type;

  void generate_Koopa_IR() const override{
    if(be_end_bl[depth_str]) return ;
    if(type==1){
        exp->generate_Koopa_IR();
        auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
        IR_reg[exp_addr] = "%"+std::to_string(now-1);
        std::string addr = IR_reg[exp_addr];
        if(reg2number.find(addr) != reg2number.end()) addr = reg2number[addr];
        //Koopa_IR += lval->generate_Koopa_IR();
        int val = exp->Calc();
        LValAST* LVal_ptr = dynamic_cast<LValAST*>(lval.get());
        
        std::string ident = LVal_ptr->ident;
        std::string Identifier = Get_Identifier(ident);

        Koopa_IR += "  store " + addr + ", @" + Identifier + "\n";
        
    } else if(type==0) {
        if(be_end_bl[depth_str]) ;
        exp->generate_Koopa_IR();
        auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
        IR_reg[exp_addr] = "%"+std::to_string(now-1);
        std::string addr = IR_reg[exp_addr];
        if(reg2number.find(addr) != reg2number.end()) addr = reg2number[addr];

        Koopa_IR += "  ret " + addr + "\n\n";
        be_end_bl[depth_str] = true;


        std::string temp_depth = depth_str;
        while(temp_depth.size()>=2&&temp_depth.substr(temp_depth.size()-2) == "_0"){
            temp_depth = temp_depth.substr(0, temp_depth.size()-2);
            be_end_bl[temp_depth] = true;
        }
    } else if(type==2){
        exp->generate_Koopa_IR();
    } else if(type==3){
        block->generate_Koopa_IR();
    } else if(type==4){
        return;
    } else if(type==5){
        return;
    } else if(type==6||type==7){ // if block
        if(be_end_bl[depth_str]) return ; // 如果当前block已经结束，不再生成IR
        if_cnt++;
        int now_if = if_cnt;

        exp->generate_Koopa_IR();

        std::string addr1 = "%"+std::to_string(now-1);
        if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];

        Koopa_IR += "\tbr " + addr1 + ", %then_" + std::to_string(now_if) + ", %end_" + std::to_string(now_if) + "\n\n";
        Koopa_IR += "%then_" + std::to_string(now_if) + ":\n";

        depth_str += "_1"+std::to_string(now_if);
        f[depth_str] = nowdepth_str;
        nowdepth_str = depth_str;

        if_blockitemlist->generate_Koopa_IR(); // 生成if的block
        

        if(!be_end_bl[depth_str+"_0"]) Koopa_IR += "\tjump %end_" + std::to_string(now_if) + "\n\n";

        nowdepth_str = f[depth_str];
        depth_str = nowdepth_str;

        Koopa_IR += "%end_" + std::to_string(now_if) + ":\n";

    } else if(type==8||type==9||type==10||type==11){ // if else block
        if(be_end_bl[depth_str]) return; // 如果当前block已经结束，不再生成IR
        if_cnt++;
        int now_if=if_cnt;
        exp->generate_Koopa_IR();
        std::string addr1 = "%"+std::to_string(now-1);
        if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];

        Koopa_IR += "\tbr " + addr1 + ", %then_" + std::to_string(now_if) + ", %else_" + std::to_string(now_if) + "\n\n";
        Koopa_IR += "%then_" + std::to_string(now_if) + ":\n";


        depth_str += "_1"+std::to_string(now_if);
        f[depth_str] = nowdepth_str;
        nowdepth_str = depth_str;
        if_blockitemlist->generate_Koopa_IR(); // 生成if的block


        // 此时从if的block中退出，需要用nowdepth+1查看之前if的block中有没有出现return，如果if的block没有结束，跳转到end
        if(!be_end_bl[depth_str+"_0"]) Koopa_IR += "\tjump %end_" + std::to_string(now_if) + "\n\n";

        nowdepth_str = f[depth_str];
        depth_str = nowdepth_str;

        Koopa_IR += "%else_" + std::to_string(now_if) + ":\n";

        depth_str += "_2"+std::to_string(now_if);
        f[depth_str] = nowdepth_str;
        nowdepth_str = depth_str;

        else_blockitemlist->generate_Koopa_IR();
        
        if(!be_end_bl[depth_str+"_0"]) Koopa_IR += "\tjump %end_" + std::to_string(now_if) + "\n\n";

        nowdepth_str = f[depth_str];
        depth_str = nowdepth_str;

        Koopa_IR += "%end_" + std::to_string(now_if) + ":\n";
    } else if(type==12||type==13){
        while_cnt++;
        whf[while_cnt] = while_now;
        while_now = while_cnt;

        if(!be_end_bl[depth_str]) Koopa_IR += "  jump %while_" + std::to_string(while_now) + "\n\n"; // 如果当前block已经结束，不再生成IR

        Koopa_IR += "%while_" + std::to_string(while_now) + ":\n";

        exp->generate_Koopa_IR();

        std::string addr1 = "%"+std::to_string(now-1);
        if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];
 
        if(!be_end_bl[depth_str]) Koopa_IR += "\tbr " + addr1 + ", %while_then_" + std::to_string(while_now) + ", %end_while_" + std::to_string(while_now) + "\n\n";

        Koopa_IR += "%while_then_" + std::to_string(while_now) + ":\n";

        depth_str += "_3"+std::to_string(while_now); // 加3代表进入while的block

        f[depth_str] = nowdepth_str;
        nowdepth_str = depth_str;

        while_blockitemlist->generate_Koopa_IR(); // 生成while的block

        // 此时从if的block中退出，需要用nowdepth+1查看之前if的block中有没有出现return，如果if的block没有结束，跳转到end
        if(!be_end_bl[depth_str+"_0"]) Koopa_IR += "\tjump %while_" + std::to_string(while_now) + "\n\n";

        Koopa_IR += "%end_while_" + std::to_string(while_now) + ":\n";

        while_now = whf[while_now];

        nowdepth_str = f[depth_str];
        depth_str = nowdepth_str;
    } else if(type==14){
        Koopa_IR += "\tjump %end_while_" + std::to_string(while_now)+ "\n";
        be_end_bl[nowdepth_str]=true; 
    } else if(type==15){
        Koopa_IR +=  "\tjump %while_" + std::to_string(while_now)+ "\n";
        be_end_bl[nowdepth_str]=true;
    }
    return;
  }

    int Calc() const override{
        return 0;
    }
};

class ExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> lorexp;

    void generate_Koopa_IR() const override{
        lorexp->generate_Koopa_IR();
        IR_reg[reinterpret_cast<uintptr_t>(lorexp.get())] = "%"+std::to_string(now-1);
        return;
    }

    int Calc() const override{
        return lorexp->Calc();
    }

    std::vector<std::string> Para() const{
        std::vector<std::string> paras;
        generate_Koopa_IR();
        std::string addr = "%" + std::to_string(now-1);
        if(reg2number.find(addr)!=reg2number.end()) addr = reg2number[addr];
        paras.push_back(addr);
        return paras;
    }
};

class LOrExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> lorexp;
    std::unique_ptr<BaseAST> landexp;
    std::string lorexpop;


    void generate_Koopa_IR() const override{
        if(lorexp){  //|| 运算的短路求值
            lorexp->generate_Koopa_IR();
            int now1 = now - 1;
            int temp = now;
            Koopa_IR += "\t@result_" + std::to_string(temp) + "_" + depth_str + " = alloc i32\n";
            std::string addr1 = "%"+std::to_string(now1);
            if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];


            Koopa_IR += "\t%" + std::to_string(now) + " = ne 0, " + addr1 + "\n";
            Koopa_IR += "\tstore %" + std::to_string(now) + ", @result_" + std::to_string(temp) + "_" + depth_str + "\n";

            now++;
            if_cnt++;
            int now_if = if_cnt;

            Koopa_IR += "\tbr " + addr1 + ", %end_" + std::to_string(now_if) + ", %then_" + std::to_string(now_if) + "\n\n";
            Koopa_IR += "%then_" + std::to_string(now_if) + ":\n";
            landexp->generate_Koopa_IR();
            int now2 = now - 1;

            Koopa_IR += "\t%" + std::to_string(now) + " = ne 0, %" + std::to_string(now2) + "\n";
            now++;
            Koopa_IR += "\tstore %" + std::to_string(now-1) + ", @result_" + std::to_string(temp) + "_" + depth_str + "\n";
            Koopa_IR += "\tjump %end_" + std::to_string(now_if) + "\n\n";
            Koopa_IR += "%end_" + std::to_string(now_if) + ":\n";
            Koopa_IR += "\t%" + std::to_string(now) + " = load @result_" + std::to_string(temp) + "_" + depth_str + "\n";
            now++;
        }
        else{
            landexp->generate_Koopa_IR();
            IR_reg[reinterpret_cast<uintptr_t>(landexp.get())] = "%"+std::to_string(now-1);
        }
        return;
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

    void generate_Koopa_IR() const override{
        if(landexp){ // && 短路求值
            landexp->generate_Koopa_IR();
            int now1 = now - 1;
            int temp = now;
            Koopa_IR += "\t@result_" + std::to_string(temp) + "_" + depth_str + " = alloc i32\n";
            std::string addr1 = "%"+std::to_string(now1);
            if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];

            Koopa_IR += "\t%" + std::to_string(now) + " = ne 0, " + addr1 + "\n";
            Koopa_IR += "\tstore %" + std::to_string(now) + ", @result_" + std::to_string(temp) + "_" + depth_str + "\n";

            now++;
            if_cnt++;
            int now_if = if_cnt;

            Koopa_IR += "\tbr " + addr1 + ", %then_" + std::to_string(now_if) + ", %end_" + std::to_string(now_if) + "\n\n";
            Koopa_IR += "%then_" + std::to_string(now_if) + ":\n";
            eqexp->generate_Koopa_IR();
            int now2 = now - 1;
            Koopa_IR += "\t%" + std::to_string(now) + " = ne 0, %" + std::to_string(now2) + "\n";
            now++;
            Koopa_IR += "\tstore %" + std::to_string(now-1) + ", @result_" + std::to_string(temp) + "_" + depth_str + "\n";
            Koopa_IR += "\tjump %end_" + std::to_string(now_if) + "\n\n";
            Koopa_IR += "%end_" + std::to_string(now_if) + ":\n";
            Koopa_IR += "\t%" + std::to_string(now) + " = load @result_" + std::to_string(temp) + "_" + depth_str + "\n";
            now++;
        }
        /*if(landexp){
            landexp->generate_Koopa_IR();
            auto landexp_addr = reinterpret_cast<uintptr_t>(landexp.get());
            IR_reg[landexp_addr] = "%"+std::to_string(now-1);
            eqexp->generate_Koopa_IR();
            auto eqexp_addr = reinterpret_cast<uintptr_t>(eqexp.get());
            IR_reg[eqexp_addr] = "%"+std::to_string(now-1);
            if(landexpop == "&&"){
                Koopa_IR += "  %" + std::to_string(now++) + " = ne " + IR_reg[landexp_addr] + ", 0" + "\n";
                Koopa_IR += "  %" + std::to_string(now++) + " = ne " + IR_reg[eqexp_addr] + ", 0" + "\n";
                Koopa_IR += "  %" + std::to_string(now) + " = and " + "%"+std::to_string(now-1) + ", " + "%"+std::to_string(now-2) + "\n";
                now++;
            }
        }*/
        else{
            eqexp->generate_Koopa_IR();
            IR_reg[reinterpret_cast<uintptr_t>(eqexp.get())] = "%"+std::to_string(now-1);
        }
        return;
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

    void generate_Koopa_IR() const override{
        if(eqexp){
            eqexp->generate_Koopa_IR();
            auto eqexp_addr = reinterpret_cast<uintptr_t>(eqexp.get());
            IR_reg[eqexp_addr] = "%"+std::to_string(now-1);
            std::string addr1 = IR_reg[eqexp_addr];
            if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];

            auto relexp_addr = reinterpret_cast<uintptr_t>(relexp.get());
            relexp->generate_Koopa_IR();
            IR_reg[relexp_addr] = "%"+std::to_string(now-1);
            std::string addr2 = IR_reg[relexp_addr];
            if(reg2number.find(addr2) != reg2number.end()) addr2 = reg2number[addr2];

            if(eqexpop == "=="){
                Koopa_IR += "  %" + std::to_string(now) + " = eq " + addr1 + ", " + addr2 + "\n";
                now++;
            }else if(eqexpop == "!="){
                Koopa_IR += "  %" + std::to_string(now) + " = ne " + addr1 + ", " + addr2 + "\n";
                now++;
            }
        }
        else{
            relexp->generate_Koopa_IR();
            auto relexp_addr = reinterpret_cast<uintptr_t>(relexp.get());
            IR_reg[relexp_addr] = "%"+std::to_string(now-1);
        }
        return ;
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

    void generate_Koopa_IR() const override{
        if(relexp){
            relexp->generate_Koopa_IR();
            auto relexp_addr = reinterpret_cast<uintptr_t>(relexp.get());
            IR_reg[relexp_addr] = "%"+std::to_string(now-1);
            std::string addr1 = IR_reg[relexp_addr];
            if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];


            addexp->generate_Koopa_IR();
            auto addexp_addr = reinterpret_cast<uintptr_t>(addexp.get());
            IR_reg[addexp_addr] = "%"+std::to_string(now-1);
            std::string addr2 = IR_reg[addexp_addr];
            if(reg2number.find(addr2) != reg2number.end()) addr2 = reg2number[addr2];

            if(relexpop == "<"){
                Koopa_IR += "  %" + std::to_string(now) + " = lt " + addr1 + ", " + addr2 + "\n";
                now++;
            }else if(relexpop == "<="){
                Koopa_IR += "  %" + std::to_string(now) + " = le " + addr1 + ", " + addr2 + "\n";
                now++;
            }else if(relexpop == ">"){
                Koopa_IR += "  %" + std::to_string(now) + " = gt " + addr1 + ", " + addr2 + "\n";
                now++;
            }else if(relexpop == ">="){
                Koopa_IR += "  %" + std::to_string(now) + " = ge " + addr1 + ", " + addr2 + "\n";
                now++;
            }
        }
        else{
            addexp->generate_Koopa_IR();
            auto addexp_addr = reinterpret_cast<uintptr_t>(addexp.get());
            IR_reg[addexp_addr] = "%"+std::to_string(now-1);
        }
        return;
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

    void generate_Koopa_IR() const override{
        if(addexp){
            addexp->generate_Koopa_IR();
            auto addexp_addr = reinterpret_cast<uintptr_t>(addexp.get());
            IR_reg[addexp_addr] = "%"+std::to_string(now-1);
            std::string addr1 = IR_reg[addexp_addr];
            if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];

            mulexp->generate_Koopa_IR();
            auto mulexp_addr = reinterpret_cast<uintptr_t>(mulexp.get());
            IR_reg[mulexp_addr] = "%"+std::to_string(now-1);
            std::string addr2 = IR_reg[mulexp_addr];
            if(reg2number.find(addr2) != reg2number.end()) addr2 = reg2number[addr2];

            if(addexpop == "+"){
                Koopa_IR += "  %" + std::to_string(now) + " = add " + addr1 + ", " + addr2 + "\n";
                now++;
            }else if(addexpop == "-"){
                Koopa_IR += "  %" + std::to_string(now) + " = sub " + addr1 + ", " + addr2 + "\n";
                now++;
            }
        }
        else{
            mulexp->generate_Koopa_IR();
            auto mulexp_addr = reinterpret_cast<uintptr_t>(mulexp.get());
            IR_reg[mulexp_addr] = "%"+std::to_string(now-1);
        }
        return ;
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

    void generate_Koopa_IR() const override{
        if(mulexp){
            mulexp->generate_Koopa_IR();
            auto mulexp_addr = reinterpret_cast<uintptr_t>(mulexp.get());
            IR_reg[mulexp_addr] = "%"+std::to_string(now-1);
            std::string addr1 = IR_reg[mulexp_addr];
            if(reg2number.find(addr1) != reg2number.end()) addr1 = reg2number[addr1];

            unaryexp->generate_Koopa_IR();
            auto unaryexp_addr = reinterpret_cast<uintptr_t>(unaryexp.get());
            IR_reg[unaryexp_addr] = "%"+std::to_string(now-1);
            std::string addr2 = IR_reg[unaryexp_addr];
            if(reg2number.find(addr2) != reg2number.end()) addr2 = reg2number[addr2];

            if(mulexpop == "*"){
                Koopa_IR += "  %" + std::to_string(now) + " = mul " + addr1 + ", " + addr2 + "\n";
                now++;
            }else if(mulexpop == "/"){
                Koopa_IR += "  %" + std::to_string(now) + " = div " + addr1 + ", " + addr2 + "\n";
                now++;
            }
            else if(mulexpop == "%"){
                Koopa_IR += "  %" + std::to_string(now) + " = mod " + addr1 + ", " + addr2 + "\n";
                now++;
            }
        }
        else{
            unaryexp->generate_Koopa_IR();
            auto unaryexp_addr = reinterpret_cast<uintptr_t>(unaryexp.get());
            IR_reg[unaryexp_addr] = "%"+std::to_string(now-1);
        }
        return ;
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

class FuncRParamsAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> funcrparams;

    void generate_Koopa_IR() const override{
        if(funcrparams){
            funcrparams->generate_Koopa_IR();
            exp->generate_Koopa_IR();
            auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
            IR_reg[exp_addr] = "%"+std::to_string(now-1);
        }else{
            exp->generate_Koopa_IR();
            auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
            IR_reg[exp_addr] = "%"+std::to_string(now-1);
        }
        return ;
    }

    int Calc() const override{
        if(funcrparams){
            return exp->Calc() + funcrparams->Calc();
        }else{
            return exp->Calc();
        }
    }

    std::vector<std::string> Para() const{
        std::vector<std::string> paras;
        if(funcrparams){
            FuncRParamsAST* FuncRParamsAST_ptr1 = dynamic_cast<FuncRParamsAST*>(funcrparams.get());
            std::vector<std::string> para1 = FuncRParamsAST_ptr1->Para();
            ExpAST* FuncRParamsAST_ptr2 = dynamic_cast<ExpAST*>(exp.get());
            std::vector<std::string> para2 =  FuncRParamsAST_ptr2->Para();
            for(auto it=para1.begin(); it!=para1.end(); it++) paras.push_back(*it);
            for(auto it=para2.begin(); it!=para2.end(); it++) paras.push_back(*it);
        }else{
            generate_Koopa_IR();
            std::string addr = "%" + std::to_string(now-1);
            if(reg2number.find(addr)!=reg2number.end()) addr = reg2number[addr];
            paras.push_back(addr);
        }
        return paras;
    }
};


class UnaryExpAST : public BaseAST {
public:
    std::unique_ptr<BaseAST> primaryexp;
    std::string unaryop;
    std::unique_ptr<BaseAST> unaryexp;
    std::unique_ptr<BaseAST> funcrparams;
    std::string ident;
    int type;

    void generate_Koopa_IR() const override{
        if((type==3||type==4)&&lib_func_decl_set.find(ident)==lib_func_decl_set.end()){
            if(ident=="getint"){
                global_IR += "decl @getint(): i32\n";
            }else if(ident=="getch"){
                global_IR += "decl @getch(): i32\n";
            }else if(ident=="getarray"){
                global_IR += "decl @getarray(*32): i32\n";
            }else if(ident=="putint"){
                global_IR += "decl @putint(i32)\n";
            }else if(ident=="putch"){
                global_IR += "decl @putch(i32)\n";
            }else if(ident=="putarray"){
                global_IR += "decl @putarray(i32, *32)\n";
            }else if(ident=="putf"){
                global_IR += "decl @putf(i32)\n";
            }else if(ident=="starttime"){
                global_IR += "decl @starttime()\n";
            }else if(ident=="stoptime"){
                global_IR += "decl @stoptime()\n";
            }
            lib_func_decl_set.insert(ident);
        }

        if(type==1){
            unaryexp->generate_Koopa_IR();
            auto unaryexp_addr = reinterpret_cast<uintptr_t>(unaryexp.get());
            IR_reg[unaryexp_addr] = "%"+std::to_string(now-1);
            std::string addr = IR_reg[unaryexp_addr];
            if(reg2number.find(addr) != reg2number.end()) addr = reg2number[addr];


            if(unaryop == "-"){
                Koopa_IR += "  %" + std::to_string(now) + " = sub 0, " + addr +" \n";
                now++;
            }else if(unaryop == "!"){
                Koopa_IR += "  %" + std::to_string(now) + " = eq 0, " + addr +" \n";
                now++;
            }else if(unaryop == "+"){
                int x = 1;
            }
        }
        else if(type==2){
            primaryexp->generate_Koopa_IR();
            auto primaryexp_addr = reinterpret_cast<uintptr_t>(primaryexp.get());
            IR_reg[primaryexp_addr] = "%"+std::to_string(now-1);
        }
        else if(type==3){
            if(ident=="getint"||ident=="getch"||ident=="getarray"||func_ret[ident]){
                Koopa_IR += "  %"+std::to_string(now)+" = call @"+ident+"()\n";
                now++;
            }else{
                Koopa_IR += "  call @" + ident + "()\n";
            }
        }else if(type==4){
            be_func_para = 1;
            FuncRParamsAST* FuncRParamsAST_ptr = dynamic_cast<FuncRParamsAST*>(funcrparams.get());
            std::vector<std::string> paras = FuncRParamsAST_ptr->Para();
            if(ident=="getint"||ident=="getch"||ident=="getarray"||func_ret[ident]){
                Koopa_IR += "  %"+std::to_string(now)+" = call @"+ident+"(";
                now++;
            } else {
                Koopa_IR += "  call @" + ident + "(";
            }
            for(auto it=paras.begin(); it!=paras.end(); it++){
                if(it!=paras.begin()) Koopa_IR += ", ";
                std::string addr = *it;
                Koopa_IR += addr;
            }
            Koopa_IR += ")\n";
            be_func_para = 0;
        }
        return ;
    }

    int Calc() const override{
        if(unaryexp){
            if(unaryop == "-"){
                return -unaryexp->Calc();
            }else if(unaryop == "!"){
                return !unaryexp->Calc();
            }
        }
        else if(primaryexp){
            return primaryexp->Calc();
        }
        return 0;
    }
};


class PrimaryExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> lval;
    int number;

    void generate_Koopa_IR() const override{
        if(exp){
            exp->generate_Koopa_IR();
            auto exp_addr = reinterpret_cast<uintptr_t>(exp.get());
            IR_reg[exp_addr] = "%"+std::to_string(now-1);
        } else if(lval){
            lval->generate_Koopa_IR();
            auto lval_addr = reinterpret_cast<uintptr_t>(lval.get());
            IR_reg[lval_addr] = "%"+std::to_string(now-1);
        } else {
            //Koopa_IR += "  %" + std::to_string(now) + " = add 0, " + std::to_string(number) +" \n";
            reg2number["%" + std::to_string(now-1)] = std::to_string(number);
            //now++;
        }
        return ;
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

    void generate_Koopa_IR() const override{
        if(constdecl){
            constdecl->generate_Koopa_IR();
        }else{
            vardecl->generate_Koopa_IR();
        }
        return;
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

    void generate_Koopa_IR() const override{
        btype->generate_Koopa_IR();
        vardeflist->generate_Koopa_IR();
        return ;
    }

    int Calc() const override{
        return vardeflist->Calc();
    }
};

class VarDefListAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> vardef;
    std::unique_ptr<BaseAST> vardeflist;

    void generate_Koopa_IR() const override{
        if(vardeflist){
            vardeflist->generate_Koopa_IR();
            vardef->generate_Koopa_IR();
        }else{
            vardef->generate_Koopa_IR();
        }
        return ;
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


    void generate_Koopa_IR() const override{
        if(initval){
            if(depth_str != ""){
                std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
                Koopa_IR += "  @" + Identifier + " = alloc i32\n";
                var_type[Identifier] = 1;
                const_val[Identifier] = initval->Calc();
                initval->generate_Koopa_IR();

                std::string addr = "%"+std::to_string(now-1);
                if(reg2number.find(addr) != reg2number.end()) addr = reg2number[addr];
                Koopa_IR += "  store " + addr + ", @" + Identifier + "\n";
            }else{
                std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
                var_type[Identifier] = 1;
                const_val[Identifier] = initval->Calc();
                Koopa_IR += "global @" + Identifier + " = alloc i32, " + std::to_string(const_val[Identifier]) + "\n";
            }
        }
        else{
            if(depth_str != ""){
                std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
                Koopa_IR += "  @" + Identifier + " = alloc i32\n";
                var_type[Identifier] = 1;
                const_val[Identifier] = 0;
            }
            else {
                std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
                Koopa_IR += "global @" + Identifier + " = alloc i32, 0\n";
                var_type[Identifier] = 1;
                const_val[Identifier] = 0;
            }
        }
        return;
    }

    int Calc() const override{
        return initval->Calc();
    }
};

class InitValAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;

    void generate_Koopa_IR() const override{
        exp->generate_Koopa_IR();
        return ;
    }

    int Calc() const override{
        return exp->Calc();
    }
};

class ConstDeclAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> constdeflist;

    void generate_Koopa_IR() const override{
        btype->generate_Koopa_IR();
        constdeflist->generate_Koopa_IR();
        return ;
    }

    int Calc() const override{
        return constdeflist->Calc();
    }
};

class ConstDefListAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> constdef;
    std::unique_ptr<BaseAST> constdeflist;

    void generate_Koopa_IR() const override{
        if(constdeflist){
            constdeflist->generate_Koopa_IR();
            constdef->generate_Koopa_IR();
        }else{
            constdef->generate_Koopa_IR();
        }
        return ;
    }

    int Calc() const override{
        return constdef->Calc();
    }
};

class ConstDefAST : public BaseAST{
public:
    std::string ident;
    std::unique_ptr<BaseAST> constinitval;

    void generate_Koopa_IR() const override{
        //Koopa_IR += constinitval->generate_Koopa_IR();
        int val = this->Calc();
        auto constinitval_addr = reinterpret_cast<uintptr_t>(constinitval.get());
        IR_reg[constinitval_addr] = std::to_string(val);
        std::string Identifier = "COMPILER__" + ident + "_"+ depth_str;
        const_reg[Identifier] = IR_reg[constinitval_addr]; // 常量放入对应寄存器
        return ;
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

    void generate_Koopa_IR() const override{
        constexp->generate_Koopa_IR();
        return ;
    }

    int Calc() const override{
        return constexp->Calc();
    }
};

class BlockAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> blockitemlist;
    int type;

    void generate_Koopa_IR() const override{
        if(type==0){
            blockitemlist->generate_Koopa_IR();  // 生成block的IR
        }else if(type==1){
            return;
        }
        return ;
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

    void generate_Koopa_IR() const override{
        if(blockitemlist){
            blockitemlist->generate_Koopa_IR();
            blockitem->generate_Koopa_IR();
        }else{
            blockitem->generate_Koopa_IR();
        }
        return ;
    }

    int Calc() const override{
        return blockitem->Calc();
    }
};

class BlockItemAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> decl;
    std::unique_ptr<BaseAST> stmt;

    void generate_Koopa_IR() const override{
        if(be_end_bl[depth_str]){
            return ;
        }
        depth_str += "_0";            // 进入block，深度加1q
        f[depth_str]=nowdepth_str;   // 新的block的父节点是之前的block
        nowdepth_str=depth_str;      // nowdepth更新为当前深度
        if(decl){
            decl->generate_Koopa_IR();
        }else if(stmt){
            stmt->generate_Koopa_IR();
        }
        nowdepth_str=f[nowdepth_str];   // 离开block前，恢复当前深度depth为父节点的深度
        depth_str = nowdepth_str;       // 恢复当前深度
        return ;
    }

    int Calc() const override{
        return 0;
    }
};


class ConstExpAST : public BaseAST{
public:
    std::unique_ptr<BaseAST> exp;

    void generate_Koopa_IR() const override{
        exp->generate_Koopa_IR();
        return ;
    }

    int Calc() const override{
        return exp->Calc();
    }
};