#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

namespace {

// so we wanna do two things - pattern match 
// then do the transform 
// this is similar to lowering pass in MLIR - where you match some pattern, then rewrite

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto &F : M) {
            errs() << "I saw a function called " << F.getName() << "!\n";
            errs() << "Function body is: \n";
            errs() << F << "\n";
            for (auto &B : F) {
              for (auto& I : B) {
                if (auto* op = dyn_cast<BinaryOperator>(&I)) {
                  errs() << *op << "\n";
                  //used IR BUilder class to build instructoins
                  IRBuilder<> builder(op);
                  Value* lhs = op->getOperand(0);
                  Value* rhs = op->getOperand(1);
                  Value *mul = builder.CreateMul(lhs, rhs);
                  //replace all uses of old instruction with the new one
                  // the old instruction will be dead code then automatically
                  for (auto& U : op->uses()) {
                    User* user = U.getUser();
                    user ->setOperand(U.getOperandNo(), mul);

                  }
                }
              }
            }
        }
        return PreservedAnalyses::all();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(SkeletonPass());
                });
        }
    };
}
