#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 10

// Process structure
typedef struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int ioWaitTime;
    int remainingBurstTime;
    int remainingIoWaitTime;
    char status[10]; // Ready, Running, Blocked
} Process;

// Global variables
Process processes[MAX_PROCESSES];
int numProcesses;
int timeQuantum;

// Function to calculate the turnaround time
int calculateTurnaroundTime(int startTime, int completionTime) {
    return completionTime - startTime;
}

// Function to calculate the waiting time
int calculateWaitingTime(int turnaroundTime, int burstTime) {
    return turnaroundTime - burstTime;
}

// Function to calculate the response time
int calculateResponseTime(int startTime, int arrivalTime) {
    return startTime - arrivalTime;
}

// Function to simulate Round Robin Scheduling
void roundRobinScheduling() {
    int time = 0;
    int completedProcesses = 0;
    int totalTurnaroundTime = 0;
    int totalWaitingTime = 0;
    int totalResponseTime = 0;
    int totalCpuTime = 0;

    printf("Time\tProcess ID\tStatus\tRemaining Time\n");

    while (completedProcesses < numProcesses) {
        int allBlocked = 1;

        for (int i = 0; i < numProcesses; i++) {
            Process *p = &processes[i];

            if (p->arrivalTime <= time && p->remainingBurstTime > 0) {
                if (strcmp(p->status, "Ready") == 0 || strcmp(p->status, "Running") == 0) {
                    if (p->remainingBurstTime > timeQuantum) {
                        p->remainingBurstTime -= timeQuantum;
                        totalCpuTime += timeQuantum;
                        time += timeQuantum;
                        printf("%d\t%d\tRunning\t%d\n", time - timeQuantum, p->id, p->remainingBurstTime);
                        strcpy(p->status, "Ready"); // After the time quantum, process goes back to Ready
                    } else {
                        time += p->remainingBurstTime;
                        totalCpuTime += p->remainingBurstTime;
                        printf("%d\t%d\tRunning\t0\n", time - p->remainingBurstTime, p->id);
                        p->remainingBurstTime = 0;
                        completedProcesses++;
                        strcpy(p->status, "Completed");

                        int turnaroundTime = calculateTurnaroundTime(p->arrivalTime, time);
                        int waitingTime = calculateWaitingTime(turnaroundTime, p->burstTime);
                        int responseTime = calculateResponseTime(time - p->remainingBurstTime, p->arrivalTime);

                        totalTurnaroundTime += turnaroundTime;
                        totalWaitingTime += waitingTime;
                        totalResponseTime += responseTime;
                    }
                }
                allBlocked = 0;
            }
        }

        // Blocked process handling
        for (int i = 0; i < numProcesses; i++) {
            Process *p = &processes[i];
            if (strcmp(p->status, "Blocked") == 0) {
                if (p->remainingIoWaitTime > 0) {
                    p->remainingIoWaitTime--;
                } else {
                    strcpy(p->status, "Ready");
                }
            }
        }

        if (allBlocked) {
            time++;
        }
    }

    // Calculate and display results
    printf("\nRR Performance Result:\n");
    for (int i = 0; i < numProcesses; i++) {
        Process *p = &processes[i];
        if (strcmp(p->status, "Completed") == 0) {
            int turnaroundTime = calculateTurnaroundTime(p->arrivalTime, time);
            int waitingTime = calculateWaitingTime(turnaroundTime, p->burstTime);
            int responseTime = calculateResponseTime(time - p->remainingBurstTime, p->arrivalTime);

            printf("Process %d: Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n", p->id, turnaroundTime, waitingTime, responseTime);
        }
    }

    float cpuUtilization = (float)totalCpuTime / time * 100;
    printf("Total CPU utilization: %.2f%%\n", cpuUtilization);
}

// Function to take input for processes
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

    for (int i = 0; i < numProcesses; i++) {
        Process *p = &processes[i];
        p->id = i + 1;

        printf("Enter Arrival Time, Burst Time, I/O Wait Time for Process %d:\n", p->id);
        printf("Arrival Time: ");
        scanf("%d", &p->arrivalTime);
        printf("Burst Time: ");
        scanf("%d", &p->burstTime);
        printf("I/O Wait Time: ");
        scanf("%d", &p->ioWaitTime);

        if (p->arrivalTime < 0 || p->burstTime < 0 || p->ioWaitTime < 0) {
            printf("Invalid input! Times must be non-negative.\n");
            exit(1);
        }

        p->remainingBurstTime = p->burstTime;
        p->remainingIoWaitTime = p->ioWaitTime;
        strcpy(p->status, "Ready");
    }
}

int main() {
    // Input Processes
    inputProcesses();

    // Start the Round Robin Scheduling
    roundRobinScheduling();

    return 0;
}
