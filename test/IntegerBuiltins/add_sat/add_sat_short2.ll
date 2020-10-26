
; RUN: clspv-opt -ReplaceOpenCLBuiltin %s -o %t.ll
; RUN: FileCheck %s < %t.ll

; AUTO-GENERATED TEST FILE
; This test was generated by add_sat_test_gen.cpp.
; Please modify the that file and regenerate the tests to make changes.

target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir-unknown-unknown"

define <2 x i16> @add_sat_short2(<2 x i16> %a, <2 x i16> %b) {
entry:
 %call = call <2 x i16> @_Z7add_satDv2_sS_(<2 x i16> %a, <2 x i16> %b)
 ret <2 x i16> %call
}

declare <2 x i16> @_Z7add_satDv2_sS_(<2 x i16>, <2 x i16>)

; CHECK: [[sext_a:%[a-zA-Z0-9_.]+]] = sext <2 x i16> %a to <2 x i32>
; CHECK: [[sext_b:%[a-zA-Z0-9_.]+]] = sext <2 x i16> %b to <2 x i32>
; CHECK: [[add:%[a-zA-Z0-9_.]+]] = add nsw <2 x i32> [[sext_a]], [[sext_b]]
; CHECK: [[clamp:%[a-zA-Z0-9_.]+]] = call <2 x i32> @_Z5clampDv2_iS_S_(<2 x i32> [[add]], <2 x i32> <i32 -32768, i32 -32768>, <2 x i32> <i32 32767, i32 32767>)
; CHECK: [[trunc:%[a-zA-Z0-9_.]+]] = trunc <2 x i32> [[clamp]] to <2 x i16>
; CHECK: ret <2 x i16> [[trunc]]
