#pragma once
#include<iostream>
#include<cstdio>
#include<memory>

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual std::string generate_Koopa_IR() const = 0;
};

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
  int number;

  void Dump() const override{
    std::cout << "StmtAST { ";
    std::cout<<number;
    std::cout<<" }";
  } 

  std::string generate_Koopa_IR() const override{
    return "  ret "+std::to_string(number)+"\n";
  }
};