#include "koopa.h"
#include <iostream>
#include <string>
#include <cassert>
#include <unordered_map>
#include <unordered_set>

void Koopa_IR2RISC_V(const char *str);
void generate_program_code(const koopa_raw_program_t &raw);
void generate_slice_code(const koopa_raw_slice_t &slice);
void generate_function_code(const koopa_raw_function_t &func);
void generate_block_code(const koopa_raw_basic_block_t &block);
void generate_value_code(const koopa_raw_value_t &value);
void generate_global_code(const koopa_raw_program_t &raw);

void generate_interger_code(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);
void generate_return_code(const koopa_raw_return_t &ret);
void generate_store_code(const koopa_raw_store_t &store);
void generate_branch_code(const koopa_raw_branch_t &branch);
void generate_load_code(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void generate_call_code(const koopa_raw_call_t &call, const koopa_raw_value_t &value);

void load_value_to_reg(const koopa_raw_value_t &value, const std::string &reg);
int get_function_st(const koopa_raw_function_t &func);
int get_value_st(const koopa_raw_value_t &value);

int get_addr(const koopa_raw_value_t &value, const char* file, const char* func, int line);


std::unordered_set<std::string> global_func_set({"getint", "getch", "getarray" ,"putint", "putch", "putarray", "putf", "starttime", "stoptime"});


int st; // 当前栈帧指针
int func_st; // 当前函数栈帧指针


std::unordered_map<koopa_raw_value_t, int> value2addr;

#define GET_ADDR(value) get_addr(value, __FILE__, __FUNCTION__, __LINE__)


int get_addr(const koopa_raw_value_t &value, const char* file, const char* func, int line) {
    if (value2addr.find(value) == value2addr.end()) {
        std::cout << "Not found value tag: " << value->kind.tag << " in " << file << " " << func << " line " << line << std::endl;
        return -1;
    }
    return value2addr[value];
}

void generate_global_code(const koopa_raw_program_t &raw){
    if(raw.values.len)
        {
            std::cout<<"   .data"<<std::endl;
            for(size_t i=0;i<raw.values.len;++i)
            {
                koopa_raw_value_t data=(koopa_raw_value_t)raw.values.buffer[i];
                std::cout<<"   .globl "<<data->name+1<<std::endl;
                std::cout<<data->name+1<<":"<<std::endl;
                if(data->kind.data.global_alloc.init->kind.tag==KOOPA_RVT_INTEGER)
                {
                    std::cout<<"   .word "<<data->kind.data.global_alloc.init->kind.data.integer.value<<std::endl;
                    std::cout<<std::endl;
                }
                else if(data->kind.data.global_alloc.init->kind.tag==KOOPA_RVT_ZERO_INIT)
                {
                    koopa_raw_type_t value=data->ty->data.pointer.base;
                    int siz=4;
                    std::cout<<"   .zero "<<siz<<std::endl;
                }
            }
        }
}

void Koopa_IR2RISC_V(const char *str) {
    // std::cout<<"hello world\n";
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);

    assert(ret == KOOPA_EC_SUCCESS);

    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    generate_global_code(raw);

    // generate_program_code(raw);
    for(size_t i=0;i<raw.funcs.len;++i){
        assert(raw.funcs.kind==KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];

        func_st = get_function_st(func);
        if(global_func_set.count(func->name+1)) continue;
        std::cout<< "  .text\n";
        std::cout<< "  .globl "<< func->name+1 <<"\n";
        std::cout<< func->name+1 <<":\n";
        std::cout<< "  addi sp, sp, -" << func_st << "\n";
        st = func_st;
        //st = 256; // 栈帧重置为0
        for(size_t j=0;j<func->bbs.len;++j){
            assert(func->bbs.kind==KOOPA_RSIK_BASIC_BLOCK);
            koopa_raw_basic_block_t block = (koopa_raw_basic_block_t) func->bbs.buffer[j];
            if(block->name!=nullptr){
                std::string bb_name = block->name;
                if(bb_name!="%entry"){
                    std::cout<< bb_name.substr(1) << ":\n";
                }
            }
            for(size_t k=0;k<block->insts.len;++k){
                assert(block->insts.kind==KOOPA_RSIK_VALUE);
                koopa_raw_value_t value = (koopa_raw_value_t) block->insts.buffer[k];
                generate_value_code(value);
            }
        }
    }

    koopa_delete_raw_program_builder(builder);

}

void generate_program_code(const koopa_raw_program_t &raw) {
    generate_slice_code(raw.funcs);
}

