// Copyright 2020 The Clspv Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const std::string preamble = R"(
; RUN: clspv-opt -ReplaceOpenCLBuiltin %s -o %t.ll
; RUN: FileCheck %s < %t.ll

; AUTO-GENERATED TEST FILE
; This test was generated by add_sat_test_gen.cpp.
; Please modify the that file and regenerate the tests to make changes.

target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir-unknown-unknown"
)";

std::string TypeName(uint32_t width, bool is_signed, uint32_t vector) {
  std::string name = (is_signed ? "" : "u");
  switch (width) {
  case 8:
    name += "char";
    break;
  case 16:
    name += "short";
    break;
  case 32:
    name += "int";
    break;
  case 64:
  default:
    name += "long";
    break;
  }

  if (vector > 1)
    name += std::to_string(vector);

  return name;
}

std::string Params(uint32_t width, bool is_signed, uint32_t vector,
                   uint32_t params) {
  std::string base;
  switch (width) {
  case 8:
    base = is_signed ? "c" : "h";
    break;
  case 16:
    base = is_signed ? "s" : "t";
    break;
  case 32:
    base = is_signed ? "i" : "j";
    break;
  case 64:
  default:
    base = is_signed ? "l" : "m";
    break;
  }

  if (vector == 1) {
    if (params == 1)
      return base;
    else if (params == 2)
      return base + base;
    else
      return base + base + base;
  }

  if (params == 1)
    return "Dv" + std::to_string(vector) + "_" + base;
  else if (params == 2)
    return "Dv" + std::to_string(vector) + "_" + base + "S_";
  else
    return "Dv" + std::to_string(vector) + "_" + base + "S_S_";
}

std::string LLVMTypeName(uint32_t width, uint32_t vector) {
  std::string base_type = "i" + std::to_string(width);
  if (vector == 1)
    return base_type;

  return "<" + std::to_string(vector) + " x " + base_type + ">";
}

std::string SelectTypeName(uint32_t vector) {
  if (vector == 1)
    return "i1";
  return "<" + std::to_string(vector) + " x i1>";
}

int64_t MinValue(uint32_t width, bool is_signed) {
  if (!is_signed)
    return 0;

  return -(1ull << (width - 1));
}

int64_t MaxValue(uint32_t width, bool is_signed) {
  if (is_signed)
    return (1ull << (width - 1)) - 1;
  else
    return (1ull << (width));
}

std::string SplatConstant(uint32_t vector, const std::string &type,
                          const std::string &value) {
  if (vector == 1)
    return value;

  std::string constant = "<";
  for (auto i = 0; i < vector; ++i) {
    constant += type + " " + value;
    constant += (i == (vector - 1) ? "" : ", ");
  }
  constant += ">";
  return constant;
}

std::string NotConstant(uint32_t vector) {
  return SplatConstant(vector, "i1", "true");
}

