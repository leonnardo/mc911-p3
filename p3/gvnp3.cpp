//
// Created by Leonnardo Rabello on 20/5/15.
//
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"

using namespace llvm;

namespace {
    struct GVN : public FunctionPass {
        static char ID;
        GVN() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) {
            bool has_changes = false;
            // Iterates in each Basic Block of Function F
            for (Function::iterator block = F.begin(), block_end = F.end(); block != block_end; ++block) {
                for (BasicBlock::iterator inst = block->begin(), inst_end = block->end(); inst != inst_end; ++inst) {
                    // Verifies if actual instruction is store and next is load
                    BasicBlock::iterator next = inst;
                    next++;
                    Value *v = inst;
                    Value *t = next;

                    if (isa<StoreInst>(*v) && isa<LoadInst>(*t)) {
                        if (inst->getOperand(1) == next->getOperand(0)) {
                            // debug tool:
                            // errs() << "performing optimizations in:\n" << '\t' << *inst << '\n' << '\t' << *next << '\n';
                            // get operand from store inst
                            Value *operand = inst->getOperand(0);
                            // replaces all uses of load instruction with operand above
                            next->replaceAllUsesWith(operand);
                            // erase load instruction
                            next->eraseFromParent();
                            has_changes = true;
                        }
                    }
                }
            }
            return has_changes;
        }
    };
}

char GVN::ID = 0;
static RegisterPass<GVN> X("gvn-p3", "Exercicio 3 Pass", false, false);

