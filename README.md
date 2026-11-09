# 2026 USTC JIANMU 编译器

在课程提供的框架上实现 Cminusf 词法、语法分析、AST 转换和 Light IR 自动生成。

## Lab2 状态

- Phase1：四个手写 IR 和四个 Light IR C++ 生成器。
- Phase2：变量、作用域、数值与关系运算、类型转换、数组及负下标检查、分支和循环。
- Phase3：同一 IR builder 支持递归、数组参数转传、连续赋值和提前返回；全部公开测试通过。
- 保留 Lab1 实现；采用上游 `8ca8504` 的加固版评测脚本。

公开评测为 81/81 样例、100/100 分：lv0_1 为 17/17，lv0_2 为 18/18，lv1 为 31/31，lv2 为 24/24，lv3 为 10/10。公开程序的 LLVM verifier 检查 81/81，自定义语义回归 10/10。公开测试通过不表示已证明所有合法程序都正确。

## 构建

要求 CMake、Flex、Bison、现代 C++ 编译器和 LLVM 14 开发库。下面的 `llvm-config`、`clang`、`lli`、`opt` 应指向 LLVM 14；C++ 编译器仍可使用系统版本。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DLLVM_DIR="$(llvm-config --cmakedir)"
cmake --build build -j4
```

## 测试

```bash
# 官方完整评测，生成 tests/2-ir-gen/autogen/eval_result。
bash tests/2-ir-gen/autogen/eval_lab2.sh

# IR 校验与额外语义回归；临时文件写入指定目录。
python3 tests/2-ir-gen/phase2_regression.py \
  --llvm-bin /绝对路径/llvm-14/bin --tmpdir /指定临时目录

# Lab1 回归，应分别为 40/40、34/34。
bash tests/1-parser/eval_phase1.sh -all yes
bash tests/1-parser/eval_phase2.sh -all yes
```

额外测试覆盖赋值求值顺序、转换后的赋值结果、作用域、嵌套返回、负下标阻止 RHS 副作用、浮点索引截断、浮点条件、NaN 比较、递归数组转传和循环局部分配。

## 生成 IR

```bash
./build/cminusfc tests/testcases_general/1-return.cminus \
  -emit-llvm -o /指定输出目录/return.ll
```

报告及按姓名学号命名的 PDF、压缩包由课程目录外层的 Lab2 打包脚本管理；平台提交仓库的 `main` 分支，附件按课程实际要求上传。
