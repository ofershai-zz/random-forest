/* 
 * File:   DecisionTree.h
 * Author: ofer
 *
 * Created on June 13, 2013, 12:30 PM
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
