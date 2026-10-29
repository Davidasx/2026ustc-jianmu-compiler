#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "IRBuilder.hpp"
#include "Module.hpp"
#include "Type.hpp"

#include <iostream>

#define CONST_INT(num) ConstantInt::get(num, module)
#define CONST_FP(num) ConstantFP::get(num, module)

// int main() {
//     float a = 5.555;
//     if (a > 1)
//         return 233;
//     return 0;
// }
int main() {
    auto module = new Module();
    auto builder = new IRBuilder(nullptr, module);

    Type *Int32Type = module->get_int32_type();
    Type *FloatType = module->get_float_type();

    auto mainFunTy = FunctionType::get(Int32Type, {});
    auto mainFun = Function::create(mainFunTy, "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    auto aAlloca = builder->create_alloca(FloatType);
    builder->create_store(CONST_FP(5.555), aAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto cmp = builder->create_fcmp_gt(aLoad, CONST_FP(1.0));

    auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
    auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
    builder->create_cond_br(cmp, trueBB, falseBB);

    builder->set_insert_point(trueBB);
    builder->create_ret(CONST_INT(233));

    builder->set_insert_point(falseBB);
    builder->create_ret(CONST_INT(0));

    std::cout << module->print();
    delete module;
    return 0;
}
