/*
 * Copyright (c) 2011 Zhang, Keguang <keguang.zhang@gmail.com>
 *
 * Modified from htop developed by Hisham Muhammad <loderunner@users.sourceforge.net>.
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

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

#ifndef PROCDIR
#define PROCDIR "/proc"
#endif

#ifndef PROCSTATFILE
#define PROCSTATFILE PROCDIR "/stat"
#endif

#ifndef PROCMEMINFOFILE
#define PROCMEMINFOFILE PROCDIR "/meminfo"
#endif

#define ITEMS 8

#define BUFLEN 256

#define String_startsWith(s, match) (strstr((s), (match)) == (s))

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct cpu_load_ {
	float total_load;
	float nice_load;
	float user_load;
	float system_load;
	float irq_load;
	float softirq_load;
	float iowait_load;
	float steal_load;
	float guest_load;
} cpu_load;

typedef struct CPUData_ {
	unsigned long long int totalTime;
	unsigned long long int userTime;
	unsigned long long int systemTime;
	unsigned long long int systemAllTime;
	unsigned long long int idleAllTime;
	unsigned long long int idleTime;
	unsigned long long int niceTime;
	unsigned long long int ioWaitTime;
	unsigned long long int irqTime;
	unsigned long long int softIrqTime;
	unsigned long long int stealTime;
	unsigned long long int guestTime;

	unsigned long long int totalPeriod;
	unsigned long long int userPeriod;
	unsigned long long int systemPeriod;
	unsigned long long int systemAllPeriod;
	unsigned long long int idleAllPeriod;
	unsigned long long int idlePeriod;
	unsigned long long int nicePeriod;
	unsigned long long int ioWaitPeriod;
	unsigned long long int irqPeriod;
	unsigned long long int softIrqPeriod;
	unsigned long long int stealPeriod;
	unsigned long long int guestPeriod;

	cpu_load cpuload; 
} CPUData;

typedef struct ProcessList_ {
	int items;
	int cpuCount;
	CPUData *cpus;
} ProcessList;

extern ProcessList *cpudata_new(void);
extern void cpudata_delete(ProcessList * this);
extern void read_cpu_stat(ProcessList * this);
extern void calc_cpu_load(ProcessList * this);
extern void print_each_cpu(ProcessList * this, int cpu);
