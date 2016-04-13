#ifndef CLASSIFICATIONFOREST_H
#define	CLASSIFICATIONFOREST_H

#include "RandomForest.h"
#include "DecisionTree.h"

class ClassificationForest : public RandomForest {

protected:
    void evaluateThread(std::vector<std::vector<double> > *prob);
    void evaluateOOBThread(std::vector<std::vector<double> > *prob, std::vector<int> *permutation, int feature);
    void evaluate(int uid, std::vector<double> &prob);
    void evaluate(int uid, std::vector<double> &prob, std::vector<int> &permutation, int feature);

    virtual Tree * getTree() {return new DecisionTree(); }
    
public:
    ClassificationForest(Data *data, int nTrees, int nFeatures, int nThreads) : RandomForest(data, nTrees, nFeatures, nThreads) {} 

    void evaluate(std::vector<std::vector<double> > &prob);
    void evaluateOOB(std::vector<std::vector<double> > &prob, std::vector<int> &permutation, int feature);

};

#endif
