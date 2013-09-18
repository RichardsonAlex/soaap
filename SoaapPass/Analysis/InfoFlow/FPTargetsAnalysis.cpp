#include "Analysis/InfoFlow/FPTargetsAnalysis.h"
#include "Util/DebugUtils.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/DebugInfo.h"
#include "soaap.h"

#include <sstream>

using namespace soaap;

void FPTargetsAnalysis::initialise(ValueContextPairList& worklist, Module& M, SandboxVector& sandboxes) {
  prevIsContextInsensitiveAnalysis = ContextUtils::IsContextInsensitiveAnalysis;
  ContextUtils::setIsContextInsensitiveAnalysis(true);
  CallGraph* CG = LLVMAnalyses::getCallGraphAnalysis();
  if (Function* F = M.getFunction("llvm.var.annotation")) {
    for (User::use_iterator UI = F->use_begin(), UE = F->use_end(); UI != UE; UI++) {
      User* user = UI.getUse().getUser();
      if (IntrinsicInst* annotateCall = dyn_cast<IntrinsicInst>(user)) {
        Value* annotatedVar = dyn_cast<Value>(annotateCall->getOperand(0)->stripPointerCasts());
        GlobalVariable* annotationStrVar = dyn_cast<GlobalVariable>(annotateCall->getOperand(1)->stripPointerCasts());
        ConstantDataArray* annotationStrValArray = dyn_cast<ConstantDataArray>(annotationStrVar->getInitializer());
        StringRef annotationStrValStr = annotationStrValArray->getAsCString();
        if (annotationStrValStr.startswith(SOAAP_FP)) {
          FunctionVector callees;
          string funcListCsv = annotationStrValStr.substr(strlen(SOAAP_FP)+1); //+1 because of _
          DEBUG(dbgs() << INDENT_1 << "FP annotation " << annotationStrValStr << " found: " << *annotatedVar << ", funcList: " << funcListCsv << "\n");
          istringstream ss(funcListCsv);
          string func;
          while(getline(ss, func, ',')) {
            // trim leading and trailing spaces
            size_t start = func.find_first_not_of(" ");
            size_t end = func.find_last_not_of(" ");
            func = func.substr(start, end-start+1);
            DEBUG(dbgs() << INDENT_2 << "Function: " << func << "\n");
            if (Function* callee = M.getFunction(func)) {
              DEBUG(dbgs() << INDENT_3 << "Adding " << callee->getName() << "\n");
              callees.push_back(callee);
            }
          }
          state[ContextUtils::SINGLE_CONTEXT][annotatedVar] = callees;
          addToWorklist(annotatedVar, ContextUtils::SINGLE_CONTEXT, worklist);
        }
      }
    }
  }

  if (Function* F = M.getFunction("llvm.ptr.annotation.p0i8")) {
    for (User::use_iterator UI = F->use_begin(), UE = F->use_end(); UI != UE; UI++) {
      User* user = UI.getUse().getUser();
      if (isa<IntrinsicInst>(user)) {
        IntrinsicInst* annotateCall = dyn_cast<IntrinsicInst>(user);
        Value* annotatedVar = dyn_cast<Value>(annotateCall->getOperand(0)->stripPointerCasts());

        GlobalVariable* annotationStrVar = dyn_cast<GlobalVariable>(annotateCall->getOperand(1)->stripPointerCasts());
        ConstantDataArray* annotationStrValArray = dyn_cast<ConstantDataArray>(annotationStrVar->getInitializer());
        StringRef annotationStrValStr = annotationStrValArray->getAsCString();
        
        if (annotationStrValStr.startswith(SOAAP_FP)) {
          FunctionVector callees;
          string funcListCsv = annotationStrValStr.substr(strlen(SOAAP_FP)+1); //+1 because of _
          DEBUG(dbgs() << INDENT_1 << "FP annotation " << annotationStrValStr << " found: " << *annotatedVar << ", funcList: " << funcListCsv << "\n");
          istringstream ss(funcListCsv);
          string func;
          while(getline(ss, func, ',')) {
            // trim leading and trailing spaces
            size_t start = func.find_first_not_of(" ");
            size_t end = func.find_last_not_of(" ");
            func = func.substr(start, end-start+1);
            DEBUG(dbgs() << INDENT_2 << "Function: " << func << "\n");
            if (Function* callee = M.getFunction(func)) {
              DEBUG(dbgs() << INDENT_3 << "Adding " << callee->getName() << "\n");
              callees.push_back(callee);
            }
          }
          state[ContextUtils::SINGLE_CONTEXT][annotateCall] = callees;
          addToWorklist(annotateCall, ContextUtils::SINGLE_CONTEXT, worklist);
        }
      }
    }
  }

}

void FPTargetsAnalysis::postDataFlowAnalysis(Module& M, SandboxVector& sandboxes) {
  // restore cached value of ContextUtils::IsContextInsensitiveAnalysis
  ContextUtils::setIsContextInsensitiveAnalysis(prevIsContextInsensitiveAnalysis);
}

// return the union of from and to
FunctionVector FPTargetsAnalysis::performMeet(FunctionVector from, FunctionVector to) {
  FunctionVector meet = from;
  for (Function* F : to) {
    if (find(meet.begin(), meet.end(), F) == meet.end()) {
      meet.push_back(F);
    }
  }
  return meet;
}

FunctionVector FPTargetsAnalysis::getTargets(Value* FP) {
  return state[ContextUtils::SINGLE_CONTEXT][FP];
}
