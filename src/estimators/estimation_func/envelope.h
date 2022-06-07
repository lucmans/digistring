#ifndef DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_ENVELOPE_H
#define DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_ENVELOPE_H


// Can be called with (NULL, NULL, 0) at any time to pre-calculate Gaussian function
void gaussian_envelope(const double norms[], double envelope[], const int n_norms);


#endif  // DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_ENVELOPE_H
