/*
 * Copyright (C) 2014 Ofer Shai
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


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
