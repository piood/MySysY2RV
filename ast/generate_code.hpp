#include<iostream>
#include<cassert>
#include "koopa.h"
#include <string>
#include <unordered_map>

std::string risc_v_code;

int temp_var_counter = 0;

std::unordered_map<koopa_raw_value_t, std::string> instruction2reg;

std::string Koopa_IR2RISC_V(const char *str);

void generate_code(const koopa_raw_program_t &program);
void generate_code(const koopa_raw_slice_t &slice);
void generate_code(const koopa_raw_function_t &func);
void generate_code(const koopa_raw_basic_block_t &bb);
void generate_code(const koopa_raw_value_t &value);
void generate_code(const koopa_raw_return_t &ret);
void generate_code(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);


std::string Koopa_IR2RISC_V(const char *str){
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);

    assert(ret == KOOPA_EC_SUCCESS);

    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();

    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);

    koopa_delete_program(program);

    risc_v_code = "  .text\n";

    risc_v_code += "  .globl";

    //得到所有定义的函数名
    for(int i = 0; i < raw.funcs.len; i++){
        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func =(koopa_raw_function_t) raw.funcs.buffer[i];
        const char* function_name = func->name;
        std::string function_name_str(function_name);
        risc_v_code += " "+function_name_str.std::string::substr(1);
    }
    risc_v_code += "\n";

    generate_code(raw);

    koopa_delete_raw_program_builder(builder);

    return risc_v_code;
}

void generate_code(const koopa_raw_program_t &program){
    generate_code(program.funcs);
}

void generate_code(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        generate_code(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        generate_code(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        generate_code(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

void generate_code(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  generate_code(bb->insts);
}

void generate_code(const koopa_raw_function_t &func) {
  const char* function_name = func->name;
  std::string function_name_str(function_name);
  risc_v_code += function_name_str.substr(1) + ":\n";

  generate_code(func->bbs);
}

void generate_code(const koopa_raw_return_t &ret) {
  // 访问返回值
  koopa_raw_value_t ret_value = ret.value;

  generate_code(ret_value);

  std::string ret_reg = instruction2reg[ret_value];
  risc_v_code += "  mv a0, " + ret_reg + "\n";
  risc_v_code += "  ret\n";
}

void generate_code(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value) {
  // 访问整数值
  // ...
    int32_t int_val = integer.value;
    if(int_val == 0){
        instruction2reg[value] = "x0";
    }else{
        std::string reg = "t" + std::to_string(temp_var_counter++);
        risc_v_code += "  li " + reg + ", " + std::to_string(int_val) + "\n";
        instruction2reg[value] = reg;
    }
}

void generate_code(const koopa_raw_value_t &value){
    //访问二元运算
    //...
    if (instruction2reg.find(value) != instruction2reg.end()) {
        return; // 如果指令已经生成，则不再重复生成
    }
    const auto &kind = value->kind;
    if(kind.tag==KOOPA_RVT_RETURN){
        // 访问 return 指令
        generate_code(kind.data.ret);
    }
    else if(kind.tag==KOOPA_RVT_INTEGER){
        // 访问 integer 指令
        generate_code(kind.data.integer, value);
    }
    else if(kind.tag==KOOPA_RVT_BINARY){
        koopa_raw_binary_t binary = kind.data.binary;
        koopa_raw_value_t lhs = binary.lhs;
        koopa_raw_value_t rhs = binary.rhs;

        koopa_raw_binary_op_t op = binary.op;

        generate_code(lhs);
        generate_code(rhs);
        std::string lreg = instruction2reg[lhs];
        std::string rreg = instruction2reg[rhs];

        std::string result_reg = "t"+std::to_string(temp_var_counter++);
        instruction2reg[value] = result_reg;
        switch (op) {
            case KOOPA_RBO_ADD:
                risc_v_code += "  add " + result_reg + ", " + lreg + ", " + rreg + "\n";
                break;
            case KOOPA_RBO_SUB:
                risc_v_code += "  sub " + result_reg + ", " + lreg + ", " + rreg + "\n";
                break;
            case KOOPA_RBO_EQ:
                risc_v_code += "  seqz " + result_reg + ", " + rreg + "\n";
                break;
            case KOOPA_RBO_NOT_EQ:
                risc_v_code += "  snez " + result_reg + ", " + rreg + "\n";
                break;
            default:
                std::cout << "Unhandled binary operation: " << op << std::endl;
        }
    }
    else{
        // 其他类型暂时遇不到
        std::cout << "Unhandled instruction kind: " << kind.tag << std::endl;
    }
}