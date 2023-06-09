//========================================================================
// FILE:
//    StaticMain.cpp
//
// DESCRIPTION:
//    A command-line tool that counts all static calls (i.e. calls as seen
//    in the source code) in the input LLVM file. Internally it uses the
//    StaticCallCounter pass.
//
// USAGE:
//    # First, generate an LLVM file:
//      clang -emit-llvm <input-file> -o <output-llvm-file>
//    # Now you can run this tool as follows:
//      <BUILD/DIR>/bin/static <output-llvm-file>
//
// License: MIT
//========================================================================
#include "StaticCallCounter.h"

#include "llvm/IRReader/IRReader.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
// Command line options
//===----------------------------------------------------------------------===//
static cl::OptionCategory CallCounterCategory{"call counter options"};

static cl::opt<std::string> InputModule{cl::Positional,
                                        cl::desc{"<Module to analyze>"},
                                        cl::value_desc{"bitcode filename"},
                                        cl::init(""),
                                        cl::Required,
                                        cl::cat{CallCounterCategory}};

//===----------------------------------------------------------------------===//
// StaticCountWrapper pass
//
// Runs StaticCallCounter and prints the result
//===----------------------------------------------------------------------===//
struct StaticCCWrapper : public PassInfoMixin<StaticCCWrapper> {
  void print(llvm::raw_ostream &OutS) const;
  ResultStaticCC DirectCalls;
  llvm::PreservedAnalyses run(llvm::Module &M,
                              llvm::ModuleAnalysisManager &MAM) {

    DirectCalls = MAM.getResult<StaticCallCounter>(M);
    printStaticCCResult(errs(), DirectCalls);
    return llvm::PreservedAnalyses::all();
  }
};

static void countStaticCalls(Module &M) {
  // Create a module pass manager and add StaticCCWrapper to it.
  ModulePassManager MPM;
  StaticCCWrapper StaticWrapper;
  MPM.addPass(StaticWrapper);

  // Create an analysis manager and register StaticCallCounter with it.
  ModuleAnalysisManager MAM;
  MAM.registerPass([&] { return StaticCallCounter(); });

  // Register all available module analysis passes defined in PassRegisty.def.
  // We only really need PassInstrumentationAnalysis (which is pulled by
  // default by PassBuilder), but to keep this concise, let PassBuilder do all
  // the _heavy-lifting_.
  PassBuilder PB;
  PB.registerModuleAnalyses(MAM);

  // Finally, run the passes registered with MPM
  MPM.run(M, MAM);
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//
int main(int Argc, char **Argv) {

  // errs() << "=== cyh ===\n";
  // errs() << Argv[0] << "\n";
  // errs() << Argv[1] << "\n";
  // errs() << Argv[2] << "\n";

  // Hide all options apart from the ones specific to this tool
  // TODO: 猜测这一行是用来决定命令行参数的数量的
  cl::HideUnrelatedOptions(CallCounterCategory); 

  cl::ParseCommandLineOptions(Argc, Argv,
                              "Counts the number of static function "
                              "calls in the input IR file\n");

  // Makes sure llvm_shutdown() is called (which cleans up LLVM objects)
  //  http://llvm.org/docs/ProgrammersManual.html#ending-execution-with-llvm-shutdown
  llvm_shutdown_obj SDO;

  // Parse the IR file passed on the command line.
  SMDiagnostic Err;
  LLVMContext Ctx;
  std::unique_ptr<Module> M = parseIRFile(InputModule.getValue(), Err, Ctx);

  if (!M) {
    errs() << "Error reading bitcode file: " << InputModule << "\n";
    Err.print(Argv[0], errs());
    return -1;
  }

  // Run the analysis and print the results
  countStaticCalls(*M);

  return 0;
}
