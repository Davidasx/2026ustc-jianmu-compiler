#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Function.hpp"
#include "IRBuilder.hpp"
#include "Module.hpp"
#include "Type.hpp"

#include <iostream>

#define CONST_INT(num) ConstantInt::get(num, module)

// int callee(int a) { return 2 * a; }
// int main() { return callee(110); }
int main() {
    auto module = new Module();
    auto builder = new IRBuilder(nullptr, module);

    Type *Int32Type = module->get_int32_type();

    std::vector<Type *> Ints(1, Int32Type);
    auto calleeFunTy = FunctionType::get(Int32Type, Ints);
    auto calleeFun = Function::create(calleeFunTy, "callee", module);
    auto bb = BasicBlock::create(module, "entry", calleeFun);
    builder->set_insert_point(bb);

    auto aAlloca = builder->create_alloca(Int32Type);
    auto args = std::vector<Value *>();
    for (auto &arg : calleeFun->get_args())
        args.push_back(&arg);
    builder->create_store(args[0], aAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto mul = builder->create_imul(CONST_INT(2), aLoad);
    builder->create_ret(mul);

    auto mainFunTy = FunctionType::get(Int32Type, {});
    auto mainFun = Function::create(mainFunTy, "main", module);
    builder->set_insert_point(BasicBlock::create(module, "entry", mainFun));

    auto call = builder->create_call(calleeFun, {CONST_INT(110)});
    builder->create_ret(call);

    std::cout << module->print();
    delete module;
    return 0;
}
