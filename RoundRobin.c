#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 10

typedef struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int remainingBurstTime;
    int ioWaitTime;
    int remainingIoWaitTime;
    char status[10]; // Ready, Running, Blocked
    int startTime;
    int completionTime;
    int responded; // Indicates if the process has been responded to
} Process;

Process processes[MAX_PROCESSES];
int numProcesses;
int timeQuantum;
int ioWaitTime;

void inputProcesses();
void roundRobinScheduling();
void calculateResults();

int totalCpuTime = 0; // Total CPU busy time
int time = 0;         // Global simulation time

int main() {
    inputProcesses();
    roundRobinScheduling();
    calculateResults();
    return 0;
}

void inputProcesses() {
    printf("Enter number of processes (1-10): ");
    scanf("%d", &numProcesses);

    if (numProcesses < 1 || numProcesses > 10) {
        printf("Invalid number of processes!\n");
        exit(1);
    }

    printf("Enter time quantum (TQ): ");
    scanf("%d", &timeQuantum);

    if (timeQuantum <= 0) {
        printf("Invalid time quantum!\n");
        exit(1);
    }

    printf("Enter I/O Wait Time: ");
    scanf("%d", &ioWaitTime);

    if (ioWaitTime <= 0) {
        printf("Invalid I/O Wait Time!\n");
        exit(1);
    }

    for (int i = 0; i < numProcesses; i++) {
        Process *p = &processes[i];
        p->id = i + 1;

        printf("Enter Arrival Time and Burst Time for Process %d:\n", p->id);
        printf("Arrival Time: ");
        scanf("%d", &p->arrivalTime);
        printf("Burst Time: ");
        scanf("%d", &p->burstTime);

        if (p->arrivalTime < 0 || p->burstTime < 0) {
            printf("Invalid input! Times must be non-negative.\n");
            exit(1);
        }

        p->remainingBurstTime = p->burstTime;
        p->ioWaitTime = ioWaitTime;
        p->remainingIoWaitTime = 0;
        strcpy(p->status, "Ready");
        p->startTime = -1;
        p->completionTime = 0;
        p->responded = 0;
    }
}

void roundRobinScheduling() {
    int completedProcesses = 0;

    printf("\nTime\tProcess ID\tStatus\tRemaining Time\n");

    while (completedProcesses < numProcesses) {
        int processRun = 0; // Tracks if a process runs in the current cycle

        for (int i = 0; i < numProcesses; i++) {
            Process *p = &processes[i];

            // Handle Ready processes
            if (strcmp(p->status, "Ready") == 0 && p->arrivalTime <= time && p->remainingBurstTime > 0) {
                if (p->startTime == -1) {
                    p->startTime = time; // First response time
                }
                if (!p->responded) {
                    p->responded = 1; // Mark as responded
                }

                // Run the process
                int executionTime = (p->remainingBurstTime > timeQuantum) ? timeQuantum : p->remainingBurstTime;
                printf("%d\t%d\tRunning\t%d\n", time, p->id, p->remainingBurstTime - executionTime);
                totalCpuTime += executionTime;
                time += executionTime;
                p->remainingBurstTime -= executionTime;

                if (p->remainingBurstTime > 0) {
                    strcpy(p->status, "Blocked");
                    p->remainingIoWaitTime = ioWaitTime; // Blocked for I/O
                } else {
                    strcpy(p->status, "Completed");
                    p->completionTime = time;
                    completedProcesses++;
                }

                processRun = 1; // Process ran in this cycle
            }
        }

        // Handle Blocked processes
        for (int i = 0; i < numProcesses; i++) {
            Process *p = &processes[i];
            if (strcmp(p->status, "Blocked") == 0) {
                p->remainingIoWaitTime--;
                if (p->remainingIoWaitTime <= 0) {
                    strcpy(p->status, "Ready");
                }
            }
        }

        // Print status of all blocked processes
        for (int i = 0; i < numProcesses; i++) {
            Process *p = &processes[i];
            if (strcmp(p->status, "Blocked") == 0) {
                printf("%d\t%d\tBlocked\t%d\n", time, p->id, p->remainingBurstTime);
            }
        }

        // If no process ran, increment time
        if (!processRun) {
            time++;
        }
    }
}

void calculateResults() {
    int totalTurnaroundTime = 0;
    int totalWaitingTime = 0;
    int totalResponseTime = 0;

    printf("\nRR Performance Result:\n");
    for (int i = 0; i < numProcesses; i++) {
        Process *p = &processes[i];

        int turnaroundTime = p->completionTime - p->arrivalTime;
        int waitingTime = turnaroundTime - p->burstTime;
        int responseTime = p->startTime - p->arrivalTime;

        totalTurnaroundTime += turnaroundTime;
        totalWaitingTime += waitingTime;
        totalResponseTime += responseTime;

        printf("Process %d: Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
               p->id, turnaroundTime, waitingTime, responseTime);
    }

    float cpuUtilization = (float)totalCpuTime / time * 100;

    printf("\nAverage Turnaround Time: %.2f\n", (float)totalTurnaroundTime / numProcesses);
    printf("Average Waiting Time: %.2f\n", (float)totalWaitingTime / numProcesses);
    printf("Average Response Time: %.2f\n", (float)totalResponseTime / numProcesses);
    printf("Total CPU utilization: %.2f%%\n", cpuUtilization);
}
