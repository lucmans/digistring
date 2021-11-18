
#include "estimator.h"

#include <map>
#include <string>


const std::map<const Estimators, const std::string> EstimatorString = {{Estimators::highres, "highres"},
                                                                       {Estimators::tuned, "tuned"}};


Estimator::Estimator() {

}

Estimator::~Estimator() {

}
