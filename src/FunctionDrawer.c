/*
 ============================================================================
 Name        : FunctionDrawer.c
 Author      : Danilo Pianini
 Version     : 2013
 Copyright   : GPL
 Description : Simple application in C + GTK
 ============================================================================
 */

#ifndef MAXVAL
#define MAXVAL 1000
#endif

#ifndef MAX_SCREEN_RESOLUTION
#define MAX_SCREEN_RESOLUTION 100000
#endif

/*
 * #include Includes system header files if used with <>, while searches the
 * local path if "" are used.
 */
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <math.h>
#include <gtk/gtk.h>
#include "colorfactory.h"
#include "functions.h"
#include "filters.h"

static double minx = -10.0, maxx = 10.0, miny = -10.0, maxy = 10;
static const int DIVS = 8, TICKWIDTH = 5; // MAX_SAMPLES_FROM_FILE = 100000;
static guint width, height;
static Point2D* points;
static unsigned int pointsnr; //, samples;
static gint active_function, active_filter;
static GArray *pts, *filtered;
static GtkWidget *window, *functions_box, *filters_box, *drawing_area;
static GtkAdjustment *minx_adj, *maxx_adj, *miny_adj, *maxy_adj, *precision_adj;

int xToScreen(double x) {
	if (x > MAXVAL) {
		return MAX_SCREEN_RESOLUTION;
	}
	if (x < -MAXVAL) {
		return -MAX_SCREEN_RESOLUTION;
	}
	return (int) ((x - minx) * width / (maxx - minx));
}

int yToScreen(double y) {
	/*
	 * y coordinates are from top to bottom!
	 */
	if (y > MAXVAL) {
		return -MAX_SCREEN_RESOLUTION;
	}
	if (y < -MAXVAL) {
		return MAX_SCREEN_RESOLUTION;
	}
	return (int) (height - ((y - miny) * height / (maxy - miny)));
}

static void draw_function(cairo_t *cr) {
	/*
	 * At least two points are needed
	 */
	if (pts->len >= 2) {
		/*
		 * Print the function in blue
		 */
		unsigned int i;
		for (i = 0; i < pts->len - 1; i++) {
			cairo_move_to(cr, xToScreen(g_array_index(pts, Point2D, i) .x), yToScreen(g_array_index(pts, Point2D, i).y));
			cairo_line_to(cr, xToScreen(g_array_index(pts, Point2D, i+1).x), yToScreen(g_array_index(pts, Point2D, i+1).y));
		}
		gdk_cairo_set_source_rgba(cr, color_blue);
		cairo_stroke(cr);

		/*
		 * Print the filtered function, in red
		 */
		for (i = 0; i < filtered->len - 1; i++) {
			cairo_move_to(cr, xToScreen(g_array_index(filtered, Point2D, i) .x), yToScreen(g_array_index(filtered, Point2D, i).y));
			cairo_line_to(cr, xToScreen(g_array_index(filtered, Point2D, i+1).x), yToScreen(g_array_index(filtered, Point2D, i+1).y));
		}
		gdk_cairo_set_source_rgba(cr, color_red);
		cairo_stroke(cr);

	}
}

static void draw_axes(cairo_t *cr) {
	gdk_cairo_set_source_rgba(cr, color_black);
	cairo_set_font_size(cr, 10);
	cairo_move_to(cr, 0, yToScreen(0));
	cairo_line_to(cr, width, yToScreen(0));
	cairo_move_to(cr, xToScreen(0), 0);
	cairo_line_to(cr, xToScreen(0), height);
	int i;
	const double xstep = (maxx - minx) / DIVS;
	const double ystep = (maxy - miny) / DIVS;
	for (i = 0; i <= DIVS; i++) {
		int xcoord = xToScreen(minx + xstep * i);
		int ycoord = yToScreen(miny + ystep * i);
		cairo_move_to(cr, xcoord, yToScreen(0) - TICKWIDTH);
		cairo_line_to(cr, xcoord, yToScreen(0) + TICKWIDTH);
		cairo_move_to(cr, xcoord - 15, yToScreen(0) + TICKWIDTH + 10);
		/*
		 * Buffer size:
		 * 3 chars for the decimals
		 * 5 chars for the integer
		 * 1 char for string termination (\0)
		 */
		char bufferx[9];
		sprintf(bufferx, "%5.3f", xstep * i + minx);
		cairo_show_text(cr, bufferx);
		cairo_move_to(cr, xToScreen(0) - TICKWIDTH, ycoord);
		cairo_line_to(cr, xToScreen(0) + TICKWIDTH, ycoord);
		cairo_move_to(cr, xToScreen(0) + TICKWIDTH + 1, ycoord + 6);
		char buffery[9];
		sprintf(buffery, "%5.3f", ystep * i + miny);
		cairo_show_text(cr, buffery);
	}
	cairo_stroke(cr);
}

gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
	/*
	 * Widget current size computation
	 */
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	/*
	 * White background
	 */
	cairo_rectangle(cr, 0, 0, width, height);
	gdk_cairo_set_source_rgba(cr, color_white);
	cairo_fill(cr);

	draw_axes(cr);

	draw_function(cr);

	/*
	 * Axes
	 */
	gdk_cairo_set_source_rgba(cr, color_black);
	cairo_fill(cr);

	return TRUE;
}

static void cleanup(void) {
	if (pts->len > 0) {
		g_array_remove_range(pts, 0, pts->len);
	}
	if (filtered->len > 0) {
		g_array_remove_range(filtered, 0, filtered->len);
	}
}

static void load() {
	GtkWidget *dialog;
	GtkWindow *parent = GTK_WINDOW(gtk_widget_get_parent_window(window));
	dialog = gtk_file_chooser_dialog_new("Open File", parent, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL );
	if (gtk_dialog_run(GTK_DIALOG (dialog) ) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog) );
		FILE *file = fopen(filename, "r");
		if (file == NULL ) {
			printf("Error Reading File\n");
		} else {
			cleanup();
			double x, y;
			while (fscanf(file, "%lf %lf", &x, &y) == 2) {
				Point2D pt;
				pt.x = x;
				pt.y = y;
				g_array_append_val(pts, pt);
			}
			g_free(filename);
		}
		fclose(file);
	} else {
		cleanup();
	}
	gtk_widget_destroy(dialog);
}

static void redraw(GtkApplication *app, gpointer user_data) {
	/*
	 * Clean previous results
	 */
	cleanup();
	/*
	 * Get current data
	 */
	maxx = gtk_adjustment_get_value(maxx_adj);
	minx = -gtk_adjustment_get_value(minx_adj);
	maxy = gtk_adjustment_get_value(maxy_adj);
	miny = -gtk_adjustment_get_value(miny_adj);
	active_function = gtk_combo_box_get_active(GTK_COMBO_BOX(functions_box) );
	active_filter = gtk_combo_box_get_active(GTK_COMBO_BOX(filters_box) );

	if (active_function >= functions_number) {
		/*
		 * Load from file
		 */
		load();
	} else {
		/*
		 * One of the built-in functions has been chosen
		 */
		function_t fun = functions[active_function].f;

		/*
		 * Compute the function. The function requires a pointer to an array of
		 * Point2D to store the result, and this pointer is instanced inside.
		 * We must then create a pointer to a pointer and allocate enough memory
		 * to store the pointer to the first element of the array.
		 */
		double precision = (maxx - minx) / gtk_adjustment_get_value(precision_adj);
		compute(fun, minx, maxx, precision, pts);
	}
	/*
	 * Run the filter
	 */
	filter_t fil = filters[active_filter].filter_function;
	fil(pts, filtered);
	/*
	 * Schedule a redraw
	 */
	gtk_widget_queue_draw_area(GTK_WIDGET(drawing_area), 0, 0, 5000, 5000);
}

static void save(GtkApplication *app, gpointer user_data) {
	GtkWidget *dialog;
	GtkWindow *parent = GTK_WINDOW(gtk_widget_get_parent_window(window));
	dialog = gtk_file_chooser_dialog_new("Save File", parent, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL );
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog), "result.txt");
	if (gtk_dialog_run(GTK_DIALOG (dialog) ) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog) );
		FILE *file = fopen(filename, "w");
		if (file != NULL ) {
			int i;
			for (i = 0; i < pts->len && fprintf(file, "%lf %lf\n", g_array_index(filtered, Point2D, i) .x, g_array_index(filtered, Point2D, i).y); i++);
			if (i != pts->len) {
				printf("Error Writing File\n");
			}
		} else {
			printf("Error Writing File\n");
		}
		fclose(file);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
}

