/*
 * filters.c
 *
 *  Created on: Jan 18, 2013
 *      Author: Danilo Pianini
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "filters.h"

Point2D* same(const Point2D *parray, const unsigned int n){
	size_t s = sizeof(Point2D) * n;
	Point2D *p = (Point2D*) malloc(s);
	memcpy(p, parray, s);
	return p;
}

Point2D* invert(const Point2D *parray, const unsigned int n){
	Point2D *p = (Point2D*) malloc(sizeof(Point2D) * n);
	unsigned int i;
	for(i = 0; i<n; i++){
		p[i].x = parray[i].x;
		p[i].y = -parray[i].y;
	}
	return p;
}


void init_filters(){
	filters_number = 2;
	filters = (filter*) malloc(sizeof(filter) * filters_number);

	filters[0].name = "Unaltered";
	filters[0].filter_function = same;

	filters[1].name = "Invert";
	filters[1].filter_function = invert;

}

