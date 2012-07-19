/*
 * cpuload, a tool to record and graph cpu load.
 * Copyright (c) 2011 Zhang, Keguang <keguang.zhang@gmail.com>
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

#include <getopt.h>		/* for getopt */
#include <signal.h>		/* for signal */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		/* for pause */

#include <sys/time.h>		/* for setitimer */

#include "config.h"
#include "read_cpu_stat.h"

#define COPYRIGHT "(C) 2012 Kelvin Cheung <keguang.zhang@gmail.com>"

#define DURATION 5		/* default duration: 5 min */
#define INTERVAL 1000		/* default interval: 1000 milliseconds (1 sec) */

int quiet = 0;
int simple = 0;
int cpu = -1;
char cpustr[16] = ""; 
int times = 0;
int duration = DURATION;
ProcessList *pl = NULL;
cpu_load *cl = NULL;

extern void graph_cpu_load(cpu_load * cl, int count);
void analyse_cpu_load(cpu_load * cl, int count);
void do_stat(void);

void error(char *s)
{
	fprintf(stderr, "Error: %s\n", s);
	exit(1);
}

static void print_usage(void)
{
	fputs("CPU Load Recorder " VERSION " - " COPYRIGHT "\n"
	      "Released under the GNU GPL.\n\n"
	      "-h --help             Print this help screen\n"
	      "-s --simple           Simplify the output\n"
	      "-q --quiet            Suppress all normal output\n"
	      "-c --cpu <num>        choose the cpu you care about (default: -1, overall)\n"
	      "-d --duration <min>   Duration in minutes (default: 5 min, 1<=d<=10)\n"
	      "-i --interval <msec>  interval in milliseconds (default: 1000 ms; 500 or 1000)\n",
	      stdout);
	exit(0);
}

static void print_title(void)
{
	if (simple)
		puts("Time\tCPU\ttotal\tuser\tsystem\tirq\tsoftirq\tiowait\n");
	else
		puts("Time\tCPU\ttotal\tnice\tuser\tsystem\tirq\tsoftirq\tiowait\tsteal\tguest\n");
}

static void print_cpu_load(cpu_load * cpuload)
{
	if (simple)
		printf
		    ("%d\tCPU%s\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\n\n",
		     times, cpustr, cpuload->total_load, cpuload->user_load,
		     cpuload->system_load, cpuload->irq_load,
		     cpuload->softirq_load, cpuload->iowait_load);
	else
		printf
		    ("%d\tCPU%s\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\n\n",
		     times, cpustr, cpuload->total_load, cpuload->nice_load,
		     cpuload->user_load, cpuload->system_load,
		     cpuload->irq_load, cpuload->softirq_load,
		     cpuload->iowait_load, cpuload->steal_load,
		     cpuload->guest_load);
}

int main(int argc, char *argv[])
{
	int opt, opti = 0;
	int interval = INTERVAL;
	int iteration = 0;
	struct itimerval it_val;	/* for setting itimer */

	static struct option long_opts[] = {
		{"help", no_argument, NULL, 'h'},
		{"simple", no_argument, NULL, 's'},
		{"quiet", no_argument, NULL, 'q'},
		{"cpu", required_argument, NULL, 'c'},
		{"duration", required_argument, NULL, 'd'},
		{"interval", required_argument, NULL, 'i'},
		{0, 0, 0, 0}
	};

	while ((opt =
		getopt_long(argc, argv, "hsqc:d:i:", long_opts, &opti)) != EOF) {
		switch (opt) {
		case 'h':
			print_usage();
			break;

		case 's':
			simple = 1;
			break;

		case 'q':
			quiet = 1;
			break;

		case 'c':
			sscanf(optarg, "%d", &cpu);
			if (cpu < -1)
				error("Invalid argument");
			break;

		case 'd':
			sscanf(optarg, "%d", &duration);
			if (duration < 1 || duration > 10)
				error("Invalid argument");
			break;

		case 'i':
			sscanf(optarg, "%d", &interval);
			switch (interval) {
			case 1000:
			case 500:
				break;
			default:
				error("Invalid argument");
			}
			break;

		default:
			exit(1);
		}
	}

	cpu++; 		/* to fit for format of /proc/stat */ 
	if (cpu)
		sprintf(cpustr, "%d", (cpu - 1));
	duration *= 60000;
	iteration = duration / interval + 1;
	cl = calloc(sizeof(cpu_load), iteration);
	if (cl == NULL) {
		perror("Failed to allocate memory");
		exit(1);
	}
	pl = cpudata_new();

	if (quiet == 0)
		print_title();

	/* Upon SIGALRM, call do_stat().
	 * Set interval timer.  We want frequency in ms, 
	 * but the setitimer call needs seconds and useconds. */
	if (signal(SIGALRM, (void (*)(int))do_stat) == SIG_ERR) {
		perror("Unable to catch SIGALRM");
		exit(1);
	}

	/* Start interval timer */
	it_val.it_value.tv_sec = interval / 1000;
	it_val.it_value.tv_usec = (interval * 1000) % 1000000;
	it_val.it_interval = it_val.it_value;

	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
		perror("error calling setitimer()");
		exit(1);
	}

	while (times < iteration)
		pause();

	/* Stop interval timer */
	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = 0;
	it_val.it_interval = it_val.it_value;

	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
		perror("error calling setitimer()");
		exit(1);
	}

	cpudata_delete(pl);

	analyse_cpu_load(cl, iteration);

	/* Graph the results */
	graph_cpu_load(cl, iteration);
	free(cl);
}

/*
 * Find out max and min load,
 * and calculate average load.
 */
void analyse_cpu_load(cpu_load * cl, int count)
{
	int i;
	float max , min, sum = 0.0;
	max = min = cl[0].total_load;

	for (i = 0; i < count; i++) {
		max = max < cl[i].total_load ? cl[i].total_load : max;
		min = min > cl[i].total_load ? cl[i].total_load : min;
		sum += cl[i].total_load;
	}

	printf ("Max: %-5.1f\n", max);
	printf ("Avg: %-5.1f\n", sum / count);
	printf ("Min: %-5.1f\n", min);
}

/*
 * Get CPU load periodically.
 */
void do_stat(void)
{
	read_cpu_stat(pl);
	calc_cpu_load(pl);
	cl[times] = pl->cpuload[cpu];
	if (quiet == 0)
		print_cpu_load(&(cl[times]));	/* print total results */
	times++;
}
