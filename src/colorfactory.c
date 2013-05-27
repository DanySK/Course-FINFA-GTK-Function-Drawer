/*
 * colorfactory.c
 *
 *  Created on: Jan 15, 2013
 *      Author: Danilo Pianini
 */
#include <gtk/gtk.h>
#include <stdlib.h>
#include "colorfactory.h"

static GdkRGBA* gen_color_white(void) {
	GdkRGBA *white = (GdkRGBA *) malloc(sizeof(GdkRGBA));
	white->red = 1.0;
	white->green = 1.0;
	white->blue = 1.0;
	white->alpha = 1.0;
	return white;
}

static GdkRGBA* gen_color_black(void) {
	GdkRGBA *white = (GdkRGBA *) malloc(sizeof(GdkRGBA));
	white->red = 0;
	white->green = 0;
	white->blue = 0;
	white->alpha = 1.0;
	return white;
}

static GdkRGBA* gen_color_blue(void) {
	GdkRGBA *white = (GdkRGBA *) malloc(sizeof(GdkRGBA));
	white->red = 0;
	white->green = 0;
	white->blue = 1;
	white->alpha = 1.0;
	return white;
}

static GdkRGBA* gen_color_red(void) {
	GdkRGBA *white = (GdkRGBA *) malloc(sizeof(GdkRGBA));
	white->red = 1;
	white->green = 0;
	white->blue = 0;
	white->alpha = 1.0;
	return white;
}

void init_color_factory(void) {
	color_white = gen_color_white();
	color_black = gen_color_black();
	color_blue = gen_color_blue();
	color_red = gen_color_red();
}

