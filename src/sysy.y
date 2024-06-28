%code requires {
  #include <memory>
  #include <string>
  #include "../ast/ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../ast/ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN CONST IF ELSE
%token <str_val> IDENT LOROP LANDOP EQOP RELOP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp MulExp AddExp LOrExp LAndExp EqExp RelExp Decl ConstDecl BType ConstDef ConstDefList ConstInitVal BlockItemList BlockItem LVal ConstExp VarDecl VarDefList VarDef InitVal
%type <int_val> Number
%type <str_val> UnaryOp MulExpOp AddExpOp

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->INT = "int";
    $$ = ast;
  }
  ;

Stmt
  : ';' {
    auto ast = new StmtAST();
    ast->type = 4;
    $$ = ast;
  }
  | RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    ast->type = 0;
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->lval = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    ast->type = 1;
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast->type = 2;
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->block = unique_ptr<BaseAST>($1);
    ast->type = 3;
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new StmtAST();
    ast->type = 5;
    $$ = ast;
  }
  | IF '(' Exp ')' BlockItem {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_blockitemlist = unique_ptr<BaseAST>($5);
    ast->type = 6;
    $$ = ast;
  }
  | IF '(' Exp ')' '{' BlockItemList '}' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_blockitemlist = unique_ptr<BaseAST>($6);
    ast->type = 7;
    $$ = ast;
  }
  | IF '(' Exp ')' BlockItem ELSE BlockItem {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_blockitemlist = unique_ptr<BaseAST>($5);
    ast->else_blockitemlist = unique_ptr<BaseAST>($7);
    ast->type = 8;
    $$ = ast;
  }
  | IF '(' Exp ')' '{' BlockItemList '}' ELSE BlockItem {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_blockitemlist = unique_ptr<BaseAST>($6);
    ast->else_blockitemlist = unique_ptr<BaseAST>($9);
    ast->type = 9;
    $$ = ast;
  }
  | IF '(' Exp ')' BlockItem ELSE '{' BlockItemList '}' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_blockitemlist = unique_ptr<BaseAST>($5);
    ast->else_blockitemlist = unique_ptr<BaseAST>($8);
    ast->type = 10;
    $$ = ast;
  }
  | IF '(' Exp ')' '{' BlockItemList '}' ELSE '{' BlockItemList '}' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->if_blockitemlist = unique_ptr<BaseAST>($6);
    ast->else_blockitemlist = unique_ptr<BaseAST>($10);
    ast->type = 11;
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lorexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

MulExp
  : MulExp MulExpOp UnaryExp{
    auto ast = new MulExpAST();
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->mulexpop = *unique_ptr<string>($2);
    ast->unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | UnaryExp {
    auto ast = new MulExpAST();
    ast->unaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

AddExp
  : AddExp AddExpOp MulExp{
    auto ast = new AddExpAST();
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->addexpop = *unique_ptr<string>($2);
    ast->mulexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp {
    auto ast = new AddExpAST();
    ast->mulexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

UnaryExp
  : UnaryOp UnaryExp {
    auto ast = new UnaryExpAST();
    ast->unaryop = *unique_ptr<string>($1);
    ast->unaryexp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->primaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST();
    ast->lval = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  |  Number {
    auto ast = new PrimaryExpAST();
    ast->number = $1;
    $$ = ast;
  }
  ;

LOrExp
  : LOrExp LOROP LAndExp {
    auto ast = new LOrExpAST();
    ast->lorexp = unique_ptr<BaseAST>($1);
    ast->lorexpop = *unique_ptr<string>($2);
    ast->landexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | LAndExp {
    auto ast = new LOrExpAST();
    ast->landexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

LAndExp
  : LAndExp LANDOP EqExp {
    auto ast = new LAndExpAST();
    ast->landexp = unique_ptr<BaseAST>($1);
    ast->landexpop = *unique_ptr<string>($2);
    ast->eqexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | EqExp {
    auto ast = new LAndExpAST();
    ast->eqexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

EqExp
  : EqExp EQOP RelExp {
    auto ast = new EqExpAST();
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->eqexpop = *unique_ptr<string>($2);
    ast->relexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp {
    auto ast = new EqExpAST();
    ast->relexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

RelExp
  : RelExp RELOP AddExp {
    auto ast = new RelExpAST();
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->relexpop = *unique_ptr<string>($2);
    ast->addexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | AddExp {
    auto ast = new RelExpAST();
    ast->addexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->constdecl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->vardecl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

VarDecl
  : BType VarDefList ';' {
    auto ast = new VarDeclAST();
    ast->btype = unique_ptr<BaseAST>($1);
    ast->vardeflist = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

VarDefList
  : VarDef {
    auto ast = new VarDefListAST();
    ast->vardef = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDefList ',' VarDef {
    auto ast = new VarDefListAST();
    ast->vardeflist = unique_ptr<BaseAST>($1);
    ast->vardef = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->initval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDefList ';' {
     auto ast = new ConstDeclAST();
     ast->btype = unique_ptr<BaseAST>($2);
     ast->constdeflist = unique_ptr<BaseAST>($3);
     $$ = ast;
  }
  ;

ConstDefList
  : ConstDef {
    auto ast = new ConstDefListAST();
    ast->constdef = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ConstDefList ',' ConstDef {
    auto ast = new ConstDefListAST();
    ast->constdeflist = unique_ptr<BaseAST>($1);
    ast->constdef = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

BType
  : INT {
    auto ast = new BTypeAST();
    ast->type = "int";
    $$ = ast;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal{
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->constinitval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->constexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Block
  : '{' BlockItemList '}' {
    auto ast = new BlockAST();
    ast->blockitemlist = unique_ptr<BaseAST>($2);
    ast->type = 0;
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new BlockAST();
    ast->type = 1;
    $$ = ast;
  }
  ;

BlockItemList
  : BlockItem {
    auto ast = new BlockItemListAST();
    ast->blockitem = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | BlockItemList BlockItem {
    auto ast = new BlockItemListAST();
    ast->blockitemlist = unique_ptr<BaseAST>($1);
    ast->blockitem = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

LVal
  : IDENT {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

UnaryOp
  : '+' {
    $$ = new std::string("+");
  }
  | '-' {
    $$ = new std::string("-");
  }
  | '!' {
    $$ = new std::string("!");
  }
  ;

MulExpOp
  : '*' {
    $$ = new std::string("*");
  }
  | '/' {
    $$ = new std::string("/");
  }
  | '%' {
    $$ = new std::string("%");
  }
  ;

AddExpOp
  : '+' {
    $$ = new std::string("+");
  }
  | '-' {
    $$ = new std::string("-");
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}