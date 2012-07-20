/*
 * Copyright (c) 2011 Zhang, Keguang <keguang.zhang@gmail.com>
 *
 * Modified from bandwidth developed by Zack T Smith <fbui@comcast.net>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include "BMP.h"
#include "config.h"
#include "read_cpu_stat.h"

/* Graphing data */
static char graph_title[100];
#define TITLE_MEMORY_GRAPH "CPU load results from cpuload " VERSION " by Kelvin Cheung <keguang.zhang@gmail.com>"
static BMP *graph;		/* Graph of results */
static int graph_width = 1024;
static int graph_height = 768;
static int graph_x_margin = 80;	/* left/right */
static int graph_y_margin = 50;	/* top/bottom */
static int graph_max_xlable = 13;
static int graph_max_ylable = 11;
static int graph_max_ypoints = 100 * 10 + 1;
static int graph_x_span = 1;
static int graph_y_span = 1;
static int graph_last_x = -1;
static int graph_last_y = -1;
static unsigned long graph_fg = RGB_BLACK;
static int legend_y;
#define MAX_GRAPH_DATA 25000
static unsigned long graph_data[MAX_GRAPH_DATA];
static int graph_data_index = 0;
#define DASHED 0x1000000	/* dashed line flag */
enum {
	DATUM_SIZE = 0,
	DATUM_AMOUNT = 1,
	DATUM_COLOR = 2,
};

extern int duration;

/*
 * Name:	graph_draw_labels_memory
 * Purpose:	Draw the labels and ticks.
 */
void graph_draw_labels_memory(void)
{
	int i;

	/* Horizontal lables */
	i = 0;
	int j;
	for (i = 0; i < graph_max_xlable; i++) {
		char str[20];
		int x = graph_x_margin +
		    i * graph_x_span / (graph_max_xlable - 1);
		int y = graph_height - graph_y_margin + 10;

		sprintf(str, "%d s",
			i * duration / (1000 * (graph_max_xlable - 1)));

		BMP_vline(graph, x, y, y - 10, RGB_BLACK);
		BMP_draw_mini_string(graph, str, x - 10, y + 8, RGB_BLACK);
	}

	/* Vertical lables */
	for (i = 0; i < graph_max_ylable; i++) {
		char str[20];
		int x = graph_x_margin - 10;
		int y = graph_height - graph_y_margin -
		    i * graph_y_span / (graph_max_ylable - 1);

		sprintf(str, "%d %%", i * 10);

		BMP_hline(graph, x, x + 10, y, RGB_BLACK);
		BMP_draw_mini_string(graph, str,
				     x - 40, y - MINIFONT_HEIGHT / 2,
				     RGB_BLACK);
	}
}

void graph_init(void)
{
	if (!graph)
		return;

	BMP_clear(graph, RGB_WHITE);

	BMP_hline(graph, graph_x_margin, graph_width - graph_x_margin,
		  graph_height - graph_y_margin, RGB_BLACK);
	BMP_vline(graph, graph_x_margin, graph_y_margin,
		  graph_height - graph_y_margin, RGB_BLACK);

	graph_x_span = graph_width - 2 * graph_x_margin;
	graph_y_span = graph_height - 2 * graph_y_margin;

	BMP_draw_mini_string(graph, graph_title,
			     graph_x_margin, graph_y_margin / 2, RGB_BLACK);

	legend_y = graph_y_margin;
}

void graph_new_line(char *str, unsigned long color)
{
	BMP_draw_mini_string(graph, str,
			     graph_width - graph_y_margin - 200, legend_y,
			     0xffffff & color);

	legend_y += 10;

	graph_fg = 0;
	graph_last_x = graph_last_y = -1;

	if (graph_data_index >= MAX_GRAPH_DATA - 2)
		error("Too many graph data.");

	graph_data[graph_data_index++] = DATUM_COLOR;
	graph_data[graph_data_index++] = color;
}

/*
 * Name:	graph_add_point
 * Purpose:	Adds a point to this list to be drawn.
 */
void graph_add_point(int size, int amount)
{
	if (graph_data_index >= MAX_GRAPH_DATA - 4)
		error("Too many graph data.");

	graph_data[graph_data_index++] = DATUM_SIZE;
	graph_data[graph_data_index++] = size;
	graph_data[graph_data_index++] = DATUM_AMOUNT;
	graph_data[graph_data_index++] = amount;
}

/*
 * Name:	graph_plot_memory
 * Purpose:	Plots a point on the current graph.
 */
void graph_plot_memory(int size, int amount, int count)
{
	int i = 0;

	int x = graph_x_margin + (int)size * graph_x_span / (count - 1);
	int y = graph_height - graph_y_margin -
	    (int)amount * graph_y_span / (graph_max_ypoints - 1);

	if (graph_last_x != -1 && graph_last_y != -1) {
		if (graph_fg & DASHED)
			BMP_line_dashed(graph, graph_last_x, graph_last_y, x, y,
					graph_fg & 0xffffff);
		else
			BMP_line(graph, graph_last_x, graph_last_y, x, y,
				 graph_fg);
	}

	graph_last_x = x;
	graph_last_y = y;
}

/*
 * Name:	graph_make_memory
 * Purpose:	Plots all lines.
 */
void graph_make_memory(int count)
{
	int i;

	graph_draw_labels_memory();

	/* OK, now draw the lines */
	int size = -1, amt = -1;
	for (i = 0; i < graph_data_index; i += 2) {
		int type = graph_data[i];
		long value = graph_data[i + 1];

		switch (type) {
		case DATUM_AMOUNT:
			amt = value;
			break;
		case DATUM_SIZE:
			size = value;
			break;
		case DATUM_COLOR:
			graph_fg = (unsigned long)value;
			graph_last_x = -1;
			graph_last_y = -1;
			break;
		}

		if (amt != -1 && size != -1) {
			graph_plot_memory(size, amt, count);
			amt = size = -1;
		}
	}
}

/*
 * Draw the curves of CPU load,
 * including user, system, irq and softirq.
 */
void graph_cpu_load(cpu_load * cl, int count)
{
	int i, chunk_size;

	strcpy(graph_title, TITLE_MEMORY_GRAPH);
	graph = BMP_new(graph_width, graph_height);
	graph_init();

	graph_new_line("Total Load", RGB_BLUE);
	for (i = 0; i < count; i++) {
		cpu_load *cpuload = &(cl[i]);
		graph_add_point(i, (cpuload->total_load * (float)10.0));
	}

	graph_new_line("User Load", RGB_GREEN);
	for (i = 0; i < count; i++) {
		cpu_load *cpuload = &(cl[i]);
		graph_add_point(i, (cpuload->user_load * (float)10.0));
	}

	graph_new_line("System Load", RGB_MAROON);
	for (i = 0; i < count; i++) {
		cpu_load *cpuload = &(cl[i]);
		graph_add_point(i, (cpuload->system_load * (float)10.0));
	}

	graph_new_line("IRQ Load", RGB_RED);
	for (i = 0; i < count; i++) {
		cpu_load *cpuload = &(cl[i]);
		graph_add_point(i, (cpuload->irq_load * (float)10.0));
	}

	graph_new_line("Softirq Load", RGB_PURPLE);
	for (i = 0; i < count; i++) {
		cpu_load *cpuload = &(cl[i]);
		graph_add_point(i, (cpuload->softirq_load * (float)10.0));
	}

	graph_make_memory(count);
	BMP_write(graph, "cpuload.bmp");
	BMP_delete(graph);
	puts("\nWrote graph to cpuload.bmp.\n");
}
