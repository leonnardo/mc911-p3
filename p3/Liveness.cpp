#include "Liveness.h"
#include <queue>

using namespace llvm;

/* Itera sobre dos blocos basicos da funcao e adiciona os defs e uses do BB no
+ * map*/
void Liveness::computeBBDefUse(Function &F)  {
	for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {
		LivenessInfo s;
		for (BasicBlock::iterator i = b->begin(), e = b->end(); i != e; ++i) {
			for (Instruction::op_iterator o = i->op_begin(), oe = i->op_end(); o != oe; ++o) {
				Value *v = *o;
				if (isa<Instruction>(*v) || isa<Argument>(*v)) {
					if (!s.use.count(v))
						s.use.insert(v);
				}
			  }
			// For the KILL set, you can use the set of all instructions
			// that are in the block (which safely includes all of the
			// pseudo-registers assigned to in the block).
			s.def.insert(&*i);
		}
		bbLivenessMap.insert(std::make_pair(&*b, s));
	}
}

/* Computa os ins e outs com o algoritmo usando workList que o Maxiwell deu de exemplo */
void Liveness::computeBBInOut(Function &F) {
	SmallVector<BasicBlock*, 32> workList;
	for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB)
		workList.push_back(&*BB);

	while (!workList.empty()) {
		BasicBlock *b = workList.pop_back_val();
		LivenessInfo info = bbLivenessMap.lookup(b);

		// Take the union of all successors
		std::set<const Value*> a;
		for (succ_iterator SI = succ_begin(b), E = succ_end(b); SI != E; ++SI) {
			std::set<const Value*> s(bbLivenessMap.lookup(*SI).in);
			a.insert(s.begin(), s.end());
		}

		std::set<const Value*> copyIn(info.in);
		info.out = a;
		info.in.clear();
		std::set_difference(a.begin(), a.end(), info.def.begin(),info.def.end(), std::inserter(info.in, info.in.end()));
		info.in.insert(info.use.begin(), info.use.end());

		if (copyIn != info.in) {
			for (pred_iterator PI = pred_begin(b), E = pred_end(b); PI != E; ++PI)
				workList.push_back(*PI);
		}

		bbLivenessMap.erase(&*b);
		bbLivenessMap.insert(std::make_pair(b, info));
	}
}

/* Computa os ins e outs das instrucoes, tambem usando exemplo do Maxiwell*/
void Liveness::computeIInOut(Function &F) {
	for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {
		BasicBlock::iterator i = --b->end();
		std::set<const Value*> liveOut(bbLivenessMap.lookup(b).out);
		std::set<const Value*> liveIn(liveOut);

		while (true) {
			// before = after - KILL + GEN
			liveIn.erase(i);

			unsigned n = i->getNumOperands();
			for (unsigned j = 0; j < n; j++) {
				Value *v = i->getOperand(j);
				if (isa<Instruction>(*v))
				liveIn.insert(cast<Instruction>(v));
			}

			LivenessInfo io;
			io.in = liveIn;
			io.out = liveOut;
			iLivenessMap.insert(std::make_pair(&*i, io));

			liveOut = liveIn;
			if (i == b->begin())
			break;
			--i;
		}
	}
}


 bool Liveness::runOnFunction(Function &F) {
	computeBBDefUse(F);
	computeBBInOut(F);
	computeIInOut(F);
	bool has_change = false;
	std::queue <inst_iterator> erase;
	/* Itera sobre todas as instrucoes do programa e verifica quais podem ser eliminadas
	*  adicioandno elas em uma fila */
	for (inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i) {
		LivenessInfo io = iLivenessMap.lookup(&*i);

		// if instruction not in live out
		if(!io.out.count(&*i) && !i->mayHaveSideEffects() &&
			!isa<TerminatorInst>(&*i) && !isa<DbgInfoIntrinsic>(&*i) &&
			!isa<LandingPadInst>(&*i) && !isa<StoreInst>(&*i)){

			erase.push(i);

		}
	}

	// Iterando sobre a fila e removendo as instrucoes
	while (!erase.empty()) {
		has_change = true;
		inst_iterator I = erase.front();
		erase.pop();
		I->eraseFromParent();
	}
	return has_change;
}

char Liveness::ID = 0;
RegisterPass<Liveness> X("dce-liveness", "Liveness Pass", false, true);
