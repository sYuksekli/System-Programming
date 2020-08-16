#ifndef CALCULATIONS_H
#define CALCULATIONS_H 


#include <math.h>
#include <stdlib.h>


// This struct represent coordinates.
struct coordinate
{
	int x;
	int y;
};

// To calculate line equation
void leastSquareMethod(double* a, double* b, struct coordinate c[10]);

// To calculate Mean Absolute Error
void calculateMAE(double* res, struct coordinate c[10], double a, double b);

// To calculate Mean Squared Error
void calculateMSE(double* res, struct coordinate c[10], double a, double b);

// To calculate Root Mean Squared Error
void calculateRMSE(double* res, double _MSE);



#endif