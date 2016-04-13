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

