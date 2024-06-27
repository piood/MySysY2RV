#include<iostream>
#include<cassert>
#include "koopa.h"
#include <string>
#include <unordered_map>
#include <unordered_set>

std::string Koopa_IR2RISC_V(const char *str);

void generate_code(const koopa_raw_program_t &program);
void generate_code(const koopa_raw_slice_t &slice);
void generate_code(const koopa_raw_function_t &func);
void generate_code(const koopa_raw_basic_block_t &bb);
void generate_code(const koopa_raw_value_t &value);
void generate_code(const koopa_raw_return_t &ret);
void generate_code(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);
void generate_code(const koopa_raw_store_t &store, const koopa_raw_value_t &value);
void generate_code(const koopa_raw_load_t &load, const koopa_raw_value_t &value);

std::string binary_get_reg_release_unused_reg(std::string lreg, std::string rreg);
std::string get_reg();
void release_reg(std::string reg);
void get_program_instruction_time(const koopa_raw_program_t &program);

std::string risc_v_code;


int st = 0; // 栈指针

std::unordered_map<koopa_raw_value_t, std::string> instruction2reg;  // 指令运行结果存放的寄存器的映射

std::unordered_map<koopa_raw_value_t, int> instruction_time; // 记录指令运行结果存放的寄存器的使用次数

std::unordered_map<std::string, koopa_raw_value_t> reg2instruction; // 记录寄存器对应的指令


std::unordered_set<int> use_reg;

std::string get_reg(){
    for(int i=0;i<=6;i++){
        if(use_reg.find(i)==use_reg.end()){
            use_reg.insert(i);
            return "t"+std::to_string(i);
        }
    }
    for(int i=0;i<=7;i++){
        if(use_reg.find(i+7)==use_reg.end()){
            use_reg.insert(i+7);
            return "a"+std::to_string(i);
        }
    }
    std::cout<<"no reg to use"<<std::endl;
    return "error";
}

std::string binary_get_reg_release_unused_reg(std::string lreg, std::string rreg){
    std::string result_reg;
    if(lreg!="x0"){
        result_reg = lreg;
    }else if(rreg!="x0"){
        result_reg = rreg;
    }else{
        result_reg = get_reg();
    }
    if(result_reg!=lreg){
        release_reg(lreg);
    }
    if(result_reg!=rreg){
        release_reg(rreg);
    }
    return result_reg;
}

void release_reg(std::string reg){
    if(reg=="x0") return;
    int index = reg[1]-'0';
    char head = reg[0];
    if(head=='t')
        use_reg.erase(index);
    else if(head=='a'){
        use_reg.erase(index+7);
    }
}


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
  risc_v_code += "  addi sp, sp, -256\n";
  generate_code(func->bbs);
}

void generate_code(const koopa_raw_return_t &ret) {
  // 访问返回值
  koopa_raw_value_t ret_value = ret.value;

  generate_code(ret_value);

  std::string ret_reg = instruction2reg[ret_value];
  risc_v_code += "  mv a0, " + ret_reg + "\n";
  risc_v_code += "  addi sp, sp, 256\n";
  risc_v_code += "  ret\n";
}

void generate_code(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value) {
  // 访问整数值
  // ...
    int32_t int_val = integer.value;
    if(int_val == 0){
        instruction2reg[value] = "x0";
    }else{
        std::string reg = get_reg();
        //std::string reg = "t0";
        risc_v_code += "  li " + reg + ", " + std::to_string(int_val) + "\n";
        instruction2reg[value] = reg;
    }
}

void generate_code(const koopa_raw_load_t &load, const koopa_raw_value_t &value) {
  // 访问 load 指令
  // ...
    koopa_raw_value_t loa_src = load.src;
    //std::string reg = "t0";
    std::string reg=get_reg();

    risc_v_code += "  lw "+ reg + ", " + instruction2reg[loa_src] + "(sp)\n";
    instruction2reg[value] = reg;
}

