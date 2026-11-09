#include "cminusf_builder.hpp"

#define CONST_FP(num) ConstantFP::get((float)num, module.get())
#define CONST_INT(num) ConstantInt::get(num, module.get())

Type *CminusfBuilder::ir_type(CminusType type) {
    switch (type) {
    case TYPE_INT:
        return module->get_int32_type();
    case TYPE_FLOAT:
        return module->get_float_type();
    case TYPE_VOID:
        return module->get_void_type();
    }
    assert(false && "Unknown Cminusf type");
    return nullptr;
}

Value *CminusfBuilder::convert(Value *value, Type *type) {
    auto *from = value->get_type();
    if (from == type)
        return value;
    if (from->is_int1_type()) {
        value = builder->create_zext(value, module->get_int32_type());
        from = value->get_type();
        if (from == type)
            return value;
    }
    if (from->is_integer_type() && type->is_float_type())
        return builder->create_sitofp(value, type);
    if (from->is_float_type() && type->is_int32_type())
        return builder->create_fptosi(value, type);
    assert(false && "Unsupported implicit conversion");
    return nullptr;
}

Value *CminusfBuilder::condition(Value *value) {
    if (value->get_type()->is_int1_type())
        return value;
    if (value->get_type()->is_float_type())
        return builder->create_fcmp_ne(value, CONST_FP(0));
    return builder->create_icmp_ne(value, CONST_INT(0));
}

void CminusfBuilder::promote(Value *&left, Value *&right) {
    auto *type = left->get_type()->is_float_type() ||
                         right->get_type()->is_float_type()
                     ? module->get_float_type()
                     : static_cast<Type *>(module->get_int32_type());
    left = convert(left, type);
    right = convert(right, type);
}

AllocaInst *CminusfBuilder::allocate_local(Type *type) {
    // Allocate in the entry block even for declarations nested in loops.
    return AllocaInst::create_alloca_begin(type, context.func->get_entry_block());
}

void CminusfBuilder::default_return() {
    auto *type = context.func->get_return_type();
    if (type->is_void_type())
        builder->create_void_ret();
    else if (type->is_float_type())
        builder->create_ret(CONST_FP(0));
    else
        builder->create_ret(CONST_INT(0));
}

BasicBlock *CminusfBuilder::new_block(const std::string &name) {
    // Explicit names are not uniquified by Light IR's printer.
    return BasicBlock::create(module.get(),
                              name + std::to_string(context.block_id++),
                              context.func);
}

Value *CminusfBuilder::visit(ASTProgram &node) {
    for (auto &decl : node.declarations)
        decl->accept(*this);
    return nullptr;
}

Value *CminusfBuilder::visit(ASTNum &node) {
    if (node.type == TYPE_FLOAT)
        return CONST_FP(node.f_val);
    return CONST_INT(node.i_val);
}

Value *CminusfBuilder::visit(ASTVarDeclaration &node) {
    auto *type = ir_type(node.type);
    if (node.num)
        type = ArrayType::get(type, node.num->i_val);

    Value *address;
    if (scope.in_global()) {
        address = GlobalVariable::create(node.id, module.get(), type, false,
                                         ConstantZero::get(type, module.get()));
    } else {
        address = allocate_local(type);
    }
    bool inserted = scope.push(node.id, address);
    assert(inserted && "Duplicate declaration in one scope");
    return address;
}

Value *CminusfBuilder::visit(ASTFunDeclaration &node) {
    std::vector<Type *> param_types;
    for (auto &param : node.params) {
        auto *type = ir_type(param->type);
        if (param->isarray)
            type = PointerType::get(type);
        param_types.push_back(type);
    }

    auto *fun_type = FunctionType::get(ir_type(node.type), param_types);
    auto *func = Function::create(fun_type, node.id, module.get());
    bool inserted = scope.push(node.id, func);
    assert(inserted && "Duplicate function declaration");
    context.func = func;
    context.block_id = 0;
    builder->set_insert_point(new_block("entry"));
    scope.enter();

    auto arg = func->get_args().begin();
    for (auto &param : node.params) {
        auto *address = param->accept(*this);
        builder->create_store(&*arg, address);
        ++arg;
    }

    context.function_body = true;
    node.compound_stmt->accept(*this);
    if (!builder->get_insert_block()->is_terminated())
        default_return();
    scope.exit();
    context.func = nullptr;
    return nullptr;
}

