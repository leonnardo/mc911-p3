#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "Liveness.h"
#include <unistd.h>
#include <stdio.h>

using namespace std;

void print_elem(const Value* i) {
  errs() << i->getName() << '\n';
}

void Liveness::addToMap(Function &F) {
    static int id = 1;
    for (inst_iterator i = inst_begin(F), ie = inst_end(F); i != ie; ++i, ++id) {
        instMap.insert(make_pair(&*i, id));
    }
}

bool Liveness::isLiveOut(Instruction *I, Value *V){
    return false;
}

/* Itera sobre dos blocos basicos da funcao e adiciona os defs e uses do BB no
 * map*/
void Liveness::computeBBDefUse(Function &F) {
    for (Function::iterator BB = F.begin(), BBE = F.end(); BB != BBE; ++BB) {
        LivenessInfo liveness;
        for (BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; ++I) {
            unsigned n = I->getNumOperands();
            for (unsigned j = 0; j < n; j++) {
                Value *v = I->getOperand(j);
                Instruction *op = (Instruction *) v;
                if (isa<Instruction>(v)) {
                    if (!liveness.def.count(op)) {
                        liveness.use.insert(op);
                        errs() << "add to use: ";
                        print_elem(op);
                    }
                }
                liveness.def.insert(&*I);
                errs() << "add to def: ";
                print_elem(I);
            }
            bbLivenessMap.insert(make_pair(&*BB, liveness));
        }
    }
}

void Liveness::computeBBInOut(Function &F){
}

void Liveness::computeIInOut(Function &F) {
}

bool Liveness::runOnFunction(Function &F) {
    computeBBDefUse(F);
    computeBBInOut(F);
    computeIInOut(F);
	return false;
}

char Liveness::ID = 0;

RegisterPass<Liveness> X("liveness", "Live vars analysis", false, false);
