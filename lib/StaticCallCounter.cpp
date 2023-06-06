//========================================================================
// FILE:
//    StaticCallCounter.cpp
//
// DESCRIPTION:
//    Counts the number of direct function calls (i.e. as seen in the source
//    code) in a file.
//
// USAGE:
//    1. Run through opt - legacy pass manager
//      opt -load <BUILD/DIR>/lib/libStaticCallCounter.so --legacy-static-cc
//      -analyze <input-llvm-file>
//    2. You can also run it through 'static':
//      <BUILD/DIR>/bin/static <input-llvm-file>
//
// License: MIT
//========================================================================
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include "StaticCallCounter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Format.h"

using namespace llvm;

//-----------------------------------------------------------------------------
// StaticCallCounter Implementation
//-----------------------------------------------------------------------------
AnalysisKey StaticCallCounter::Key;

StaticCallCounter::Result StaticCallCounter::runOnModule(Module &M) {
  llvm::DenseMap<const llvm::Function *, unsigned> Res;

  // 一个三元组Vector，用来储存所有的突变点
  std::vector<std::tuple<int, int, int>> MutationPoints;

  // 用来解析突变点文件的正则表达式 
  std::regex pattern("\\((\\d+),\\s*(\\d+),\\s*(\\d+)\\)");

  // 读取 Mutation Point 文件
  std::ifstream file("all_mutate.txt"); // open the file
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

  int funcID = 0;
  
  for (auto &Func : M) {

    s2n_conn_set_handshake_type
    s2n_handshake_type_set_tls12_flag

    errs() << "Function Name: " << Func.getName() << "\n";    

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
              // errs() << "icmp ne\n";
              break;
            // 14 Lt < One of <=, >=, >, ==, !=
            case CmpInst::ICMP_SLT:
              // errs() << "icmp slt\n";
              break;
            case CmpInst::ICMP_ULT:
              // errs() << "icmp ult\n";
              break;
            // 15 Le <= One of <, >=, >, ==, !=
            case CmpInst::ICMP_SLE:
              // errs() << "icmp sle\n";
              break;
            case CmpInst::ICMP_ULE:
              // errs() << "icmp ule\n";
              break;
            // 16 Ge >= One of <, <=, >, ==, !=
            case CmpInst::ICMP_SGE:
              // errs() << "icmp sge\n";
              break;
            case CmpInst::ICMP_UGE:
              // errs() << "icmp uge\n";
              break;
            // 17 Gt > One of <, <=, >=, ==, !=
            case CmpInst::ICMP_SGT:
              // errs() << "icmp sgt\n";
              break;
            case CmpInst::ICMP_UGT:
              // errs() << "icmp ugt\n";
              break;
            // 18 Equality Eq == !=
            case CmpInst::ICMP_EQ:
              // errs() << "icmp eq\n";
              break;
            default:
              // errs() << "unknown icmp predicate\n";
              continue;
              break;
          }

          MutationPoints.push_back(std::make_tuple(insID, bbID, funcID));

        }
        else if (auto *op = dyn_cast<UnaryOperator>(&Ins)) { 
          // errs() << "Unary operator: " << op->getOpcodeName() << "\n";
        }
        else if (auto *op = dyn_cast<BinaryOperator>(&Ins)) { 
          switch (op->getOpcode()) {
            // 1 Unary Neg - Drop the operator  似乎作为 0 - operand 了，突变相当于改成 + 号
            // 4 Sub - One of +, *, /, %
            case Instruction::Sub:
              // errs() << "cyh: sub" << "\n";
              break;
            // 3 Add + One of -, *, /, %
            case Instruction::Add:
              // errs() << "cyh: Add" << "\n";
              break;
            // 5 Mul * One of +, -, /, %
            case Instruction::Mul:
              // errs() << "cyh: Mul" << "\n";
              break;
            // 6 Div / One of +, -, *, %
            case Instruction::SDiv:
              // errs() << "cyh: SDiv" << "\n";
              break;
            case Instruction::UDiv:
              // errs() << "cyh: UDiv" << "\n";
              break;
            // 7 Mod % One of +, -, *, /
            case Instruction::SRem:
              // errs() << "cyh: SRem" << "\n";
              break;
            case Instruction::URem:
              // errs() << "cyh: URem" << "\n";
              break;
            // 8 BitAnd & One of |, ˆ
            case Instruction::And:
              // errs() << "cyh: And" << "\n";
              break;
            // 9 BitOr | One of &, ˆ
            case Instruction::Or:
              // errs() << "cyh: Or" << "\n";
              break;
            // 10 BitXor ˆ One of &, |
            case Instruction::Xor:
              // errs() << "cyh: Xor" << "\n";
              break;
            // 11 Shl « One of »L, »A
            case Instruction::Shl:
              // errs() << "cyh: Shl" << "\n";
              break;
            // 12 LShr »L Shl «
            case Instruction::LShr:
              // errs() << "cyh: LShr" << "\n";
              break;
            // 13 AShr »A Shl «
            case Instruction::AShr:
              // errs() << "cyh: AShr" << "\n";
              break;
            default:
              // errs() << "Binary operator: " << op->getOpcodeName() << "\n";
              continue;
              break;
          }

          MutationPoints.push_back(std::make_tuple(insID, bbID, funcID));

        }

        insID++;

      }

      bbID++;

    }

    funcID++;

  }

  return Res;
}

