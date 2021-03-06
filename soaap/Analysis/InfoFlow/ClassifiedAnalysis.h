#ifndef SOAAP_ANALYSIS_INFOFLOW_CLASSIFIEDANALYSIS_H
#define SOAAP_ANALYSIS_INFOFLOW_CLASSIFIEDANALYSIS_H

#include "Analysis/InfoFlow/InfoFlowAnalysis.h"
#include "Common/Typedefs.h"

using namespace llvm;

namespace soaap {

  class ClassifiedAnalysis: public InfoFlowAnalysis<int> {
    public:
      ClassifiedAnalysis(bool contextInsensitive) : InfoFlowAnalysis<int>(contextInsensitive) { }

    protected:
      virtual void initialise(ValueContextPairList& worklist, Module& M, SandboxVector& sandboxes);
      virtual void postDataFlowAnalysis(Module& M, SandboxVector& sandboxes);
      virtual bool performMeet(int from, int& to);
      virtual bool performUnion(int from, int& to);
      virtual int bottomValue() { return 0; }
      virtual string stringifyFact(int fact);
  };
}

#endif 