void generate_code(const koopa_raw_store_t &store, const koopa_raw_value_t &value) {
  // 访问 store 指令
  // ...
    koopa_raw_value_t sto_value = store.value;
    koopa_raw_value_t sto_dest = store.dest;
    if(sto_value->kind.tag == KOOPA_RVT_INTEGER){
        risc_v_code += "   li t0, " + std::to_string(sto_value->kind.data.integer.value) + "\n";
    }
    if(instruction2reg.find(sto_dest)==instruction2reg.end()){
        instruction2reg[sto_dest] = std::to_string(st);
        st += 4;
    }else{
        std::cout<<"\n sto_dest: "<<instruction2reg[sto_dest]<<std::endl;
    }
    //risc_v_code += "  sw t0, " + instruction2reg[sto_dest] + "(sp)\n";
    //instruction2reg[value] = "t0";
    risc_v_code += "  sw " + instruction2reg[sto_value] + ", " + instruction2reg[sto_dest] + "(sp)\n";
    release_reg(instruction2reg[sto_value]);
    instruction2reg[value] = instruction2reg[sto_dest];
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
    else if(kind.tag==KOOPA_RVT_ALLOC){
        instruction2reg[value] = std::to_string(st);
        st+=4;
        std::cout<<"stack pointer: "<<st<<std::endl;
    }else if(kind.tag==KOOPA_RVT_LOAD){ 
        generate_code(kind.data.load, value);
    }else if(kind.tag==KOOPA_RVT_STORE){
        generate_code(kind.data.store, value);
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

        if(op==KOOPA_RBO_ADD&&lreg=="x0"){
            instruction2reg[value] = rreg;
            return;
        }

        std::string result_reg;
        switch (op) {
            case KOOPA_RBO_ADD:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  add " + result_reg + ", " + rreg + ", " + lreg + "\n";
                break;
            case KOOPA_RBO_SUB:
                risc_v_code += "  sub " + rreg + ", " + lreg + ", " + rreg + "\n";
                instruction2reg[value] = rreg;
                release_reg(lreg);
                break;
            case KOOPA_RBO_MUL:
                risc_v_code += "  mul " + rreg + ", " + rreg + ", " + lreg + "\n";
                instruction2reg[value] = rreg;
                release_reg(lreg);
                break;
            case KOOPA_RBO_DIV:
                risc_v_code += "  div " + rreg + ", " + lreg + ", " + rreg + "\n";
                instruction2reg[value] = rreg;
                release_reg(lreg);
                break;
            case KOOPA_RBO_MOD:
                risc_v_code += "  rem " + rreg + ", " + lreg + ", " + rreg + "\n";
                instruction2reg[value] = rreg;
                release_reg(lreg);
                break;
            case KOOPA_RBO_EQ:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  xor " + result_reg + ", " + rreg + ", " + lreg + "\n";  // 使用异或比较两寄存器
                risc_v_code += "  seqz " + result_reg + ", " + result_reg + "\n";          // 检查异或结果是否为0，从而判断两寄存器内容是否相等
                break;
            case KOOPA_RBO_NOT_EQ:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  xor " + result_reg + ", " + rreg + ", " + lreg + "\n";  // 使用异或比较两寄存器
                risc_v_code += "  snez " + result_reg + ", " + result_reg + "\n";          // 检查异或结果是否不为0，从而判断两寄存器内容是否不相等
                break;
            case KOOPA_RBO_OR:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  or " + result_reg + ", " + rreg + ", " + lreg + "\n";
                break;
            case KOOPA_RBO_AND:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  and " + result_reg + ", " + rreg + ", " + lreg + "\n";
                break;
            case KOOPA_RBO_LT:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  slt " + result_reg + ", " + lreg + ", " + rreg + "\n";
                break;
            case KOOPA_RBO_GT:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  sgt " + result_reg + ", " + lreg + ", " + rreg + "\n";
                break;
            case KOOPA_RBO_LE:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  slt " + result_reg + ", " + rreg + ", " + lreg + "\n";  // 使用 slt 判断 rreg < lreg
                risc_v_code += "  xori " + result_reg + ", " + result_reg + ", 1\n";      // 取反结果
                break;
            case KOOPA_RBO_GE:
                result_reg = binary_get_reg_release_unused_reg(lreg, rreg);
                instruction2reg[value] = result_reg;
                risc_v_code += "  slt " + result_reg + ", " + lreg + ", " + rreg + "\n";
                risc_v_code += "  xori " + result_reg + ", " + result_reg + ", 1\n";
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