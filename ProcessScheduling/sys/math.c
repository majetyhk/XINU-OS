#include <stdio.h>
#include "math.h"

double pow(double base, int exponent){
	double result=1;
	int counter=0;
	while(counter<exponent){
		result=result*base;
		counter++;
	}
	return result;
}

double log(double inp){
	int nmax=20;
	int i=0;
	double numerator = inp-1;
	double denom = inp+1;
	int exponent;
	double result=0;
	while(i<nmax){
		exponent=2*i+1;
		result=result+((pow(numerator,exponent)/pow(denom,exponent))/exponent);
		i++;
	}
	result=result*2;
	return result;
}

double expdev(double lambda) {
    double dummy;
    do
        dummy= (double) rand() / RAND_MAX;
    while (dummy == 0.0);
    return -log(dummy) / lambda;
}

/*int main(){
	printf("%f\n", log(1));
	printf("%f\n", log(2));
	printf("%f\n", log(3));
	printf("%f\n", log(0.5));
	printf("%f\n", log(0.1));
	return 0;
}*/