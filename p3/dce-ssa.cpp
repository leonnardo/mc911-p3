#include "Liveness.h"
#include <set>
using namespace std;

namespace {
    struct DCE_SSA : public FunctionPass {
        static char ID;
        DCE_SSA() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) {
            set<Value*> instructions;
            bool optimized = false;
            /* cria a lista de todas as variaveis usadas no programa */
            for (inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i) {
                if (i->use_empty() && !i->isTerminator()) {
                    instructions.insert(&*i);
                }
            }
            
            while(!instructions.empty()) {
                Value *v = &**(instructions.begin());
                bool empty = true;
                instructions.erase(instructions.begin());

                for(User *u : v->users()) {
                    empty = false;
                    break;
                }
                if (empty) {
                    if (Instruction *aux = (Instruction*) v) {
                        if (!aux->mayHaveSideEffects()) {
                            for (Value::use_iterator I = aux->use_begin(), E = aux->use_end(); I != E; ++I) {
                                Use *u = &*I;
                                Value *uv = u->get();
                            }
                            aux->eraseFromParent();
                            optimized = true;
                        }
                    }
                }
            }
            
            return optimized;
        }
    };
}

char DCE_SSA::ID = 1;
RegisterPass<DCE_SSA> X("dce-ssa", "Dead code elimination", false, false);
