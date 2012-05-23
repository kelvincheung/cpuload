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

#include <assert.h>
#include <unistd.h>

#include "config.h"
#include "read_cpu_stat.h"

extern int quiet;
extern int simple;
extern int time;

ProcessList *cpudata_new(void)
{
	int i;
	ProcessList *this;
	this = calloc(sizeof(ProcessList), 1);
	if (this == NULL) {
		perror("Failed to allocate memory");
		exit(1);
	}

	FILE *file = fopen(PROCSTATFILE, "r");
	assert(file != NULL);
	char buffer[BUFLEN];
	int cpus = -1;
	do {
		cpus++;
		if (fgets(buffer, BUFLEN - 1, file) == NULL) {
			perror("Failed to get CPU number");
			exit(1);
		}
	} while (String_startsWith(buffer, "cpu"));
	fclose(file);

	this->cpuCount = cpus - 1;
	this->items = ITEMS;
	this->cpus = calloc(sizeof(CPUData), cpus);

	for (i = 0; i < cpus; i++) {
		this->cpus[i].totalTime = 1;
		this->cpus[i].totalPeriod = 1;
	}

	return this;
}

void cpudata_delete(ProcessList * this)
{
	free(this->cpus);
	free(this);
}

void read_cpu_stat(ProcessList * this)
{
	int i;
	unsigned long long int usertime, nicetime, systemtime, systemalltime,
	    idlealltime, idletime, totaltime, virtalltime;
	unsigned long long int swapFree = 0;

	int cpus = this->cpuCount;

	if (access(PROCDIR, R_OK) != 0) {
		fprintf(stderr,
			"Error: could not read procfs (compiled to look in %s).\n",
			PROCDIR);
		exit(1);
	}

	FILE *file = fopen(PROCSTATFILE, "r");
	assert(file != NULL);
	for (i = 0; i <= cpus; i++) {
		char buffer[BUFLEN];
		int cpuid;
		unsigned long long int ioWait, irq, softIrq, steal, guest;
		ioWait = irq = softIrq = steal = guest = 0;
		// Dependending on your kernel version,
		// 5, 7 or 8 of these fields will be set.
		// The rest will remain at zero.
		if (fgets(buffer, BUFLEN - 1, file) == NULL) {
			perror("Failed to read CPU stat");
			exit(1);
		}
		if (i == 0)
			sscanf(buffer,
			       "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu",
			       &usertime, &nicetime, &systemtime, &idletime,
			       &ioWait, &irq, &softIrq, &steal, &guest);
		else {
			sscanf(buffer,
			       "cpu%d %llu %llu %llu %llu %llu %llu %llu %llu %llu",
			       &cpuid, &usertime, &nicetime, &systemtime,
			       &idletime, &ioWait, &irq, &softIrq, &steal,
			       &guest);
			assert(cpuid == i - 1);
		}
		// Fields existing on kernels >= 2.6
		// (and RHEL's patched kernel 2.4...)
		idlealltime = idletime + ioWait;
		systemalltime = systemtime + irq + softIrq;
		virtalltime = steal + guest;
		totaltime =
		    usertime + nicetime + systemalltime + idlealltime +
		    virtalltime;
		CPUData *cpuData = &(this->cpus[i]);
		assert(usertime >= cpuData->userTime);
		assert(nicetime >= cpuData->niceTime);
		assert(systemtime >= cpuData->systemTime);
		assert(idletime >= cpuData->idleTime);
		assert(totaltime >= cpuData->totalTime);
		assert(systemalltime >= cpuData->systemAllTime);
		assert(idlealltime >= cpuData->idleAllTime);
		assert(ioWait >= cpuData->ioWaitTime);
		assert(irq >= cpuData->irqTime);
		assert(softIrq >= cpuData->softIrqTime);
		assert(steal >= cpuData->stealTime);
		assert(guest >= cpuData->guestTime);
		cpuData->userPeriod = usertime - cpuData->userTime;
		cpuData->nicePeriod = nicetime - cpuData->niceTime;
		cpuData->systemPeriod = systemtime - cpuData->systemTime;
		cpuData->systemAllPeriod =
		    systemalltime - cpuData->systemAllTime;
		cpuData->idleAllPeriod = idlealltime - cpuData->idleAllTime;
		cpuData->idlePeriod = idletime - cpuData->idleTime;
		cpuData->ioWaitPeriod = ioWait - cpuData->ioWaitTime;
		cpuData->irqPeriod = irq - cpuData->irqTime;
		cpuData->softIrqPeriod = softIrq - cpuData->softIrqTime;
		cpuData->stealPeriod = steal - cpuData->stealTime;
		cpuData->guestPeriod = guest - cpuData->guestTime;
		cpuData->totalPeriod = totaltime - cpuData->totalTime;
		cpuData->userTime = usertime;
		cpuData->niceTime = nicetime;
		cpuData->systemTime = systemtime;
		cpuData->systemAllTime = systemalltime;
		cpuData->idleAllTime = idlealltime;
		cpuData->idleTime = idletime;
		cpuData->ioWaitTime = ioWait;
		cpuData->irqTime = irq;
		cpuData->softIrqTime = softIrq;
		cpuData->stealTime = steal;
		cpuData->guestTime = guest;
		cpuData->totalTime = totaltime;
	}
	fclose(file);
}

void calc_cpu_load(ProcessList * this)
{
	int i;
	char buffer[BUFLEN];

	for (i = 0; i <= this->cpuCount; i++) {
		CPUData *cpuData = &(this->cpus[i]);
		cpu_load *cpuLoad = &(cpuData->cpuload);
		float total =
		    (float)(cpuData->totalPeriod ==
			    0 ? 1 : cpuData->totalPeriod);

		cpuLoad->nice_load = cpuData->nicePeriod / total * 100.0;
		cpuLoad->user_load = cpuData->userPeriod / total * 100.0;
		cpuLoad->system_load = cpuData->systemPeriod / total * 100.0;
		cpuLoad->irq_load = cpuData->irqPeriod / total * 100.0;
		cpuLoad->softirq_load = cpuData->softIrqPeriod / total * 100.0;
		cpuLoad->iowait_load = cpuData->ioWaitPeriod / total * 100.0;
		cpuLoad->steal_load = cpuData->stealPeriod / total * 100.0;
		cpuLoad->guest_load = cpuData->guestPeriod / total * 100.0;
		cpuLoad->total_load =
		    MIN(100.0,
			MAX(0.0,
			    (cpuLoad->nice_load + cpuLoad->user_load +
			     cpuLoad->system_load + cpuLoad->irq_load +
			     cpuLoad->softirq_load)));

		if ((quiet == 0) && (this->cpuCount > 1) && (i > 0))
			print_each_cpu(this, i);
	}
}

void print_each_cpu(ProcessList * this, int cpu)
{
	cpu_load *cpuLoad = &(this->cpus[cpu].cpuload);

	if (simple)
		printf
		    ("%d\tCPU%d\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\n\n",
		     time, cpu, cpuLoad->total_load, cpuLoad->user_load,
		     cpuLoad->system_load, cpuLoad->irq_load,
		     cpuLoad->softirq_load, cpuLoad->iowait_load);
	else
		printf
		    ("%d\tCPU%d\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\t%-5.1f\n\n",
		     time, cpu, cpuLoad->total_load, cpuLoad->nice_load,
		     cpuLoad->user_load, cpuLoad->system_load,
		     cpuLoad->irq_load, cpuLoad->softirq_load,
		     cpuLoad->iowait_load, cpuLoad->steal_load,
		     cpuLoad->guest_load);
}
