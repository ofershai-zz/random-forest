/* 
 * File:   DecisionTree.h
 * Author: ofer
 *
 * Created on June 13, 2013, 12:30 PM
 */

#ifndef DECISIONTREE_H
#define	DECISIONTREE_H

#include "Tree.h"

class DecisionTree : public Tree {
    
protected: 
    
    bool isSingleClass(DataSubset &data, const std::vector<int> &users);
    int mostPopularClass(DataSubset &data, const std::vector<int> &users);
    virtual bool checkAndMakeLeaf(DataSubset &data, const std::vector<int> &userIndeces, Node *node, int depth, bool forceLeaf = false);

    virtual void load(Node *node, std::ifstream &in);
    virtual void save(Node *node, std::ofstream &out);
    
    
public:
    virtual ~DecisionTree() {};
    DecisionTree();
    void bestThreshold(DataSubset &data, int feature, double &gini, double &split);

    virtual void evaluate (Data &data, int uid, int *out);    
    virtual bool evaluateOOB (Data &data, int uid, std::vector<int> &permutation, int feature, int *out);    

};

#endif	/* DECISIONTREE_H */

