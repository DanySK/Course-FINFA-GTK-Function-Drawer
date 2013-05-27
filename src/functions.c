/*
 * functions.c
 *
 *  Created on: Jan 17, 2013
 *      Author: Danilo Pianini
 */

#include "functions.h"

static double x(double x) {
	return x;
}

unsigned int compute(function_t f, double min, double max, double precision, GArray *points) {
	unsigned int samples = (max - min) / precision + 1;
	unsigned int i;
	for (i = 0; i < samples; i++) {
		double v = min + i * precision;
		Point2D pt;
		pt.x = v;
		pt.y = f(v);
		g_array_append_val(points, pt);
	}
	return samples;
}

static double sinx(double x) {
	return sin(x);
}

static double cosx(double x) {
	return cos(x);
}

static double ex(double x) {
	return exp(x);
}

static double logx(double x) {
	return log(x);
}

static double tanx(double x) {
	return tan(x);
}

static double wings(double x) {
	if (x < 0) {
		return cos(x) + 5 * log(-x + 1);
	} else {
		return cos(x) + 5 * log(x + 1);
	}
}

void init_functions(void) {
	functions_number = 7;

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

	functions[6].name = "y = wings(x)";
	functions[6].f = wings;

}
