#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "IRBuilder.hpp"
#include "Module.hpp"
#include "Type.hpp"

#include <iostream>

#define CONST_INT(num) ConstantInt::get(num, module)

// int main() {
//     int a;
//     int i;
//     a = 10;
//     i = 0;
//     while (i < 10) {
//         i = i + 1;
//         a = a + i;
//     }
//     return a;
// }
int main() {
    auto module = new Module();
    auto builder = new IRBuilder(nullptr, module);

    Type *Int32Type = module->get_int32_type();

    auto mainFunTy = FunctionType::get(Int32Type, {});
    auto mainFun = Function::create(mainFunTy, "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    auto aAlloca = builder->create_alloca(Int32Type);
    auto iAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(10), aAlloca);
    builder->create_store(CONST_INT(0), iAlloca);

    auto condBB = BasicBlock::create(module, "condBB", mainFun);
    auto bodyBB = BasicBlock::create(module, "bodyBB", mainFun);
    auto endBB = BasicBlock::create(module, "endBB", mainFun);
    builder->create_br(condBB);

    builder->set_insert_point(condBB);
    auto iLoad = builder->create_load(iAlloca);
    auto cmp = builder->create_icmp_lt(iLoad, CONST_INT(10));
    builder->create_cond_br(cmp, bodyBB, endBB);

    builder->set_insert_point(bodyBB);
    auto iLoad2 = builder->create_load(iAlloca);
    auto iNext = builder->create_iadd(iLoad2, CONST_INT(1));
    builder->create_store(iNext, iAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto iLoad3 = builder->create_load(iAlloca);
    auto aNext = builder->create_iadd(aLoad, iLoad3);
    builder->create_store(aNext, aAlloca);
    builder->create_br(condBB);

    builder->set_insert_point(endBB);
    auto aRet = builder->create_load(aAlloca);
    builder->create_ret(aRet);

    std::cout << module->print();
    delete module;
    return 0;
}
