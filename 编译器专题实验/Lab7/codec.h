#pragma once
// Lab7 实验八：把四元式 + MemPlan 翻译成 ARMv7 (AArch32) 汇编，写入 os。
#include "irgen.h"
#include "memory.h"
#include <ostream>

void gen_arm(const vector<Quad>& quads, const MemPlan& plan, ostream& os);