int main() {

  std::vector<uint32_t> widths = {8, 16, 32, 64};
  std::vector<uint32_t> sizes = {1, 2, 3, 4};

  for (auto width : widths) {
    for (auto is_signed : {false, true}) {
      for (auto size : sizes) {
        const std::string c_name = TypeName(width, is_signed, size);
        const std::string llvm_name = LLVMTypeName(width, size);
        const std::string selector_name = SelectTypeName(size);

        std::ofstream str("add_sat_" + c_name + ".ll");
        str << preamble << "\n";

        str << "define " << llvm_name << " @add_sat_" << c_name << "("
            << llvm_name << " %a, " << llvm_name << " %b) {\n";
        str << "entry:\n";
        str << " %call = call " << llvm_name << " @_Z7add_sat"
            << Params(width, is_signed, size, 2) << "(" << llvm_name << " %a, "
            << llvm_name << " %b)\n";
        str << " ret " << llvm_name << " %call\n";
        str << "}\n\n";
        str << "declare " << llvm_name << " @_Z7add_sat"
            << Params(width, is_signed, size, 2) << "(" << llvm_name << ", "
            << llvm_name << ")\n";
        str << "\n";

        if (is_signed) {
          if (width < 32) {
            uint32_t extended_width = width << 1;
            const std::string extended_name =
                LLVMTypeName(extended_width, size);
            str << "; CHECK: [[sext_a:%[a-zA-Z0-9_.]+]] = sext " << llvm_name
                << " %a to " << extended_name << "\n";
            str << "; CHECK: [[sext_b:%[a-zA-Z0-9_.]+]] = sext " << llvm_name
                << " %b to " << extended_name << "\n";
            str << "; CHECK: [[add:%[a-zA-Z0-9_.]+]] = add nsw "
                << extended_name << " [[sext_a]], [[sext_b]]\n";
            std::string min_value = std::to_string(MinValue(width, is_signed));
            std::string max_value = std::to_string(MaxValue(width, is_signed));
            str << "; CHECK: [[clamp:%[a-zA-Z0-9_.]+]] = call " << extended_name
                << " @_Z5clamp" << Params(extended_width, is_signed, size, 3)
                << "(" << extended_name << " [[add]], " << extended_name << " "
                << SplatConstant(size, LLVMTypeName(extended_width, 1),
                                 min_value)
                << ", " << extended_name << " "
                << SplatConstant(size, LLVMTypeName(extended_width, 1),
                                 max_value)
                << ")\n";
            str << "; CHECK: [[trunc:%[a-zA-Z0-9_.]+]] = trunc "
                << extended_name << " [[clamp]] to " << llvm_name << "\n";
            str << "; CHECK: ret " << llvm_name << " [[trunc]]\n";
          } else {
            const std::string max_value =
                std::to_string(MaxValue(width, is_signed));
            const std::string min_value =
                std::to_string(MinValue(width, is_signed));
            str << "; CHECK: [[add:%[a-zA-Z0-9_.]+]] = add " << llvm_name
                << " %a, %b\n";
            str << "; CHECK: [[add_gt_a:%[a-zA-Z0-9_.]+]] = icmp sgt "
                << llvm_name << " [[add]], %a\n";
            str << "; CHECK: [[min_clamp:%[a-zA-Z0-9_.]+]] = select "
                << selector_name << " [[add_gt_a]], " << llvm_name << " "
                << SplatConstant(size, LLVMTypeName(width, 1), min_value)
                << ", " << llvm_name << " [[add]]\n";
            str << "; CHECK: [[add_lt_a:%[a-zA-Z0-9_.]+]] = icmp slt "
                << llvm_name << " [[add]], %a\n";
            str << "; CHECK: [[max_clamp:%[a-zA-Z0-9_.]+]] = select "
                << selector_name << " [[add_lt_a]], " << llvm_name << " "
                << SplatConstant(size, LLVMTypeName(width, 1), max_value)
                << ", " << llvm_name << " [[add]]\n";
            str << "; CHECK: [[b_lt_0:%[a-zA-Z0-9_.]+]] = icmp slt "
                << llvm_name << " %b, " << (size > 1 ? "zeroinitializer" : "0")
                << "\n";
            str << "; CHECK: [[sel:%[a-zA-Z0-9_.]+]] = select " << selector_name
                << " [[b_lt_0]], " << llvm_name << " [[min_clamp]], "
                << llvm_name << " [[max_clamp]]\n";
            str << "; CHECK: ret " << llvm_name << " [[sel]]\n";
          }
        } else {
          str << "; CHECK: [[call:%[a-zA-Z0-9_.]+]] = call { " << llvm_name
              << ", " << llvm_name << " } @_Z8spirv.op.149."
              << Params(width, false, size, 1) << Params(width, false, size, 1)
              << "(i32 149, " << llvm_name << " %a, " << llvm_name << " %b)\n";
          str << "; CHECK: [[ex0:%[a-zA-Z0-9_.]+]] = extractvalue { "
              << llvm_name << ", " << llvm_name << " } [[call]], 0\n";
          str << "; CHECK: [[ex1:%[a-zA-Z0-9_.]+]] = extractvalue { "
              << llvm_name << ", " << llvm_name << " } [[call]], 1\n";
          str << "; CHECK: [[cmp:%[a-zA-Z0-9_.]+]] = icmp eq " << llvm_name
              << " [[ex1]], " << (size > 1 ? "zeroinitializer" : "0") << "\n";
          std::string constant =
              SplatConstant(size, LLVMTypeName(width, 1), "-1");
          str << "; CHECK: [[sel:%[a-zA-Z0-9_.]+]] = select " << selector_name
              << " [[cmp]], " << llvm_name << " [[ex0]], " << llvm_name << " "
              << constant << "\n";
          str << "; CHECK: ret " << llvm_name << " [[sel]]\n";
        }

        str.close();
      }
    }
  }

  return 0;
}
