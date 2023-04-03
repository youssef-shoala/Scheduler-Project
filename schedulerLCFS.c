#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

// LCFS scheduler info
typedef struct {
    list_t* LCFS_list; 
    job_t* curr_job;
} scheduler_LCFS_t;

// Compare function designed for FCFS to be given to linked list
int LCFS_compare(void* data1, void* data2)
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
void* schedulerLCFSCreate()
{
    scheduler_LCFS_t* info = malloc(sizeof(scheduler_LCFS_t));
    if (info == NULL) {
        return NULL;
    }

    info->LCFS_list = list_create(LCFS_compare); 
    info->curr_job = NULL; 
    return info;
}

// Destroys scheduler specific info
void schedulerLCFSDestroy(void* schedulerInfo)
{
    scheduler_LCFS_t* info = (scheduler_LCFS_t*)schedulerInfo;

    list_destroy(info->LCFS_list); 
    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerLCFSScheduleJob(void* schedulerInfo, scheduler_t* scheduler, job_t* job, uint64_t currentTime)
{
    scheduler_LCFS_t* info = (scheduler_LCFS_t*)schedulerInfo;
    list_t* list = info->LCFS_list; 

    list_insert(list, (void*)job); 
    //if the inserted job is the only job in queue then we scheule its completion
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
job_t* schedulerLCFSCompleteJob(void* schedulerInfo, scheduler_t* scheduler, uint64_t currentTime)
{
    scheduler_LCFS_t* info = (scheduler_LCFS_t*)schedulerInfo;
    list_t* list = info->LCFS_list; 
    job_t* completed_job = info->curr_job; 

    list_remove(list, list_find(list, completed_job)); 
    if(list_tail(list) != NULL)
    {
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(list_data(list_head(list))); 
        info->curr_job = list_data(list_head(list)); 
        schedulerScheduleNextCompletion(scheduler, job_completion_time); 
    }
    else
    {
        info->curr_job = NULL; 
    }

    return completed_job;
}
