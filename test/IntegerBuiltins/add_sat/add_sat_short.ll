
; RUN: clspv-opt -ReplaceOpenCLBuiltin %s -o %t.ll
; RUN: FileCheck %s < %t.ll

; AUTO-GENERATED TEST FILE
; This test was generated by add_sat_test_gen.cpp.
; Please modify the that file and regenerate the tests to make changes.

target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir-unknown-unknown"

define i16 @add_sat_short(i16 %a, i16 %b) {
entry:
 %call = call i16 @_Z7add_satss(i16 %a, i16 %b)
 ret i16 %call
}

declare i16 @_Z7add_satss(i16, i16)

; CHECK: [[sext_a:%[a-zA-Z0-9_.]+]] = sext i16 %a to i32
; CHECK: [[sext_b:%[a-zA-Z0-9_.]+]] = sext i16 %b to i32
; CHECK: [[add:%[a-zA-Z0-9_.]+]] = add nsw i32 [[sext_a]], [[sext_b]]
; CHECK: [[clamp:%[a-zA-Z0-9_.]+]] = call i32 @_Z5clampiii(i32 [[add]], i32 -32768, i32 32767)
; CHECK: [[trunc:%[a-zA-Z0-9_.]+]] = trunc i32 [[clamp]] to i16
; CHECK: ret i16 [[trunc]]
