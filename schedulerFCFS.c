#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

// FCFS scheduler info
typedef struct {
    list_t* FCFS_list;
    job_t* curr_job; 
} scheduler_FCFS_t;

// Compare function designed for FCFS to be given to linked list
int FCFS_compare(void* data1, void* data2)
{
    uint64_t arrival_time1 = jobGetArrivalTime(data1);
    uint64_t arrival_time2 = jobGetArrivalTime(data2);
    if (arrival_time1 == arrival_time2) {
        return 0;
    } else if (arrival_time1 > arrival_time2) {
        return -1;
    }
    return 1; 
}

// Creates and returns scheduler specific info
void* schedulerFCFSCreate()
{
    scheduler_FCFS_t* info = malloc(sizeof(scheduler_FCFS_t));
    if (info == NULL) {
        return NULL;
    }

    info->FCFS_list = list_create(FCFS_compare); 
    info->curr_job = NULL; 
    return info;
}

// Destroys scheduler specific info
void schedulerFCFSDestroy(void* schedulerInfo)
{
    scheduler_FCFS_t* info = (scheduler_FCFS_t*)schedulerInfo;

    list_destroy(info->FCFS_list);
    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerFCFSScheduleJob(void* schedulerInfo, scheduler_t* scheduler, job_t* job, uint64_t currentTime)
{
    scheduler_FCFS_t* info = (scheduler_FCFS_t*)schedulerInfo;
    list_t* list = info->FCFS_list;

    list_insert(list, (void*)job); 
    //if the inserted job is the only job in queue then we schedule its completion
    if(list_next(list_head(list)) == NULL || info->curr_job == NULL)
    {
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(job);
        info->curr_job = job; 
        schedulerScheduleNextCompletion(scheduler, job_completion_time); 
    }
}

// Called to complete a job in response to an earlier call to schedulerScheduleNextCompletion
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// currentTime - the current simulated time
// Returns the job that is being completed
job_t* schedulerFCFSCompleteJob(void* schedulerInfo, scheduler_t* scheduler, uint64_t currentTime)
{
    scheduler_FCFS_t* info = (scheduler_FCFS_t*)schedulerInfo;
    list_t* list = info->FCFS_list; 
    job_t* completed_job = info->curr_job;  
    
    list_remove(list, list_find(list, completed_job));
    if(list_tail(list) != NULL)
    {
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(list_data(list_tail(list)));
        info->curr_job = list_data(list_tail(list)); 
        schedulerScheduleNextCompletion(scheduler, job_completion_time); 
    }
    else
    {
        info->curr_job = NULL; 
    }
    
    return completed_job;
}
