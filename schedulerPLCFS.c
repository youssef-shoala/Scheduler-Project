#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

// PLCFS scheduler info
typedef struct {
    list_t* PLCFS_list; 
    job_t* curr_job;
    uint64_t last_working_time; 
} scheduler_PLCFS_t;

// Compare function designed for FCFS to be given to linked list
int PLCFS_compare(void* data1, void* data2)
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
void* schedulerPLCFSCreate()
{
    scheduler_PLCFS_t* info = malloc(sizeof(scheduler_PLCFS_t));
    if (info == NULL) {
        return NULL;
    }

    info->PLCFS_list = list_create(PLCFS_compare);
    info->curr_job = NULL; 
    return info;
}

// Destroys scheduler specific info
void schedulerPLCFSDestroy(void* schedulerInfo)
{
    scheduler_PLCFS_t* info = (scheduler_PLCFS_t*)schedulerInfo;

    list_destroy(info->PLCFS_list);
    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerPLCFSScheduleJob(void* schedulerInfo, scheduler_t* scheduler, job_t* job, uint64_t currentTime)
{
    scheduler_PLCFS_t* info = (scheduler_PLCFS_t*)schedulerInfo;
    list_t* list = info->PLCFS_list; 

    if(info->curr_job != NULL){
        jobSetRemainingTime(info->curr_job, jobGetRemainingTime(info->curr_job)-(currentTime - info->last_working_time)); 
        schedulerCancelNextCompletion(scheduler);
    }
    list_insert(list, (void*)job);
    uint64_t job_completion_time = currentTime + jobGetRemainingTime(job); 
    info->curr_job = job; 
    info->last_working_time = currentTime; 
    schedulerScheduleNextCompletion(scheduler, job_completion_time);
}

// Called to complete a job in response to an earlier call to schedulerScheduleNextCompletion
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// currentTime - the current simulated time
// Returns the job that is being completed
job_t* schedulerPLCFSCompleteJob(void* schedulerInfo, scheduler_t* scheduler, uint64_t currentTime)
{
    scheduler_PLCFS_t* info = (scheduler_PLCFS_t*)schedulerInfo;
    list_t* list = info->PLCFS_list; 
    job_t* completed_job = info->curr_job; 

    list_remove(list, list_find(list, completed_job)); 
    if(list_head(list)!=NULL)
    {
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(list_data(list_head(list))); 
        info->curr_job = list_data(list_head(list)); 
        info->last_working_time = currentTime; 
        schedulerScheduleNextCompletion(scheduler, job_completion_time);
    }
    else
    {
        info->curr_job = NULL; 
    }
    return completed_job;
}