StaticCallCounter::Result
StaticCallCounter::run(llvm::Module &M, llvm::ModuleAnalysisManager &) {
  return runOnModule(M);
}

void LegacyStaticCallCounter::print(raw_ostream &OutS, Module const *) const {
  // TODO: do nothing here
  // printStaticCCResult(OutS, DirectCalls);
}

bool LegacyStaticCallCounter::runOnModule(llvm::Module &M) {
  DirectCalls = Impl.runOnModule(M);
  return false;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getStaticCallCounterPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "static-cc", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerAnalysisRegistrationCallback(
                [](ModuleAnalysisManager &MAM) {
                  MAM.registerPass([&] { return StaticCallCounter(); });
                });
          }};
};

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getStaticCallCounterPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyStaticCallCounter::ID = 0;

// Register the pass - required for (among others) opt
RegisterPass<LegacyStaticCallCounter>
    X("legacy-static-cc", "For each function print the number of direct calls",
      true, // Doesn't modify the CFG => true
      true  // It's a pure analysis pass => true
    );

//------------------------------------------------------------------------------
// Helper functions
//------------------------------------------------------------------------------
void printStaticCCResult(raw_ostream &OutS, const ResultStaticCC &DirectCalls) {
  // TODO: 这部分代码是用来打印静态分析结果的，先注释掉
  // OutS << "================================================="
  //      << "\n";
  // OutS << "LLVM-TUTOR: static analysis results\n";
  // OutS << "=================================================\n";
  // const char *str1 = "NAME";
  // const char *str2 = "#N DIRECT CALLS";
  // OutS << format("%-20s %-10s\n", str1, str2);
  // OutS << "-------------------------------------------------"
  //      << "\n";

  // // Generate a vector of captured functions, sorted alphabetically by function
  // // names. The solution implemented here is a suboptimal - a separate
  // // container with functions is created for sorting.
  // // TODO Make this more elegant (i.e. avoid creating a separate container)
  // std::vector<const Function *> FuncNames;
  // FuncNames.reserve(DirectCalls.size());
  // for (auto &CallCount : DirectCalls) {
  //   FuncNames.push_back(CallCount.getFirst());
  // }
  // std::sort(FuncNames.begin(), FuncNames.end(),
  //           [](const Function *x, const Function *y) {
  //             return (x->getName().str() < y->getName().str());
  //           });

  // // Print functions (alphabetically)
  // for (auto &Func : FuncNames) {
  //   unsigned NumDirectCalls = (DirectCalls.find(Func))->getSecond();
  //   OutS << format("%-20s %-10lu\n", Func->getName().str().c_str(),
  //                  NumDirectCalls);
  // }
}

        // // As per the comments in CallSite.h (more specifically, comments for
        // // the base class CallSiteBase), ImmutableCallSite constructor creates
        // // a valid call-site or NULL for something which is NOT a call site.
        // auto ICS = ImmutableCallSite(&Ins);

        // // Check whether the instruction is actually a call/invoke
        // if (nullptr == ICS.getInstruction()) {
        //   continue;
        // }

        // // Check whether the called function is directly invoked
        // auto DirectInvoc =
        //     dyn_cast<Function>(ICS.getCalledValue()->stripPointerCasts());
        // if (nullptr == DirectInvoc) {
        //   continue;
        // }

        // // Update the count for the particular call
        // auto CallCount = Res.find(DirectInvoc);
        // if (Res.end() == CallCount) {
        //   CallCount = Res.insert(std::make_pair(DirectInvoc, 0)).first;
        // }
        // ++CallCount->second;



        // 尝试使用以下两种方法去识别常数，都没用
        // if(auto *CI = dyn_cast<ConstantInt>(&Ins)) {
        //   errs() << "CI: " << CI->getValue() << "\n";
        // }
        // if(auto *C = dyn_cast<Constant>(&Ins)) {
        //   errs() << "C: " << *C << "\n";
        // }

        // for(auto &cyhoperand = Ins.value_op_begin(); cyhoperand != Ins.value_op_end(); cyhoperand++) {
        //   if (isa<Constant>(&cyhoperand))
        //     Constant *C = cast<Constant>(&cyhoperand);
        //     errs() << "C: " << *C << "\n";
        // }

        // 尝试使用这种方法去识别常数，能够识别C程序中的常数，但是也会识别出很多无关的东西
        // for (auto operand = Ins.operands().begin(); operand != Ins.operands().end(); ++operand) {
        //   if(auto *CI = dyn_cast<ConstantInt>(operand))
        //     errs() << "CI: " << CI->getValue() << "\n";
        //     // errs() << "cyh operand: " << operand->get() << "\n";
        // }

        // 下面这个分支会捕捉到 if, if-else, for循环，while循环，三元运算符。
        // TODO: 可以想想怎么把 if-else 和 三元运算符 分离出来
        // else if (auto *BI = dyn_cast<BranchInst>(&Ins)) {
        //   if (BI->isConditional()) {
        //     // This is an if-else branch
        //     // Perform any analysis or transformations here
        //     errs() << "This is an if-else branch" << "\n";
        //   }
        // }