/*
 * filters.h
 *
 *  Created on: Jan 18, 2013
 *      Author: Danilo Pianini
 */

#ifndef FILTERS_H_
#define FILTERS_H_

#include "point.h"

/*
 * Defines a type for the filter functions: functions which, given an array of
 * n points, return an array of n points
 */
typedef Point2D* (*filter_t)(const Point2D *points, const unsigned int n);

typedef struct filter {
	char *name;
	filter_t filter_function;
} filter;


filter* filters;
int filters_number;

void init_filters(void);

#endif /* FILTERS_H_ */
