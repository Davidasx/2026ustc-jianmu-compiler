#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "IRBuilder.hpp"
#include "Module.hpp"
#include "Type.hpp"

#include <iostream>

#define CONST_INT(num) ConstantInt::get(num, module)

// int main() {
//     int a[10];
//     a[0] = 10;
//     a[1] = a[0] * 2;
//     return a[1];
// }
int main() {
    auto module = new Module();
    auto builder = new IRBuilder(nullptr, module);

    Type *Int32Type = module->get_int32_type();
    auto *arrayType = ArrayType::get(Int32Type, 10);

    auto mainFunTy = FunctionType::get(Int32Type, {});
    auto mainFun = Function::create(mainFunTy, "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    auto aAlloca = builder->create_alloca(arrayType);

    auto a0GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});
    builder->create_store(CONST_INT(10), a0GEP);

    auto a0LoadGEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(0)});
    auto a0Load = builder->create_load(a0LoadGEP);
    auto mul = builder->create_imul(a0Load, CONST_INT(2));

    auto a1GEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});
    builder->create_store(mul, a1GEP);

    auto a1LoadGEP = builder->create_gep(aAlloca, {CONST_INT(0), CONST_INT(1)});
    auto a1Load = builder->create_load(a1LoadGEP);
    builder->create_ret(a1Load);

    std::cout << module->print();
    delete module;
    return 0;
}
