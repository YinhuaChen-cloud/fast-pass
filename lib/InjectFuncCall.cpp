//========================================================================
// FILE:
//    InjectFuncCall.cpp
//
// DESCRIPTION:
//    For each function defined in the input IR module, InjectFuncCall inserts
//    a call to printf (from the C standard I/O library). The injected IR code
//    corresponds to the following function call in ANSI C:
//    ```C
//      printf("(llvm-tutor) Hello from: %s\n(llvm-tutor)   number of arguments: %d\n",
//             FuncName, FuncNumArgs);
//    ```
//    This code is inserted at the beginning of each function, i.e. before any
//    other instruction is executed.
//
//    To illustrate, for `void foo(int a, int b, int c)`, the code added by InjectFuncCall
//    will generated the following output at runtime:
//    ```
//    (llvm-tutor) Hello World from: foo
//    (llvm-tutor)   number of arguments: 3
//    ```
//
// USAGE:
//    1. Legacy pass manager:
//      $ opt -load <BUILD_DIR>/lib/libInjectFuncCall.so --legacy-inject-func-call <bitcode-file>
//    2. New pass maanger:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libInjectFunctCall.so -passes=-"inject-func-call" <bitcode-file>
//
// License: MIT
//========================================================================
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <random>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <ctime>

#include "InjectFuncCall.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

#define DEBUG_TYPE "inject-func-call"
// 日志函数 by cyh --- start
class Logger {
  public:
    Logger(bool enabled) : enabled_ (enabled) {}

    void log(const char* format, ...) {
      if (enabled_) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
      }
    }

    void setEnabled(bool enabled) {
      enabled_ = enabled;
    }

  private:
    bool enabled_;
};
// 日志函数 by cyh --- end

//-----------------------------------------------------------------------------
// InjectFuncCall implementation
//-----------------------------------------------------------------------------
bool InjectFuncCall::runOnModule(Module &M) {

  // 初始化日志类
  Logger logger(true);

  // 一个三元组Vector，用来储存所有的突变点
  std::vector<std::tuple<int, int, int>> MutationPoints;

  int funcID = 0;

  std::string str1 = "s2n_conn_set_handshake_type";
  std::string str2 = "s2n_handshake_type_set_tls12_flag";
  
  for (auto &Func : M) {

    // logger.log("%s\n", Func.getName());

    if (Func.getName() != str1 && Func.getName() != str2) {
      funcID++;
      continue;
    }

    int bbID = 0;

    for (auto &BB : Func) {

      int insID = 0;

      for (auto &Ins : BB) {

        if (isa<ICmpInst>(&Ins)) {  
          // MutationPoints.push_back(std::make_tuple(functionName, bbcounter, icounter));
          // This is an icmp instruction
          ICmpInst *icmpInst = cast<ICmpInst>(&Ins);
          // Get the icmp predicate
          CmpInst::Predicate predicate = icmpInst->getPredicate();
          // Print the corresponding string representation of the predicate
          switch (predicate) {
            // 2 Not ! Drop the operator        作为 icmp ne 处理，即 value != 0
            // 19 Neq != ==
            case CmpInst::ICMP_NE:
            // 14 Lt < One of <=, >=, >, ==, !=
            case CmpInst::ICMP_SLT:
            case CmpInst::ICMP_ULT:
            // 15 Le <= One of <, >=, >, ==, !=
            case CmpInst::ICMP_SLE:
            case CmpInst::ICMP_ULE:
            // 16 Ge >= One of <, <=, >, ==, !=
            case CmpInst::ICMP_SGE:
            case CmpInst::ICMP_UGE:
            // 17 Gt > One of <, <=, >=, ==, !=
            case CmpInst::ICMP_SGT:
            case CmpInst::ICMP_UGT:
            // 18 Equality Eq == !=
            case CmpInst::ICMP_EQ:
              logger.log("(%d, %d, %d)\n", funcID, bbID, insID);
              break;
            default:
              logger.log("unknown icmp predicate\n");
              continue;
              break;
          }
        }
        else if (auto *op = dyn_cast<UnaryOperator>(&Ins)) { 
          errs() << "Unary operator: " << op->getOpcodeName() << "\n";
        }
        else {

        }

        insID++;

      }

      bbID++;

    }

    funcID++;

  }

  return false; // 肯定不会修改

}

PreservedAnalyses InjectFuncCall::run(llvm::Module &M,
                                       llvm::ModuleAnalysisManager &) {
  bool Changed =  runOnModule(M);

  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool LegacyInjectFuncCall::runOnModule(llvm::Module &M) {
  bool Changed = Impl.runOnModule(M);

  return Changed;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getInjectFuncCallPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "inject-func-call", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "inject-func-call") {
                    MPM.addPass(InjectFuncCall());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getInjectFuncCallPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyInjectFuncCall::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyInjectFuncCall>
    X("legacy-inject-func-call", "Inject ",
      false, // does modify the CFG => false
      false  // not a pure analysis pass => false
    );
