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


#include "DenseData.h"
#include "SparseData.h"
#include "DecisionTree.h"
#include "ClassificationForest.h"
#include "RegressionForest.h"
#include "DataSubset.h"
#include "ExecutionConfiguration.h"
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

void printUsage() {
    std::cout
            << "#--------------- common ---------------" << std::endl
            << "<property=runmode    type=string>  train | evaluate" << std::endl
            << "<property=forestmode type=string>  classification | regression" << std::endl
            << "<property=datamode   type=string>  sparse | dense" << std::endl

            << "<property=input      type=string>  filename of features" << std::endl
            << "<property=forest     type=string>  filename for forest file. input for evaluation, output for training" << std::endl
            << "#--------------- optional for common ---------------" << std::endl
            << "<property=threads    type=integer> number of threads (default: 1)" << std::endl
            << std::endl
            << "#--------------- training ---------------" << std::endl
            << "<property=trees      type=integer> number of trees in the forest" << std::endl
            << "<property=target     type=string>  target file" << std::endl
            << "<property=oob        type=integer> 1 for running out of bag evaluation, 0 for no OOB (default: 0)" << std::endl
            << "<property=relevance  type=string>  a file name to which to write features relevance" << std::endl
            << "#--------------- optional for training ---------------" << std::endl
            << "<property=features   type=string>  log | sqrt (default: sqrt)" << std::endl
            << "<property=filter     type=string>  file with a filter for which features to use" << std::endl
            << "<property=maxdepth   type=integer> maximum depth of each tree (default: 0, unlimited)" << std::endl
            << "<property=minsplit   type=integer> minimum number of usecases in a node being split (default: 2)" << std::endl
            << std::endl
            << "#--------------- evaluation ---------------" << std::endl
            << "<property=output     type=string>  output file" << std::endl
            << "#--------------- optional for evaluation ---------------" << std::endl
            << "<property=relevance  type=string>  conpute feature relevance, and print to file" << std::endl;
}

bool verifyOptions() {
    if (!ExecutionConfiguration::stringExists("runmode"))
        return false;
    if (!ExecutionConfiguration::stringExists("forestmode"))
        return false;
    if (!ExecutionConfiguration::stringExists("datamode"))
        return false;
    if (!ExecutionConfiguration::stringExists("input"))
        return false;
    if (!ExecutionConfiguration::stringExists("forest"))
        return false;
    if (ExecutionConfiguration::getString("runmode").compare("train") == 0) {
        if (!ExecutionConfiguration::intExists("trees"))
            return false;
        if (!ExecutionConfiguration::stringExists("target"))
            return false;
    } else if (ExecutionConfiguration::getString("runmode").compare("evaluate") == 0) {
        if (!ExecutionConfiguration::stringExists("output"))
            return false;
    } else {
        return false;
    }

    return true;
}

