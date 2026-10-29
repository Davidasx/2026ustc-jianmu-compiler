define i32 @main() {
label_entry:
  %op0 = alloca [10 x i32]
  %op1 = alloca i32
  %op2 = getelementptr [10 x i32], [10 x i32]* %op0, i32 0, i32 0
  store i32 10, i32* %op2
  %op3 = getelementptr [10 x i32], [10 x i32]* %op0, i32 0, i32 0
  %op4 = load i32, i32* %op3
  %op5 = mul i32 %op4, 2
  %op6 = getelementptr [10 x i32], [10 x i32]* %op0, i32 0, i32 1
  store i32 %op5, i32* %op6
  %op7 = getelementptr [10 x i32], [10 x i32]* %op0, i32 0, i32 1
  %op8 = load i32, i32* %op7
  ret i32 %op8
}
