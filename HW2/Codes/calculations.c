#include "calculations.h"

// Line equation is represented like ax +b.
void leastSquareMethod(double* a, double* b, struct coordinate c[10])
{	
	int i;
	int xi = 0, yi = 0, xi_yi = 0, xi2 = 0;

	for (i = 0; i < 10; ++i)
	{
		xi += c[i].x;
		yi += c[i].y;
		xi_yi += c[i].x*c[i].y;
		xi2 += c[i].x*c[i].x;
	}

	*b = (double) (xi2*yi - xi_yi*xi) / (10*xi2 - xi*xi) ;
	*a = (double) (10*xi_yi - xi*yi) / (10*xi2 - xi*xi) ;
}

// The difference between the actual value and predicted value equals to error(e).
// The predicted value is calculated from the line equation.
// To calculate MSE, You must divide the sum of error by total numbers of data set.
void calculateMAE(double* res, struct coordinate c[10], double a, double b)
{
	int i;
	double MAE = 0;
	double exp, absl;
	for  (i = 0; i < 10; ++i)
	{
		exp = a*c[i].x + b;
		absl = abs(c[i].y - exp);
		MAE += absl;
	}

	MAE = MAE / 10;
	*res = MAE;
}

// The difference between the actual value and predicted value equals to error(e).
// The predicted value is calculated from the line equation.
// To calculate MSE, You must divide the sum of e's squares by total numbers of data set.
void calculateMSE(double* res, struct coordinate c[10], double a, double b)
{	
	int i;
	double MSE = 0;
	double exp, absl;
	for (i = 0; i < 10; ++i)
	{
		exp = a*c[i].x + b;
		absl = c[i].y - exp;
		absl *= absl;
		MSE += absl;
	}

	MSE = MSE / 10;
	*res = MSE;
}

// It equals to square root of mse.
void calculateRMSE(double* res, double _MSE)
{
	*res = sqrt(_MSE);
}





