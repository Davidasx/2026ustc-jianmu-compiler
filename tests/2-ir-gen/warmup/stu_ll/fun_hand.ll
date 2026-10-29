define i32 @callee(i32 %op0) {
label_entry:
  %op1 = alloca i32
  store i32 %op0, i32* %op1
  %op2 = load i32, i32* %op1
  %op3 = mul i32 2, %op2
  ret i32 %op3
}

define i32 @main() {
label_entry:
  %op0 = alloca i32
  %op1 = call i32 @callee(i32 110)
  ret i32 %op1
}
