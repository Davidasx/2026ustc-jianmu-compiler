# 简介

本仓库为 USTC 编译原理和技术 2026 的课程实验仓库。在本学期的编译实验中，你们将构建一个从词法分析器开始到后端代码生成的JIANMU编译器。

你们需要 fork 此 repo 到自己的仓库下，随后在自己的仓库中完成实验。


## 测试脚本使用方法

eval_phase1.sh: 测试phase1 词法分析器+语法分析器部分

    -all: 测试所有测试集 (easy, normal, hard, testcases_general)

    easy/normal/hard/testcases_general + yes/no/verbose: 测试单个测试集

        yes: 使用diff判断正确性

        no: 不使用diff判断正确性

        verbose: 使用diff判断正确性，并看到更详细的输出

eval_phase2.sh: 测试phase2 syntax_tree -> ast 部分

    使用方法与eval_phase1.sh完全相同, 会自动跳过生成syntax_tree fail的样例
    
