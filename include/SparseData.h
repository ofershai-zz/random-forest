#ifndef SPARSEDATA_H
#define	SPARSEDATA_H

#include "Data.h"
#include <vector>
#include <string>

class SparseData : public Data {
protected:

    std::vector<int> _user;
    std::vector<int> _feature;
    std::vector<double> _val;

    std::vector<int> _userT;
    std::vector<int> _featureT;
    std::vector<double> _valT;

    void transpose();

public:

    SparseData() : Data() {
    }

    virtual void iterator(DataSubsetIterator *iter, int feature, std::vector<int> &);
    virtual void loadFeatures(std::string filename);
    virtual bool at(int user, int feature, double &v);
};


#endif	/* DATA_H */

