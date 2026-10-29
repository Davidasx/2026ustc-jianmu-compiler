define i32 @main() {
label_entry:
  %op0 = alloca i32
  %op1 = alloca i32
  store i32 10, i32* %op0
  store i32 0, i32* %op1
  br label %label_cond
label_cond:                                       ; preds = %label_entry, %label_body
  %op2 = load i32, i32* %op1
  %op3 = icmp slt i32 %op2, 10
  br i1 %op3, label %label_body, label %label_end
label_body:                                       ; preds = %label_cond
  %op4 = load i32, i32* %op1
  %op5 = add i32 %op4, 1
  store i32 %op5, i32* %op1
  %op6 = load i32, i32* %op0
  %op7 = load i32, i32* %op1
  %op8 = add i32 %op6, %op7
  store i32 %op8, i32* %op0
  br label %label_cond
label_end:                                        ; preds = %label_cond
  %op9 = load i32, i32* %op0
  ret i32 %op9
}
