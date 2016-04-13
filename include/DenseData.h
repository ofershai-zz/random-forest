#ifndef DENSEDATA_H
#define	DENSEDATA_H

#include "Data.h"

class DenseData : public Data {
protected:
    std::vector<std::vector<double> > _features; // [feature][user]
public:

    DenseData() : Data() {
    }

    virtual void iterator(DataSubsetIterator *iter, int feature, std::vector<int> &indeces);
    virtual void loadFeatures(std::string filename);
    virtual bool at(int user, int feature, double &v);
};

#endif	/* DENSEDATA_H */