Value *CminusfBuilder::visit(ASTParam &node) {
    auto *type = ir_type(node.type);
    if (node.isarray)
        type = PointerType::get(type);
    auto *address = allocate_local(type);
    bool inserted = scope.push(node.id, address);
    assert(inserted && "Duplicate parameter declaration");
    return address;
}

Value *CminusfBuilder::visit(ASTCompoundStmt &node) {
    bool nested = !context.function_body;
    context.function_body = false;
    if (nested)
        scope.enter();

    for (auto &decl : node.local_declarations)
        decl->accept(*this);
    for (auto &stmt : node.statement_list) {
        if (builder->get_insert_block()->is_terminated())
            break;
        stmt->accept(*this);
    }

    if (nested)
        scope.exit();
    return nullptr;
}

Value *CminusfBuilder::visit(ASTExpressionStmt &node) {
    return node.expression ? node.expression->accept(*this) : nullptr;
}

Value *CminusfBuilder::visit(ASTSelectionStmt &node) {
    auto *cond = condition(node.expression->accept(*this));
    auto *then_bb = new_block("if_then");
    auto *else_bb = new_block("if_else");
    builder->create_cond_br(cond, then_bb, else_bb);

    builder->set_insert_point(then_bb);
    node.if_statement->accept(*this);
    auto *then_end = builder->get_insert_block();

    builder->set_insert_point(else_bb);
    if (node.else_statement)
        node.else_statement->accept(*this);
    auto *else_end = builder->get_insert_block();

    // No join block is needed if both branches return.
    if (then_end->is_terminated() && else_end->is_terminated())
        return nullptr;
    auto *end_bb = new_block("if_end");
    if (!then_end->is_terminated()) {
        builder->set_insert_point(then_end);
        builder->create_br(end_bb);
    }
    if (!else_end->is_terminated()) {
        builder->set_insert_point(else_end);
        builder->create_br(end_bb);
    }
    builder->set_insert_point(end_bb);
    return nullptr;
}

Value *CminusfBuilder::visit(ASTIterationStmt &node) {
    auto *cond_bb = new_block("while_cond");
    auto *body_bb = new_block("while_body");
    auto *end_bb = new_block("while_end");
    builder->create_br(cond_bb);

    builder->set_insert_point(cond_bb);
    auto *cond = condition(node.expression->accept(*this));
    builder->create_cond_br(cond, body_bb, end_bb);

    builder->set_insert_point(body_bb);
    node.statement->accept(*this);
    if (!builder->get_insert_block()->is_terminated())
        builder->create_br(cond_bb);
    builder->set_insert_point(end_bb);
    return nullptr;
}

Value *CminusfBuilder::visit(ASTReturnStmt &node) {
    if (!node.expression) {
        builder->create_void_ret();
    } else {
        auto *value = node.expression->accept(*this);
        builder->create_ret(convert(value, context.func->get_return_type()));
    }
    return nullptr;
}

Value *CminusfBuilder::var_address(ASTVar &node) {
    auto *address = scope.find(node.id);
    if (!node.expression)
        return address;

    // Cminusf evaluates the index before the assignment's right operand.
    auto *index = convert(node.expression->accept(*this), module->get_int32_type());
    auto *negative = builder->create_icmp_lt(index, CONST_INT(0));
    auto *error_bb = new_block("index_error");
    auto *valid_bb = new_block("index_valid");
    builder->create_cond_br(negative, error_bb, valid_bb);

    builder->set_insert_point(error_bb);
    builder->create_call(scope.find("neg_idx_except"), {});
    // The runtime exits, but the IR still requires a well-typed terminator.
    default_return();
    builder->set_insert_point(valid_bb);

    auto *element = address->get_type()->get_pointer_element_type();
    if (element->is_array_type())
        return builder->create_gep(address, {CONST_INT(0), index});
    // An array parameter has an alloca of T*, rather than an alloca of [N x T].
    assert(element->is_pointer_type() && "Subscript applied to a scalar");
    return builder->create_gep(builder->create_load(address), {index});
}