void generate_function_code(const koopa_raw_function_t &func) {
    std::cout << "Function: \n";
    generate_slice_code(func->bbs);
}

void generate_block_code(const koopa_raw_basic_block_t &block) {
    std::cout << "Block: \n";
    generate_slice_code(block->insts);
}

void generate_slice_code(const koopa_raw_slice_t &slice) {
    for (size_t i = 0; i < slice.len; ++i) {
        auto ptr = slice.buffer[i];
        switch (slice.kind) {
            case KOOPA_RSIK_FUNCTION:
                std::cout << "KOOPA_RSIK_FUNCTION\n";
                generate_function_code(reinterpret_cast<koopa_raw_function_t>(ptr));
                break;
            case KOOPA_RSIK_BASIC_BLOCK:
                std::cout << "KOOPA_RSIK_BASIC_BLOCK\n";
                generate_block_code(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
                break;
            case KOOPA_RSIK_VALUE:
                std::cout << "KOOPA_RSIK_VALUE\n";
                break;
                generate_value_code(reinterpret_cast<koopa_raw_value_t>(ptr));
                std::cout<<" back\n";
                break;
            default:
                std::cout << "Error: Unknown kind of slice\n";
        }
    }
}
void generate_return_code(const koopa_raw_return_t &ret) {
    if(ret.value){
        //generate_value_code(ret.value);
        load_value_to_reg(ret.value, "a0");
    }
    else{
        std::cout<< "  li a0, 0\n";
    }
    std::cout<< "  addi sp, sp, "<< func_st << "\n";
    std::cout<< "  ret\n\n";
}

void generate_interger_code(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value) {
    std::cout<< "  li t0, " + std::to_string(integer.value) + "\n";
    std::cout<< "  sw t0, " << std::to_string(st) << "(sp)\n\n";
    value2addr[value] = st;
    st -= 4;
}

void generate_store_code(const koopa_raw_store_t &store) {
    koopa_raw_value_t sto_value = store.value;
    koopa_raw_value_t sto_dest = store.dest;
    if(sto_value->kind.tag == KOOPA_RVT_INTEGER){
        std::cout<< "  li t0, " + std::to_string(sto_value->kind.data.integer.value) + "\n";
    } else if(sto_value->kind.tag == KOOPA_RVT_BINARY){
        std::cout<< "  lw t0, " << std::to_string(GET_ADDR(sto_value)) << "(sp)\n";
    } else if(sto_value->kind.tag == KOOPA_RVT_LOAD){
        std::cout<< "  lw t0, " << std::to_string(GET_ADDR(sto_value)) << "(sp)\n";
    } else if(sto_value->kind.tag == KOOPA_RVT_FUNC_ARG_REF){
        koopa_raw_func_arg_ref_t arg = sto_value->kind.data.func_arg_ref;
        if(arg.index<8){
            std::cout<< "  mv t0, a" << arg.index << "\n";
        }else{
            std::cout<< "  lw t0, " << ((arg.index-8)*4) << "(sp)\n";
        }
    } else if(sto_value->kind.tag == KOOPA_RVT_CALL){
        std::cout<< "  lw t0, " << std::to_string(GET_ADDR(sto_value)) << "(sp)\n";
    }
    else{
        std::cout<< "Error: Unknown value kind of store\n"<<sto_value->kind.tag<<std::endl;
    }

    if(sto_dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        std::cout<<"   la t1, "<<sto_dest->name+1<<std::endl;
        std::cout<<"   sw t0, 0(t1)"<<std::endl;
    }else{
        //std::cout<<"sto_dest tag "<<sto_dest->kind.tag<<std::endl;
        std::cout<< "  sw t0, " << GET_ADDR(sto_dest) << "(sp)\n";
    }
}

void generate_branch_code(const koopa_raw_branch_t &branch) {
  // 访问 branch 指令
  // ...
    koopa_raw_value_t branch_cond = branch.cond;
    koopa_raw_basic_block_t true_block = branch.true_bb;
    koopa_raw_basic_block_t false_block = branch.false_bb;

    generate_value_code(branch_cond);

    int addr = GET_ADDR(branch_cond);

    std::cout<< "  lw t0, " << addr << "(sp)\n";

    std::cout<< "  bnez t0, " << true_block->name+1 << "\n";
    std::cout<< "  j " << false_block->name+1 << "\n\n";
}

void generate_jump_code(const koopa_raw_jump_t &jump) {
    // 访问 jump 指令
    // ...
    koopa_raw_basic_block_t target_block = jump.target;
    std::cout<< "  j " << target_block->name+1 << "\n\n";
}

void generate_call_code(const koopa_raw_call_t &call, const koopa_raw_value_t &value) {
    // 访问 call 指令
    // ...
    int return_addr = st;
    std::cout<<"  sw ra, " << return_addr << "(sp)\n";
    st -= 4;

    int nowst = 0;
    auto args = call.args;
    auto func = call.callee;

    int callee_st = get_function_st(func);
    // std::cout<<"  args len: "<<args.len<<std::endl;
    for(int i=1;i<=args.len;i++){
        auto ptr = args.buffer[i-1];
        auto arg = reinterpret_cast<koopa_raw_value_t>(ptr);
        generate_value_code(arg);
        if(i<=8){
            std::cout<< "  lw a" << i-1 << ", " << GET_ADDR(arg) << "(sp)\n";
        }else{
            std::cout<< "  lw t0, " << GET_ADDR(arg) << "(sp)\n";
            std::cout<< "  sw t0, " << (i-9)*4 - callee_st << "(sp)\n";
            nowst += 4;
        }
    }

    std::cout<< "  call " << func->name+1 << "\n";
    value2addr[value] = st;
    st -= 4;
    std::cout<< "  sw a0, " << value2addr[value] << "(sp)\n\n";

    std::cout<< "  lw ra, " << return_addr << "(sp)\n";
}

void generate_load_code(const koopa_raw_load_t &load, const koopa_raw_value_t &value) {
    // 访问 load 指令
    // ...
    koopa_raw_value_t loa_src = load.src;
    //std::string reg = "t0";
    if(loa_src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC){
        std::cout<<"   la t0, "<<value->kind.data.load.src->name+1<<std::endl;
        std::cout<<"   lw t0, 0(t0)"<<std::endl;
        std::cout<<"   li t4, "<<st<<std::endl;
        std::cout<<"   add t4, t4, sp"<<std::endl;
        std::cout<<"   sw t0, (t4)"<<std::endl;
        value2addr[value] = st;
        st -= 4;
    }else{
        std::cout<< "  lw t0, " << GET_ADDR(loa_src) << "(sp)\n";
        std::cout<< "  sw t0, " << std::to_string(st) << "(sp)\n\n";
        value2addr[value] = st;
        st -= 4;
    }
}

void load_value_to_reg(const koopa_raw_value_t &value, const std::string &reg){
    if(value->kind.tag==KOOPA_RVT_INTEGER){
        std::cout<< "  li " << reg << ", " << std::to_string(value->kind.data.integer.value) << "\n";
    }
    else if(value->kind.tag==KOOPA_RVT_BINARY){
        int addr = GET_ADDR(value);
        std::cout<< "  lw " << reg << ", " << std::to_string(addr) << "(sp)\n";
    }
    else if(value->kind.tag==KOOPA_RVT_LOAD){
        int addr = GET_ADDR(value);
        std::cout<< "  lw " << reg << ", " << std::to_string(addr) << "(sp)\n";
    }
    else if(value->kind.tag==KOOPA_RVT_CALL){
        int addr = GET_ADDR(value);
        std::cout<< "  lw " << reg << ", " << std::to_string(addr) << "(sp)\n";
    }
    else{
        std::cout<< "Error: Unknown kind of value\n";
        std::cout<<value->kind.tag<<std::endl;
    }
}

int get_function_st(const koopa_raw_function_t &func){
    int func_st = 0;
    for(size_t i=0;i<func->bbs.len;++i){
        assert(func->bbs.kind==KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t block = (koopa_raw_basic_block_t) func->bbs.buffer[i];
        for(size_t j=0;j<block->insts.len;++j){
            assert(block->insts.kind==KOOPA_RSIK_VALUE);
            koopa_raw_value_t value = (koopa_raw_value_t) block->insts.buffer[j];
            func_st += get_value_st(value);
        }
    }
    func_st = ((func_st+15) /16) *16;
    return func_st;
}

int get_value_st(const koopa_raw_value_t &value){

    if(value->kind.tag==KOOPA_RVT_ALLOC||value->kind.tag==KOOPA_RVT_INTEGER||value->kind.tag==KOOPA_RVT_LOAD){
        return 4;
    }
    else if(value->kind.tag==KOOPA_RVT_BINARY){
        auto lhs = value->kind.data.binary.lhs;
        auto rhs = value->kind.data.binary.rhs;
        return get_value_st(lhs)+get_value_st(rhs)+4;
    }
    else if(value->kind.tag==KOOPA_RVT_CALL){
        auto args = value->kind.data.call.args;
        int value_st = 0;
        for(size_t i=0;i<args.len;++i){
            assert(args.kind==KOOPA_RSIK_VALUE);
            koopa_raw_value_t arg = (koopa_raw_value_t) args.buffer[i];
            value_st += get_value_st(arg);
        }
        return value_st+8;
    }
    else if(value->kind.tag==KOOPA_RVT_RETURN||value->kind.tag==KOOPA_RVT_STORE){
        return 0;
    }
    else{
        return 0;
    }
    return 0;
}

void generate_value_code(const koopa_raw_value_t &value) {
    if (!value) return;
    if(value2addr.find(value)!=value2addr.end()){
        return;
    }
    const auto &kind = value->kind;
    if (kind.tag == KOOPA_RVT_RETURN) {
        //std::cout << "KOOPA_RVT_RETURN\n";
        generate_return_code(kind.data.ret);
    } else if (kind.tag == KOOPA_RVT_INTEGER) {
        generate_interger_code(kind.data.integer, value);
    } else if (kind.tag == KOOPA_RVT_ALLOC) {
        //std::cout << "KOOPA_RVT_ALLOC\n";
        value2addr[value] = st;
        st -= 4;
    } else if (kind.tag == KOOPA_RVT_LOAD) {
        //std::cout << "KOOPA_RVT_LOAD\n";
        generate_load_code(kind.data.load, value);
    } else if (kind.tag == KOOPA_RVT_STORE) {
        //std::cout << "KOOPA_RVT_STORE\n";
        generate_store_code(kind.data.store);
    } else if (kind.tag == KOOPA_RVT_JUMP) {
        //std::cout << "KOOPA_RVT_JUMP\n";
        generate_jump_code(kind.data.jump);
    } else if (kind.tag == KOOPA_RVT_BRANCH) {
        //std::cout << "KOOPA_RVT_BRANCH\n";
        generate_branch_code(kind.data.branch);
    } else if (kind.tag == KOOPA_RVT_CALL) {
        //std::cout << "KOOPA_RVT_CALL\n";
        generate_call_code(kind.data.call, value);
    } else if (kind.tag == KOOPA_RVT_BINARY) {
        // std::cout << "KOOPA_RVT_BINARY\n";
        koopa_raw_binary_t binary = kind.data.binary;
        koopa_raw_value_t lhs = binary.lhs;
        koopa_raw_value_t rhs = binary.rhs;

        load_value_to_reg(lhs, "t0");
        load_value_to_reg(rhs, "t1");

        koopa_raw_binary_op_t op = binary.op;

        switch (op) {
            case KOOPA_RBO_ADD:
                std::cout<< "  add t0, t1, t0\n";
                break;
            case KOOPA_RBO_SUB:
                std::cout<< "  sub t0, t0, t1\n";
                break;
            case KOOPA_RBO_MUL:
                std::cout<< "  mul t0, t1, t0\n";
                break;
            case KOOPA_RBO_DIV:
                std::cout<< "  div t0, t0, t1\n";
                break;
            case KOOPA_RBO_MOD:
                std::cout<< "  rem t0, t0, t1\n";
                break;
            case KOOPA_RBO_EQ:
                std::cout<< "  xor t0, t1, t0\n";
                std::cout<< "  seqz t0, t0\n";
                break;
            case KOOPA_RBO_NOT_EQ:
                std::cout<< "  xor t0, t1, t0\n";
                std::cout<< "  snez t0, t0\n";
                break;
            case KOOPA_RBO_OR:
                std::cout<< "  or t0, t1, t0\n";
                break;
            case KOOPA_RBO_AND:
                std::cout<< "  and t0, t1, t0\n";
                break;
            case KOOPA_RBO_LT:
                std::cout<< "  slt t0, t0, t1\n";
                break;
            case KOOPA_RBO_GT:
                std::cout<< "  sgt t0, t0, t1\n";
                break;
            case KOOPA_RBO_LE:
                std::cout<< "  slt t0, t1, t0\n";
                std::cout<< "  xori t0, t0, 1\n";
                break;
            case KOOPA_RBO_GE:
                std::cout<< "  slt t0, t0, t1\n";
                std::cout<< "  xori t0, t0, 1\n";
                break;
            default:
                std::cout << "Unhandled binary operation: " << op << std::endl;
        }
        std::cout<< "  sw t0, " << std::to_string(st) << "(sp)\n\n";
        value2addr[value] = st;
        st -= 4;
    } else {
        std::cout << "Unhandled instruction kind: " << kind.tag << std::endl;
    }
}
