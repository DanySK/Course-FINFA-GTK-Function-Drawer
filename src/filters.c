/*
 * filters.c
 *
 *  Created on: Jan 18, 2013
 *      Author: Danilo Pianini
 */

#include "filters.h"

void same(GArray *points, GArray *filtered) {
	if (filtered->len > 0) {
		g_array_remove_range(filtered, 0, filtered->len);
	}
	int i;
	for (i = 0; i < points->len; i++) {
		g_array_append_val(filtered, g_array_index(points, Point2D, i));
	}
}

void invert(GArray *points, GArray *filtered) {
	if (filtered->len > 0) {
		g_array_remove_range(filtered, 0, filtered->len);
	}
	int i;
	for (i = 0; i < points->len; i++) {
		Point2D pto = g_array_index(points, Point2D, i) ;
		Point2D pt;
		pt.x = pto.x;
		pt.y = -pto.y;
		g_array_append_val(filtered, pt);
	}
}

void init_filters(void) {
	filters_number = 2;
	filters = (filter*) malloc(sizeof(filter) * filters_number);

	filters[0].name = "Unaltered";
	filters[0].filter_function = same;

	filters[1].name = "Invert";
	filters[1].filter_function = invert;

}

