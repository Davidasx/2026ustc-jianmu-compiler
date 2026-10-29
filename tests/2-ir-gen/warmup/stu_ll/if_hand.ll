define i32 @main() {
label_entry:
  %op0 = alloca float
  store float 0x40163851E0000000, float* %op0
  %op1 = load float, float* %op0
  %op2 = fcmp ogt float %op1, 1.000000e+00
  br i1 %op2, label %label_true, label %label_false
label_true:                                       ; preds = %label_entry
  ret i32 233
label_false:                                      ; preds = %label_entry
  ret i32 0
}
