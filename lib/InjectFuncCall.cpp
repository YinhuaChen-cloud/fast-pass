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

  // 初始化随机数种子
  srand(time(NULL)); // seed the random number generator with the current time

  // 一个三元组Vector，用来储存所有的突变点
  std::vector<std::tuple<int, int, int>> MutationPoints;

  // 用来解析突变点文件的正则表达式 
  std::regex pattern("\\((\\d+),\\s*(\\d+),\\s*(\\d+)\\)");

  // 读取 Mutation Point 文件
  std::ifstream file("fast_mutate.txt"); // open the file
  if (file.is_open()) { // check if the file was opened successfully
    std::string line;
    while (std::getline(file, line)) { // read each line of the file
      std::smatch matches;
      if (std::regex_match(line, matches, pattern)) {
        std::tuple<int, int, int> point(std::stoi(matches[1]), std::stoi(matches[2]), std::stoi(matches[3]));
        MutationPoints.push_back(point);
      }
      else {
        std::cerr << "Regex cannot parse\n";
        exit(1);
      }
    }
    file.close(); // close the file when we're done
  } else {
    std::cerr << "Failed to open mutation point file\n";
    exit(1);
  }

  int index = rand() % MutationPoints.size();

  // 抽取随机突变点
  std::tuple<int, int, int> random_point = MutationPoints[index];

  // If program reach here, means reading mutationPoint file successfully
  logger.log("the mutation point selected is (%d, %d, %d)\n", 
    std::get<0>(random_point), std::get<1>(random_point), std::get<2>(random_point));

  // 初始化突变选择子
  int mutate_sel = -1;
  // 标注是否修改
  bool modified = false;

  int funcID = 0;
  
  for (auto &Func : M) {

    if(funcID != std::get<0>(random_point)) {
      funcID++;
      continue;
    }

    int bbID = 0;

    for (auto &BB : Func) {

      if(bbID != std::get<1>(random_point)) {
        bbID++;
        continue;
      }

      int insID = 0;

      for (auto &Ins : BB) {

        if(insID != std::get<2>(random_point)) {
          insID++;
          continue;
        }

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
              icmpInst->setPredicate(CmpInst::ICMP_EQ);
              modified = true;
              logger.log("icmp ne\n");
              break;
            // 14 Lt < One of <=, >=, >, ==, !=
            case CmpInst::ICMP_SLT:
              mutate_sel = rand() % 5;
              if(0 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SLE);
              else if(1 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SGE);
              else if(2 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SGT);
              else if(3 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp slt\n");
              break;
            case CmpInst::ICMP_ULT:
              mutate_sel = rand() % 5;
              if(0 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_ULE);
              else if(1 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_UGE);
              else if(2 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_UGT);
              else if(3 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp ult\n");
              break;
            // 15 Le <= One of <, >=, >, ==, !=
            case CmpInst::ICMP_SLE:
              mutate_sel = rand() % 5;
              if(0 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SLT);
              else if(1 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SGE);
              else if(2 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SGT);
              else if(3 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp sle\n");
              break;
            case CmpInst::ICMP_ULE:
              mutate_sel = rand() % 5;
              if(0 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_ULT);
              else if(1 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_UGE);
              else if(2 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_UGT);
              else if(3 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp ule\n");
              break;
            // 16 Ge >= One of <, <=, >, ==, !=
            case CmpInst::ICMP_SGE:
              mutate_sel = rand() % 5;
              if(0 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SLT);
              else if(1 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SLE);
              else if(2 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SGT);
              else if(3 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp sge\n");
              break;
            case CmpInst::ICMP_UGE:
              mutate_sel = rand() % 5;
              if(0 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_ULT);
              else if(1 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_ULE);
              else if(2 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_UGT);
              else if(3 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp uge\n");
              break;
            // 17 Gt > One of <, <=, >=, ==, !=
            case CmpInst::ICMP_SGT:
              mutate_sel = rand() % 5;
              if(0 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SLT);
              else if(1 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SLE);
              else if(2 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_SGE);
              else if(3 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp sgt\n");
              break;
            case CmpInst::ICMP_UGT:
              mutate_sel = rand() % 5;
              if(0 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_ULT);
              else if(1 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_ULE);
              else if(2 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_UGE);
              else if(3 == mutate_sel)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp ugt\n");
              break;
            // 18 Equality Eq == !=
            case CmpInst::ICMP_EQ:
              icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp eq\n");
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
        else if (auto *op = dyn_cast<BinaryOperator>(&Ins)) { 
          // 初始化 IRBuilder
          IRBuilder<> builder(op);
          Value* lhs = op->getOperand(0);
          Value* rhs = op->getOperand(1);
          Value* newop = NULL;

          switch (op->getOpcode()) {
            // 1 Unary Neg - Drop the operator  似乎作为 0 - operand 了，突变相当于改成 + 号
            // 4 Sub - One of +, *, /, %
            // TODO: 没有做有符号/无符号的区分
            case Instruction::Sub:
              mutate_sel = rand() % 4;
              if(0 == mutate_sel)
                newop = builder.CreateAdd(lhs, rhs);
              else if(1 == mutate_sel)
                newop = builder.CreateMul(lhs, rhs);
              else if(2 == mutate_sel)
                newop = builder.CreateSDiv(lhs, rhs);
              else
                newop = builder.CreateSRem(lhs, rhs);
              logger.log("cyh: sub\n");
              break;
            // 3 Add + One of -, *, /, %
            case Instruction::Add:
              mutate_sel = rand() % 4;
              if(0 == mutate_sel)
                newop = builder.CreateSub(lhs, rhs);
              else if(1 == mutate_sel)
                newop = builder.CreateMul(lhs, rhs);
              else if(2 == mutate_sel)
                newop = builder.CreateSDiv(lhs, rhs);
              else
                newop = builder.CreateSRem(lhs, rhs);
              logger.log("cyh: add\n");
              break;
            // 5 mul * one of +, -, /, %
            case Instruction::Mul:
              mutate_sel = rand() % 4;
              if(0 == mutate_sel)
                newop = builder.CreateAdd(lhs, rhs);
              else if(1 == mutate_sel)
                newop = builder.CreateSub(lhs, rhs);
              else if(2 == mutate_sel)
                newop = builder.CreateSDiv(lhs, rhs);
              else
                newop = builder.CreateSRem(lhs, rhs);
              logger.log("cyh: mul\n");
              break;
            // 6 div / one of +, -, *, %
            case Instruction::SDiv:
              mutate_sel = rand() % 4;
              if(0 == mutate_sel)
                newop = builder.CreateAdd(lhs, rhs);
              else if(1 == mutate_sel)
                newop = builder.CreateSub(lhs, rhs);
              else if(2 == mutate_sel)
                newop = builder.CreateMul(lhs, rhs);
              else
                newop = builder.CreateSRem(lhs, rhs);
              logger.log("cyh: sdiv\n");
              break;
            case Instruction::UDiv:
              mutate_sel = rand() % 4;
              if(0 == mutate_sel)
                newop = builder.CreateAdd(lhs, rhs);
              else if(1 == mutate_sel)
                newop = builder.CreateSub(lhs, rhs);
              else if(2 == mutate_sel)
                newop = builder.CreateMul(lhs, rhs);
              else
                newop = builder.CreateURem(lhs, rhs);
              logger.log("cyh: udiv\n");
              break;
            // 7 mod % one of +, -, *, /
            case Instruction::SRem:
              mutate_sel = rand() % 4;
              if(0 == mutate_sel)
                newop = builder.CreateAdd(lhs, rhs);
              else if(1 == mutate_sel)
                newop = builder.CreateSub(lhs, rhs);
              else if(2 == mutate_sel)
                newop = builder.CreateMul(lhs, rhs);
              else
                newop = builder.CreateSDiv(lhs, rhs);
              logger.log("cyh: srem\n");
              break;
            case Instruction::URem:
              mutate_sel = rand() % 4;
              if(0 == mutate_sel)
                newop = builder.CreateAdd(lhs, rhs);
              else if(1 == mutate_sel)
                newop = builder.CreateSub(lhs, rhs);
              else if(2 == mutate_sel)
                newop = builder.CreateMul(lhs, rhs);
              else
                newop = builder.CreateUDiv(lhs, rhs);
              logger.log("cyh: urem\n");
              break;
            // 8 bitand & one of |, ˆ
            case Instruction::And:
              mutate_sel = rand() % 2;
              if(0 == mutate_sel)
                newop = builder.CreateOr(lhs, rhs);
              else
                newop = builder.CreateXor(lhs, rhs);
              logger.log("cyh: and\n");
              break;
            // 9 bitor | one of &, ˆ
            case Instruction::Or:
              mutate_sel = rand() % 2;
              if(0 == mutate_sel)
                newop = builder.CreateAnd(lhs, rhs);
              else
                newop = builder.CreateXor(lhs, rhs);
              logger.log("cyh: or\n");
              break;
            // 10 bitxor ˆ one of &, |
            case Instruction::Xor:
              mutate_sel = rand() % 2;
              if(0 == mutate_sel)
                newop = builder.CreateAnd(lhs, rhs);
              else
                newop = builder.CreateOr(lhs, rhs);
              logger.log("cyh: xor\n");
              break;
            // 11 shl « one of »l, »a
            case Instruction::Shl:
              mutate_sel = rand() % 2;
              if(0 == mutate_sel)
                newop = builder.CreateLShr(lhs, rhs);
              else
                newop = builder.CreateAShr(lhs, rhs);
              logger.log("cyh: shl\n");
              break;
            // 12 lshr »l shl «
            case Instruction::LShr:
              newop = builder.CreateShl(lhs, rhs);
              logger.log("cyh: lshr\n");
              break;
            // 13 ashr »a shl «
            case Instruction::AShr:
              newop = builder.CreateShl(lhs, rhs);
              logger.log("cyh: ashr\n");
              break;
            default:
              errs() << "Binary operator: " << op->getOpcodeName() << "\n";
              continue;
              break;
          }

          // Everywhere the old instruction was used as an operand, use our
          // new multiply instruction instead.
          for (auto& U : op->uses()) {
            User* user = U.getUser();  // A User is anything with operands.
            user->setOperand(U.getOperandNo(), newop);
          }

          (&Ins)->eraseFromParent(); // 删除旧指令

          modified = true;

        }

        insID++;

      }

      bbID++;

    }

    funcID++;

  }

  return modified;

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