static void activate(GtkApplication *app, gpointer user_data) {
	GtkWidget *hlayout, *rlayout, *save_button, *minx_scale, *maxx_scale, *miny_scale, *maxy_scale, *precision_scale;

	gdouble v = 10, lower = 0.001, upper = MAXVAL, step = 0.1, page = 1, page_s = 1;

	minx_adj = gtk_adjustment_new(v, lower, upper, step, page, page_s);
	maxx_adj = gtk_adjustment_new(v, lower, upper, step, page, page_s);
	miny_adj = gtk_adjustment_new(v, lower, upper, step, page, page_s);
	maxy_adj = gtk_adjustment_new(v, lower, upper, step, page, page_s);

	precision_adj = gtk_adjustment_new(100, 10, 1000, 1, page, page_s);

	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW (window), "FINF-A Function Drawer");
	gtk_window_set_default_size(GTK_WINDOW(window), 1024, 768);

	hlayout = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(hlayout) );

	drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, 768, 768);
	g_signal_connect(G_OBJECT (drawing_area), "draw", G_CALLBACK (draw_callback), NULL);
	gtk_paned_add1(GTK_PANED(hlayout), drawing_area);

	rlayout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_paned_add2(GTK_PANED(hlayout), rlayout);

	/*
	 * Functions box
	 */
	functions_box = gtk_combo_box_text_new();
	/*
	 * Init the external load
	 */
	char *buffer = (char *) malloc(sizeof(char) * 5);
	sprintf(buffer, "Load");
	gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(functions_box), 0, buffer, "Load from file");
	/*
	 * Init the built-in functions
	 */
	int i;
	for (i = functions_number - 1; i >= 0; i--) {
		char *buffer = (char *) malloc(sizeof(char) * 3);
		sprintf(buffer, "%2d", functions_number - i - 1);
		gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(functions_box), 0, buffer, functions[i].name);
	}
	active_function = 0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(functions_box), active_function);
	g_signal_connect(functions_box, "changed", G_CALLBACK(redraw), NULL);

	/*
	 * Filters box
	 */
	filters_box = gtk_combo_box_text_new();
	for (i = filters_number - 1; i >= 0; i--) {
		char *buffer = (char *) malloc(sizeof(char) * 3);
		sprintf(buffer, "%2d", functions_number - i - 1);
		gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(filters_box), 0, buffer, filters[i].name);
	}
	active_filter = 0;
	gtk_combo_box_set_active(GTK_COMBO_BOX(filters_box), active_filter);
	g_signal_connect(filters_box, "changed", G_CALLBACK(redraw), NULL);

	/*
	 * Save button
	 */
	save_button = gtk_button_new_with_label("Save");
	g_signal_connect(save_button, "clicked", G_CALLBACK(save), NULL);

	/*
	 * Scales
	 */
	minx_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, minx_adj);
	maxx_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, maxx_adj);
	miny_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, miny_adj);
	maxy_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, maxy_adj);
	precision_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, precision_adj);
	g_signal_connect(minx_adj, "value-changed", G_CALLBACK(redraw), NULL);
	g_signal_connect(maxx_adj, "value-changed", G_CALLBACK(redraw), NULL);
	g_signal_connect(miny_adj, "value-changed", G_CALLBACK(redraw), NULL);
	g_signal_connect(maxy_adj, "value-changed", G_CALLBACK(redraw), NULL);
	g_signal_connect(precision_adj, "value-changed", G_CALLBACK(redraw), NULL);

	gtk_box_pack_start(GTK_BOX(rlayout), gtk_label_new("Base function"), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), functions_box, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), gtk_label_new("Function filter"), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), filters_box, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), gtk_label_new("Precision"), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), precision_scale, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), gtk_label_new("Min X"), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), minx_scale, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), gtk_label_new("Max X"), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), maxx_scale, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), gtk_label_new("Min Y"), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), miny_scale, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), gtk_label_new("Max Y"), TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), maxy_scale, TRUE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(rlayout), save_button, TRUE, FALSE, 0);
	gtk_widget_show_all(window);

	/*
	 * Close operation
	 */
	g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	/*
	 * First draw
	 */
	redraw(NULL, NULL );
}

static void init() {
	init_color_factory();
	init_functions();
	init_filters();
	/*
	 * Fake data
	 */
	pts = g_array_new(FALSE, FALSE, sizeof(Point2D)); //(Point2D *) malloc(sizeof(Point2D *));
	filtered = g_array_new(FALSE, FALSE, sizeof(Point2D)); //(Point2D *) malloc(sizeof(Point2D *));

	pointsnr = 0;
	points = (Point2D *) malloc(pointsnr * sizeof(Point2D));
}

int main(int argc, char **argv) {
	init();

	GtkApplication *app;
	int status;

	/*
	 * Creates a new GApplication instance.
	 * If non-NULL, the application id must be valid. See
	 * g_application_id_is_valid().
	 * If no application ID is given then some features of GApplication (most
	 * notably application uniqueness) will be disabled.
	 */
	app = gtk_application_new("it.unibo.FunctionDrawer", G_APPLICATION_FLAGS_NONE);

	/*
	 * Connects a GCallback function to a signal for a particular object.
	 * The handler will be called before the default handler of the signal.
	 *
	 * It is mandatory to associate a callback function to the signal
	 * "activate", which is the main entry point of a GTK+ application.
	 */
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

	/*
	 * This function is intended to be run from main() and its return value is
	 * intended to be returned by main().
	 * Although you are expected to pass the argc, argv parameters from main()
	 * to this function, it is possible to pass NULL if argv is not available
	 * or commandline handling is not required.
	 */
	status = g_application_run(G_APPLICATION(app), 0, NULL );

	/*
	 * Decreases the reference count of object. When its reference count drops
	 * to 0, the object is finalized (i.e. its memory is freed).
	 */
	g_object_unref(app);
	return status;
}
