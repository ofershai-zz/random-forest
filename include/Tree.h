/* 
 * File:   DecisionTree.h
 * Author: ofer
 *
 * Created on June 13, 2013, 12:30 PM
 */

#ifndef TREE_H
#define	TREE_H

#include "DataSubset.h"
#include <fstream>

class Tree {
    
protected: 
    
    class Node {
    public:
        bool   _isLeaf;
        double _threshold;
        int    _feature;
        Node   *_noValue;
        Node   *_greaterThan;
        Node   *_lessThan;        
        union {
            int _cls;
            float _val;
        };
        
        void splitData(DataSubset &data, std::vector<int> &noVal, std::vector<int> &greater, std::vector<int> &less);
	void findUsedFeatures(std::vector<bool> &features);
        Node() :  _isLeaf(true), _threshold(0), _feature(-1), _noValue(NULL), _greaterThan(NULL), _lessThan(NULL), _cls(-1) {}  
    };
    
    Node _root;
    int _maxDepth;
    int _minSplit;
    std::vector<int> _users;
    void train(DataSubset &data, const std::vector<int> &userIndeces, Node *node, int nFeatures, int depth);
    virtual bool checkAndMakeLeaf(DataSubset &data, const std::vector<int> &userIndeces, Node *node, int depth, bool forceLeaf = false) = 0;
    virtual void bestThreshold(DataSubset &data, int feature, double &gini, double &split) = 0;    

    Node *getNode(Data &data, int uid);
    Node *getNodeOOB (Data &data, int uid, std::vector<int> &permutation, int feature);

    virtual void save(Node *node, std::ofstream &out) = 0;
    virtual void load(Node *node, std::ifstream &in) = 0;
    
    
public:
    Tree();
    virtual ~Tree() {}
    void train(DataSubset &data, int nFeatures);

    void save(std::ofstream &out);
    void load(std::ifstream &in);

    void findUsedFeatures(std::vector<bool> &features) {
        _root.findUsedFeatures(features);
    }
};

#endif	/* TREE_H */

