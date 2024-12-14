#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 10
#define MAX_TIME 1000

// Process structure
typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
	int io_finish_time;
	int io_finished;
	int run_finish_time;
    int io_wait_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int response_time;
    int first_response;
    char status[10];
} Process;

// Queue structure for ready queue
typedef struct {
    Process* processes[MAX_PROCESSES];
    int front;
    int rear;
    int size;
} Queue;

// Function declarations
void initQueue(Queue* q);
void enqueue(Queue* q, Process* p);
Process* dequeue(Queue* q);
int isQueueEmpty(Queue* q);
void calculateMetrics(Process processes[], int n, int total_time, int io_wait);
void printGanttChart(int gantt_chart[], int time_elapsed);
void exportGanttChartToCSV(int gantt_chart[], int time_elapsed, const char* filename);

int main() {
    int n, time_quantum, io_wait;
    Process processes[MAX_PROCESSES];
    Queue ready_queue;
    initQueue(&ready_queue);

    int gantt_chart[MAX_TIME] = {-1}; // Gantt chart timeline
    int time_elapsed = 0;

    // Input
    printf("Enter number of processes (1-10): ");
    scanf("%d", &n);
    if (n < 1 || n > MAX_PROCESSES) {
        printf("Invalid number of processes!\n");
        return 1;
    }

    printf("Enter time quantum (TQ): ");
    scanf("%d", &time_quantum);
    if (time_quantum <= 0) {
        printf("Invalid time quantum!\n");
        return 1;
    }

    printf("Enter I/O wait time: ");
    scanf("%d", &io_wait);
    if (io_wait < 0) {
        printf("Invalid I/O wait time!\n");
        return 1;
    }

    // Input process details
    for (int i = 0; i < n; i++) {
        printf("\nProcess %d:\n", i + 1);
        processes[i].pid = i + 1;

        printf("Arrival Time = ");
        scanf("%d", &processes[i].arrival_time);

        printf("Burst Time = ");
        scanf("%d", &processes[i].burst_time);

        processes[i].remaining_time = processes[i].burst_time;
        processes[i].first_response = -1;
        strcpy(processes[i].status, "Ready");

        processes[i].io_finished = 0;
    }

    // Simulation variables
    int current_time = 0;
    int completed = 0;
    int total_idle_time = 0;
    int running_process_pid = -1;

    printf("\nTime\tProcess ID\tStatus\tRemaining Time\n");
    printf("----------------------------------------\n");

    // Main scheduling loop
    while (completed < n) {
        // Check for new arrivals or I/O finished processes
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time == current_time && i != running_process_pid - 1) {
                enqueue(&ready_queue, &processes[i]);
            }

            if (strcmp(processes[i].status, "Blocked") == 0 && processes[i].io_finish_time == current_time && processes[i].io_finish_time != 0 && i != running_process_pid - 1) {
                strcpy(processes[i].status, "Ready");
                processes[i].io_finish_time = 0;
                processes[i].io_finished = 1;
                enqueue(&ready_queue, &processes[i]);
            }
        }

        if (running_process_pid != -1) {
            Process* running_process = &processes[running_process_pid - 1];

            if (running_process->run_finish_time == current_time) {
                if (running_process->burst_time - running_process->remaining_time == time_quantum) {
                    strcpy(running_process->status, "Blocked");
                    running_process->io_finish_time = current_time + io_wait;
                    running_process_pid = -1;
                } else {
                    strcpy(running_process->status, "Ready");
                    enqueue(&ready_queue, running_process);
                    running_process_pid = -1;
                }
            } else {
                running_process->remaining_time -= 1;
                gantt_chart[time_elapsed++] = running_process->pid;

                if (running_process->remaining_time == 0) {
                    running_process->completion_time = current_time;
                    completed++;
                    running_process_pid = -1;
                } else {
                    current_time += 1;
                    continue;
                }
            }
        }

        Process* current_process = NULL;

        while (!isQueueEmpty(&ready_queue)) {
            current_process = dequeue(&ready_queue);

            if (current_process->burst_time <= time_quantum && !current_process->io_finished) {
                strcpy(current_process->status, "Blocked");
                current_process->io_finish_time = current_time + io_wait;
                current_process = NULL;
            } else
                break;
        }

        if (current_process != NULL) {
            strcpy(current_process->status, "Running");
            running_process_pid = current_process->pid;

            if (current_process->first_response == -1) {
                current_process->first_response = current_time;
                current_process->response_time = current_time - current_process->arrival_time;
            }

            current_process->remaining_time -= 1;
            gantt_chart[time_elapsed++] = current_process->pid;

            if (current_process->remaining_time == 0) {
                current_process->completion_time = current_time;
                completed++;
                running_process_pid = -1;
            } else {
                current_process->run_finish_time = current_time + time_quantum;
            }
        } else {
            gantt_chart[time_elapsed++] = -1;
            total_idle_time++;
        }

        current_time += 1;
    }

    calculateMetrics(processes, n, current_time, io_wait);
    printGanttChart(gantt_chart, time_elapsed);
    exportGanttChartToCSV(gantt_chart, time_elapsed, "gantt_chart.csv");

    float cpu_utilization = ((float)(current_time - total_idle_time) / current_time) * 100;
    printf("\nTotal CPU utilization: %.2f%%\n", cpu_utilization);

    int ret = system("python generateGanttChart.py");
    if (ret != 0) {
        printf("Error: Could not execute the Python script.\n");
    }
    return 0;
}

