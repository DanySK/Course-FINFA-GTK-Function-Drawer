/*
 * functions.c
 *
 *  Created on: Jan 17, 2013
 *      Author: Danilo Pianini
 */

#include <stdlib.h>
#include <math.h>
#include "functions.h"

static double x(double x){
	return x;
}

unsigned int compute(function_t f, double min, double max, double precision, Point2D **points){
	unsigned int samples = (max-min) / precision +1;
	*points = (Point2D *) malloc(sizeof(Point2D) * samples);
	Point2D* deref_points = *points;
	unsigned int i;
	for(i = 0; i<samples; i++){
		double v = min+i*precision;
		deref_points[i].x = v;
		deref_points[i].y = f(v);
	}
	return samples;
}

static double sinx(double x){
	return sin(x);
}

static double cosx(double x){
	return cos(x);
}

static const double MAX_COMPUTABLE_EXP = 12;
static double ex(double x){
	return exp(x < MAX_COMPUTABLE_EXP ? x : MAX_COMPUTABLE_EXP);
}

static double logx(double x){
	return log(x);
}

static double tanx(double x){
	return tan(x);
}

void init_functions(){
	functions_number = 6;

	functions = (function *) malloc(sizeof(function) * functions_number);
	functions[0].name = "y = x";
	functions[0].f = x;

	functions[1].name = "y = sin(x)";
	functions[1].f = sinx;

	functions[2].name = "y = cos(x)";
	functions[2].f = cosx;

	functions[3].name = "y = e^(x)";
	functions[3].f = ex;

	functions[4].name = "y = log(x)";
	functions[4].f = logx;

	functions[5].name = "y = tan(x)";
	functions[5].f = tanx;
}