int main(int argc, char * argv[]) {

    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " config" << std::endl;
        printUsage();
        exit(1);
    }

    ExecutionConfiguration::parseConfiguration(argv[1]);
    if (!verifyOptions()) {
        std::cerr << "Errors while parsing configuration. Run with no parameters to see requirements." << std::endl;
        std::cerr << "Configuration:" << std::endl;
        ExecutionConfiguration::printConfiguration(std::cerr);
        exit(2);
    }

    Data *d;
    try {
        if (ExecutionConfiguration::getString("datamode").compare("dense") == 0)
            d = new DenseData();
        else if (ExecutionConfiguration::getString("datamode").compare("sparse") == 0)
            d = new SparseData();
        else {
            std::cout << "Unrecognized data format " << ExecutionConfiguration::getString("datamode") << std::endl;
            exit(1);
        }
        std::cout << "Loading feature file " << ExecutionConfiguration::getString("input") << std::endl;
        d->loadFeatures(ExecutionConfiguration::getString("input"));
        if (ExecutionConfiguration::stringExists("filter")) {
            std::cout << "Using only features listed in file " << ExecutionConfiguration::getString("filter") << std::endl;
            d->filterFeatures(ExecutionConfiguration::getString("filter"));
        }
    } catch (std::runtime_error &e) {
        std::cerr << "Error occurred while reading file: " << e.what() << std::endl;
        return 1;
    }

    if (ExecutionConfiguration::getString("runmode").compare("train") == 0) {
        std::cout << d->nFeatures() << " " << d->nFilteredFeatures() << " " << d->nUsers() << std::endl;
        int useFeatures = sqrt(d->nFilteredFeatures()) + 1;
        if (ExecutionConfiguration::stringExists("features") && ExecutionConfiguration::getString("features").compare("log") == 0)
            useFeatures = log(d->nFilteredFeatures()) + 1;

        RandomForest *forest;

        try {
            if (ExecutionConfiguration::getString("forestmode").compare("classification") == 0) {
                std::cout << "Loading classification target file " << ExecutionConfiguration::getString("target") << std::endl;
                d->loadClassificationY(ExecutionConfiguration::getString("target"));
                forest = new ClassificationForest(d, ExecutionConfiguration::getInt("trees"), useFeatures,
                        ExecutionConfiguration::intExists("threads") ? ExecutionConfiguration::getInt("threads") : 1);
            } else if (ExecutionConfiguration::getString("forestmode").compare("regression") == 0) {
                std::cout << "Loading regression target file " << ExecutionConfiguration::getString("target") << std::endl;
                d->loadRegressionY(ExecutionConfiguration::getString("target"));
                forest = new RegressionForest(d, ExecutionConfiguration::getInt("trees"), useFeatures,
                        ExecutionConfiguration::intExists("threads") ? ExecutionConfiguration::getInt("threads") : 1);
            }
        } catch (std::runtime_error &e) {
            std::cerr << "Error occurred while reading file: " << e.what() << std::endl;
            return 1;
        }

        forest->train();
        std::ofstream out(ExecutionConfiguration::getString("forest").c_str());
        forest->save(out);


        // support OOB evaluation for classification
        if (ExecutionConfiguration::getString("forestmode").compare("classification") == 0 &&
                (ExecutionConfiguration::stringExists("relevance") ||
                (ExecutionConfiguration::intExists("oob") && ExecutionConfiguration::getInt("oob")))) {
            std::cout << "OOB evaluation" << std::endl;
            std::vector<int> permutation;
            std::vector<std::vector<double> > probOOB(d->nUsers());
            ClassificationForest *cforest = (ClassificationForest *) forest;
            // since there are no nodes classifying on features d.nFeatures() (since it's one more than
            // the maximum index), nothing will get permuted, and this is doing straight up OOB evaluation
            cforest->evaluateOOB(probOOB, permutation, d->nFeatures()); 

            // we use this to compute the AUC. For multi-class problems, we define the AUC as the average pair-wise AUC
            int nTrees = ExecutionConfiguration::getInt("trees");
            std::vector<std::vector<std::vector<int> > > counts(nTrees+1);
            for (size_t i = 0; i < counts.size(); ++i) {
                counts[i].resize(cforest->nClasses());
                for (size_t j = 0; j < counts[i].size(); ++j) {
                    counts[i][j].resize(cforest->nClasses());
                }
            }
            
            double acc = 0;
            
            for (int u = 0; u < d->nUsers(); u++) {
                assert(d->classificationY(u) < probOOB[u].size());
                acc += probOOB[u][d->classificationY(u)];

                assert(cforest->nClasses() == probOOB[u].size());
                for (size_t i = 0; i < probOOB[u].size(); ++i) {
                    long score = lround(probOOB[u][i] * nTrees);
                    counts[score][d->classificationY(u)][i]++;
                }                
            }
            double totalAUC = 0;
            for (int i = 0; i < cforest->nClasses(); ++i) {
                for (int j = i+1; j < cforest->nClasses(); ++j) {
                    double AUC0 = 0, cnt00 = 0, cnt01 = 0;
                    double AUC1 = 0, cnt10 = 0, cnt11 = 0;
                    for (size_t u = 0; u < counts.size(); ++u) {
                        cnt00 += counts[u][i][i];
                        cnt01 += counts[u][j][i];
                        cnt10 += counts[u][i][j];
                        cnt11 += counts[u][j][j];
                        
                        AUC0 += counts[u][i][i]*cnt01;
                        AUC1 += counts[u][j][j]*cnt10;
                    }
                    totalAUC += AUC0/cnt00/cnt01 + AUC1/cnt10/cnt11;
                }
            }
            acc = acc / d->nUsers();
            totalAUC = totalAUC / cforest->nClasses() / (cforest->nClasses()-1);
            std::cout << "OOB score: " << acc << std::endl;
            std::cout << "AUC score: " << totalAUC << std::endl;

            if (ExecutionConfiguration::stringExists("relevance")) {
                std::cout << "feature evaluation" << std::endl;
                permutation.resize(d->nUsers());
                DataSubset::permute(permutation);
                std::vector<bool> usedFeatures(d->nFeatures());
                forest->findUsedFeatures(usedFeatures);
                std::vector<std::vector<double> > prob(d->nUsers());
                std::ofstream fr(ExecutionConfiguration::getString("relevance").c_str());
                fr << "%%MatrixMarket matrix array real general" << std::endl << "%" << std::endl
                        << d->nFeatures() << " 1" << std::endl;

                for (int f = 0; f < d->nFeatures(); f++) {
                    if (usedFeatures[f]) {
                        cforest->evaluateOOB(prob, permutation, f);
                        double permAcc = 0;
                        for (int u = 0; u < d->nUsers(); u++) {
                            assert(d->classificationY(u) < prob[u].size());
                            permAcc += prob[u][d->classificationY(u)];
                        }
                        permAcc = acc - permAcc / d->nUsers();
                        fr << permAcc << std::endl;
                    } else {
                        fr << 0 << std::endl;
                    }
                }
            }
        }
    } else {
        DataSubset ds = d->createSubset();
        std::ifstream in(ExecutionConfiguration::getString("forest").c_str());
        try {
            if (ExecutionConfiguration::getString("forestmode").compare("classification") == 0) {
                ClassificationForest forest(d, 0, 0,
                        ExecutionConfiguration::intExists("threads") ? ExecutionConfiguration::getInt("threads") : 1);
                forest.load(in);
                std::vector<std::vector<double> > prob;
                prob.resize(d->nUsers());
                forest.evaluate(prob);

                std::ofstream out(ExecutionConfiguration::getString("output").c_str());
                out << "%%MatrixMarket matrix array real general" << std::endl << "%" << std::endl
                        << d->nUsers() << " " << forest.nClasses() << std::endl;
                for (int i = 0; i < forest.nClasses(); i++) {
                    for (int u = 0; u < ds.nUsers(); u++) {
                        assert(prob[u].size() == forest.nClasses());
                        out << prob[u][i] << std::endl;
                    }
                }
            } else if (ExecutionConfiguration::getString("forestmode").compare("regression") == 0) {
                RegressionForest forest(d, 0, 0,
                        ExecutionConfiguration::intExists("threads") ? ExecutionConfiguration::getInt("threads") : 1);
                forest.load(in);
                std::vector<double> Y(d->nUsers());
                forest.evaluate(Y);

                std::ofstream out(ExecutionConfiguration::getString("output").c_str());
                out << "%%MatrixMarket matrix array real general" << std::endl << "%" << std::endl
                        << d->nUsers() << " 1" << std::endl;
                for (int u = 0; u < ds.nUsers(); u++) {
                    out << Y[u] << std::endl;
                }
            }
        } catch (std::runtime_error &e) {
            std::cerr << "Error occurred while reading file: " << e.what() << std::endl;
            return 1;
        }

    }

    return 0;
}
