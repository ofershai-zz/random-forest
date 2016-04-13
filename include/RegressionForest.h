#ifndef REGRESSIONFOREST_H
#define	REGRESSIONFOREST_H

#include "RandomForest.h"

class RegressionForest : public RandomForest {

protected:
    void evaluateThread(std::vector<double> *out);
    void evaluateOOBThread(std::vector<double> *out, std::vector<int> *permutation, int feature);
    double evaluate(int uid);
    double evaluate(int uid, std::vector<int> &permutation, int feature);

    virtual Tree * getTree() {return new RegressionTree(); }
    
public:
    RegressionForest(Data *data, int nTrees, int nFeatures, int nThreads) : RandomForest(data, nTrees, nFeatures, nThreads) {        
    }
    
    void evaluate(std::vector<double> &Y);
    void evaluateOOB(std::vector<double> &Y, std::vector<int> &permutation, int feature);

};


#endif	/* REGRESSIONFOREST_H */

