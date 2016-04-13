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

#ifndef REGRESSIONTREE_H
#define	REGRESSIONTREE_H

#include "Tree.h"

class RegressionTree : public Tree {
    
protected: 
    
    bool isSingleValue(DataSubset &data, const std::vector<int> &users);
    double averageValue(DataSubset &data, const std::vector<int> &users);
    virtual bool checkAndMakeLeaf(DataSubset &data, const std::vector<int> &userIndeces, Node *node, int depth, bool forceLeaf = false);

    virtual void load(Node *node, std::ifstream &in);
    virtual void save(Node *node, std::ofstream &out);

    
public:
    virtual ~RegressionTree() {};
    RegressionTree();
    void bestThreshold(DataSubset &data, int feature, double &val, double &split);

    virtual void evaluate (Data &data, int uid, double *out);    
    virtual bool evaluateOOB (Data &data, int uid, std::vector<int> &permutation, int feature, double *out);    
};

#endif	/* DECISIONTREE_H */
