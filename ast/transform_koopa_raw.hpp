#include<iostream>
#include<cassert>
#include "koopa.h"

void koopa_IR2koopa_raw(const char *str);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);


void koopa_IR2koopa_raw(const char *str){
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);

    assert(ret == KOOPA_EC_SUCCESS);

    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();

    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);

    koopa_delete_program(program);

    Visit(raw);

    koopa_delete_raw_program_builder(builder);
}

void Koopa_IR2RISC_V(const char *str){
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);

    assert(ret == KOOPA_EC_SUCCESS);

    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();

    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);

    koopa_delete_program(program);

    for(int i = 0; i < raw.funcs.len; i++){

        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);

        koopa_raw_function_t func =(koopa_raw_function_t) raw.funcs.buffer[i];
        for(int j = 0; j < func->bbs.len; j++){

            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);

            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];
            for(int k = 0; k < bb->insts.len; k++){
                koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];
                if(value->kind.tag == KOOPA_RVT_RETURN){
                    koopa_raw_return_t ret = value->kind.data.ret;

                    if(ret.value->kind.tag == KOOPA_RVT_INTEGER){
                        koopa_raw_integer_t integer = ret.value->kind.data.integer;
                        if(integer.value == 0){
                            std::cout<<"integer value"<<integer.value<<std::endl;
                        }
                    }
                }
            }
        }
    }

    koopa_delete_raw_program_builder(builder);
}



void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  Visit(program.values);
  // 访问所有函数
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

void Visit(const koopa_raw_return_t &ret) {
  // 访问返回值
  koopa_raw_value_t ret_value = ret.value;
  assert(ret_value->kind.tag == KOOPA_RVT_INTEGER);

  Visit(ret_value);
}

void Visit(const koopa_raw_integer_t &integer) {
  // 访问整数值
  // ...
  
  int32_t int_val = integer.value;
  assert(int_val == 0);
}