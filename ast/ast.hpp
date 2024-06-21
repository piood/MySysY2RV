#pragma once
#include<iostream>
#include<cstdio>
#include<memory>

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
  virtual void generate_Koopa_IR() const = 0;
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

  void generate_Koopa_IR() const override{
    func_def->generate_Koopa_IR();
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
    std::cout<<" }";
  }

  void generate_Koopa_IR() const override{
    std::cout<<"fun ";
    std::cout<<"@"<<ident<<"(): ";
    func_type->generate_Koopa_IR();
    std::cout<<"{\n";
    block->generate_Koopa_IR();
    std::cout<<"}";
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

  void generate_Koopa_IR() const override{
    std::cout<<"i32 ";
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

  void generate_Koopa_IR() const override{
    std::cout << "%entry:\n";
    stmt->generate_Koopa_IR();
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

  void generate_Koopa_IR() const override{
    std::cout<<"  ret "<<number<<"\n";
  }
};