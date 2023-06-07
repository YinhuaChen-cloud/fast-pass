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
#include <random>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include "StaticCallCounter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Format.h"

using namespace llvm;

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
// StaticCallCounter Implementation
//-----------------------------------------------------------------------------
AnalysisKey StaticCallCounter::Key;

StaticCallCounter::Result StaticCallCounter::runOnModule(Module &M) {

  // 初始化日志类
  Logger logger(true);

  // 初始化随机数种子
  srand(time(NULL)); // seed the random number generator with the current time

  llvm::DenseMap<const llvm::Function *, unsigned> Res;

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

  exit(0);

  // 初始化突变选择子
  int mul_select = -1;

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
              mul_select = rand() % 5;
              if(0 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SLE);
              else if(1 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SGE);
              else if(2 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SGT);
              else if(3 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp slt\n");
              break;
            case CmpInst::ICMP_ULT:
              mul_select = rand() % 5;
              if(0 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_ULE);
              else if(1 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_UGE);
              else if(2 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_UGT);
              else if(3 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp ult\n");
              break;
            // 15 Le <= One of <, >=, >, ==, !=
            case CmpInst::ICMP_SLE:
              mul_select = rand() % 5;
              if(0 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SLT);
              else if(1 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SGE);
              else if(2 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SGT);
              else if(3 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp sle\n");
              break;
            case CmpInst::ICMP_ULE:
              mul_select = rand() % 5;
              if(0 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_ULT);
              else if(1 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_UGE);
              else if(2 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_UGT);
              else if(3 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp ule\n");
              break;
            // 16 Ge >= One of <, <=, >, ==, !=
            case CmpInst::ICMP_SGE:
              mul_select = rand() % 5;
              if(0 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SLT);
              else if(1 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SLE);
              else if(2 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SGT);
              else if(3 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp sge\n");
              break;
            case CmpInst::ICMP_UGE:
              mul_select = rand() % 5;
              if(0 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_ULT);
              else if(1 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_ULE);
              else if(2 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_UGT);
              else if(3 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp uge\n");
              break;
            // 17 Gt > One of <, <=, >=, ==, !=
            case CmpInst::ICMP_SGT:
              mul_select = rand() % 5;
              if(0 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SLT);
              else if(1 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SLE);
              else if(2 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_SGE);
              else if(3 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_EQ);
              else
                icmpInst->setPredicate(CmpInst::ICMP_NE);
              modified = true;
              logger.log("icmp sgt\n");
              break;
            case CmpInst::ICMP_UGT:
              mul_select = rand() % 5;
              if(0 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_ULT);
              else if(1 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_ULE);
              else if(2 == mul_select)
                icmpInst->setPredicate(CmpInst::ICMP_UGE);
              else if(3 == mul_select)
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
          switch (op->getOpcode()) {
            // 1 Unary Neg - Drop the operator  似乎作为 0 - operand 了，突变相当于改成 + 号
            // 4 Sub - One of +, *, /, %
            case Instruction::Sub:
              logger.log("cyh: sub\n");
              break;
            // 3 Add + One of -, *, /, %
            case Instruction::Add:
              logger.log("cyh: add\n");
              break;
            // 5 mul * one of +, -, /, %
            case Instruction::Mul:
              logger.log("cyh: mul\n");
              break;
            // 6 div / one of +, -, *, %
            case Instruction::SDiv:
              logger.log("cyh: sdiv\n");
              break;
            case Instruction::UDiv:
              logger.log("cyh: udiv\n");
              break;
            // 7 mod % one of +, -, *, /
            case Instruction::SRem:
              logger.log("cyh: srem\n");
              break;
            case Instruction::URem:
              logger.log("cyh: urem\n");
              break;
            // 8 bitand & one of |, ˆ
            case Instruction::And:
              logger.log("cyh: and\n");
              break;
            // 9 bitor | one of &, ˆ
            case Instruction::Or:
              logger.log("cyh: or\n");
              break;
            // 10 bitxor ˆ one of &, |
            case Instruction::Xor:
              logger.log("cyh: xor\n");
              break;
            // 11 shl « one of »l, »a
            case Instruction::Shl:
              logger.log("cyh: shl\n");
              break;
            // 12 lshr »l shl «
            case Instruction::LShr:
              logger.log("cyh: lshr\n");
              break;
            // 13 ashr »a shl «
            case Instruction::AShr:
              logger.log("cyh: ashr\n");
              break;
            default:
              errs() << "Binary operator: " << op->getOpcodeName() << "\n";
              continue;
              break;
          }
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