// Queue operations
void initQueue(Queue* q) {
    q->front = q->rear = -1;
    q->size = 0;
}

void enqueue(Queue* q, Process* p) {
    if (q->size == MAX_PROCESSES) return;
    
    if (q->size == 0) {
        q->front = q->rear = 0;
    } else {
        q->rear = (q->rear + 1) % MAX_PROCESSES;
    }
    
    q->processes[q->rear] = p;
    q->size++;
}

Process* dequeue(Queue* q) {
    if (q->size == 0) return NULL;
    
    Process* p = q->processes[q->front];
    if (q->size == 1) {
        q->front = q->rear = -1;
    } else {
        q->front = (q->front + 1) % MAX_PROCESSES;
    }
    
    q->size--;
    return p;
}

int isQueueEmpty(Queue* q) {
    return q->size == 0;
}

void calculateMetrics(Process processes[], int n, int total_time, int io_wait) {
    float avg_turnaround = 0, avg_waiting = 0, avg_response = 0;
    
    printf("\nRR Performance Result:\n");
    for (int i = 0; i < n; i++) {
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time + 1;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time - io_wait;
        
        printf("Process %d: Turnaround Time = %d, Waiting Time = %d, Response Time = %d\n",
               processes[i].pid,
               processes[i].turnaround_time,
               processes[i].waiting_time,
               processes[i].response_time);
        
        avg_turnaround += processes[i].turnaround_time;
        avg_waiting += processes[i].waiting_time;
        avg_response += processes[i].response_time;
    }
    
    printf("\nAverage Turnaround Time: %.2f\n", avg_turnaround/n);
    printf("Average Waiting Time: %.2f\n", avg_waiting/n);
    printf("Average Response Time: %.2f\n", avg_response/n);
}

void printGanttChart(int gantt_chart[], int time_elapsed) {
    printf("\nGantt Chart:\n");

    // Print top border
    for (int i = 0; i < time_elapsed; i++) {
        printf("----");
    }
    printf("-\n");

    // Print process IDs
    for (int i = 0; i < time_elapsed; i++) {
        if (gantt_chart[i] == -1) {
            printf("| I "); // Idle time
        } else {
            printf("| P%d ", gantt_chart[i]);
        }
    }
    printf("|\n");

    // Print bottom border
    for (int i = 0; i < time_elapsed; i++) {
        printf("----");
    }
    printf("-\n");

    // Print timeline
    for (int i = 0; i <= time_elapsed; i++) {
        printf("%2d  ", i);
    }
    printf("\n");
}

void exportGanttChartToCSV(int gantt_chart[], int time_elapsed, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: Could not open file to write Gantt chart.\n");
        return;
    }

    fprintf(file, "Time,Process\n");
    for (int i = 0; i < time_elapsed; i++) {
        if (gantt_chart[i] == -1) {
            fprintf(file, "%d,Idle\n", i);
        } else {
            fprintf(file, "%d,P%d\n", i, gantt_chart[i]);
        }
    }

    fclose(file);
    printf("Gantt chart exported to %s.\n", filename);
}