Value *CminusfBuilder::visit(ASTVar &node) {
    auto *address = var_address(node);
    auto *element = address->get_type()->get_pointer_element_type();
    if (element->is_array_type())
        return builder->create_gep(address, {CONST_INT(0), CONST_INT(0)});
    return builder->create_load(address);
}

Value *CminusfBuilder::visit(ASTAssignExpression &node) {
    auto *address = var_address(*node.var);
    auto *value = node.expression->accept(*this);
    value = convert(value, address->get_type()->get_pointer_element_type());
    builder->create_store(value, address);
    return value;
}

Value *CminusfBuilder::visit(ASTSimpleExpression &node) {
    auto *left = node.additive_expression_l->accept(*this);
    if (!node.additive_expression_r)
        return left;
    auto *right = node.additive_expression_r->accept(*this);
    promote(left, right);

    Value *result = nullptr;
    if (left->get_type()->is_float_type()) {
        // The library prints unordered float predicates. Invert the opposite
        // predicate to obtain ordered comparisons, including the NaN case.
        bool invert = true;
        switch (node.op) {
        case OP_LE:  result = builder->create_fcmp_gt(left, right); break;
        case OP_LT:  result = builder->create_fcmp_ge(left, right); break;
        case OP_GT:  result = builder->create_fcmp_le(left, right); break;
        case OP_GE:  result = builder->create_fcmp_lt(left, right); break;
        case OP_EQ:  result = builder->create_fcmp_ne(left, right); break;
        case OP_NEQ: result = builder->create_fcmp_ne(left, right); invert = false; break;
        }
        if (invert) {
            auto *integer = builder->create_zext(result, module->get_int32_type());
            result = builder->create_icmp_eq(integer, CONST_INT(0));
        }
    } else {
        switch (node.op) {
        case OP_LE:  result = builder->create_icmp_le(left, right); break;
        case OP_LT:  result = builder->create_icmp_lt(left, right); break;
        case OP_GT:  result = builder->create_icmp_gt(left, right); break;
        case OP_GE:  result = builder->create_icmp_ge(left, right); break;
        case OP_EQ:  result = builder->create_icmp_eq(left, right); break;
        case OP_NEQ: result = builder->create_icmp_ne(left, right); break;
        }
    }
    // A relational expression has Cminusf type int (0 or 1), not i1.
    return builder->create_zext(result, module->get_int32_type());
}

Value *CminusfBuilder::visit(ASTAdditiveExpression &node) {
    if (!node.additive_expression)
        return node.term->accept(*this);
    auto *left = node.additive_expression->accept(*this);
    auto *right = node.term->accept(*this);
    promote(left, right);
    if (left->get_type()->is_float_type()) {
        if (node.op == OP_PLUS)
            return builder->create_fadd(left, right);
        return builder->create_fsub(left, right);
    }
    if (node.op == OP_PLUS)
        return builder->create_iadd(left, right);
    return builder->create_isub(left, right);
}

Value *CminusfBuilder::visit(ASTTerm &node) {
    if (!node.term)
        return node.factor->accept(*this);
    auto *left = node.term->accept(*this);
    auto *right = node.factor->accept(*this);
    promote(left, right);
    if (left->get_type()->is_float_type()) {
        if (node.op == OP_MUL)
            return builder->create_fmul(left, right);
        return builder->create_fdiv(left, right);
    }
    if (node.op == OP_MUL)
        return builder->create_imul(left, right);
    return builder->create_isdiv(left, right);
}

Value *CminusfBuilder::visit(ASTCall &node) {
    auto *func = static_cast<Function *>(scope.find(node.id));
    auto *type = func->get_function_type();
    assert(node.args.size() == type->get_num_of_args() && "Wrong argument count");
    std::vector<Value *> args;
    for (unsigned i = 0; i < node.args.size(); ++i) {
        auto *value = node.args[i]->accept(*this);
        args.push_back(convert(value, type->get_param_type(i)));
    }
    return builder->create_call(func, args);
}
