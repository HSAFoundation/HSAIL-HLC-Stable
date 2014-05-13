; RUN: llc -march=amdil -mattr=+fp64 -o - %s | FileCheck %s

define double @test_ddiv(double %a, double %b) nounwind {
; CHECK: ddiv
  %result = fdiv double %a, %b
  ret double %result
}

define <2 x double> @test_ddiv_2(<2 x double> %a, <2 x double> %b) nounwind {
; CHECK: ddiv
; CHECK: ddiv
  %result = fdiv <2 x double> %a, %b
  ret <2 x double> %result
}
