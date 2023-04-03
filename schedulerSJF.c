#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

// SJF scheduler info
typedef struct {
    list_t* SJF_list; 
    job_t* curr_job; 
} scheduler_SJF_t;

int SJF_compare(void* data1, void* data2)
{
    uint64_t job_time1 = jobGetJobTime(data1);
    uint64_t job_time2 = jobGetJobTime(data2);
    if (job_time1 == job_time2) {
        return 0;
    } else if (job_time1 > job_time2) {
        return -1;
    }
    return 1; 
}

// Creates and returns scheduler specific info
void* schedulerSJFCreate()
{
    scheduler_SJF_t* info = malloc(sizeof(scheduler_SJF_t));
    if (info == NULL) {
        return NULL;
    }

    info->SJF_list = list_create(SJF_compare); 
    info->curr_job = NULL; 
    return info;
}

// Destroys scheduler specific info
void schedulerSJFDestroy(void* schedulerInfo)
{
    scheduler_SJF_t* info = (scheduler_SJF_t*)schedulerInfo;

    list_destroy(info->SJF_list); 
    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerSJFScheduleJob(void* schedulerInfo, scheduler_t* scheduler, job_t* job, uint64_t currentTime)
{
    scheduler_SJF_t* info = (scheduler_SJF_t*)schedulerInfo;
    list_t* list = info->SJF_list; 

    list_insert(list, (void *)job); 
    //if the inserted job is the only job in queue then we scheule its completion
    if(list_next(list_head(list)) == NULL || info->curr_job == NULL)
    {
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(job); 
        info->curr_job = job; 
        schedulerScheduleNextCompletion(scheduler, job_completion_time); 
    }
    //if a job with the same arrival time arrives, make sure the one with the shortest job is executed
    /*
    if(jobGetArrivalTime(job) == jobGetArrivalTime(info->curr_job))
    {
        if(jobGetJobTime(job) < jobGetJobTime(info->curr_job))
        {
            //cancel curr job, its not the shortest
            schedulerCancelNextCompletion(scheduler); 
            //start job with same arrival time but shorter job time
            uint64_t job_completion_time = currentTime + jobGetRemainingTime(job); 
            info->curr_job = job; 
            schedulerScheduleNextCompletion(scheduler, job_completion_time); 
        }
    }
    */
}

// Called to complete a job in response to an earlier call to schedulerScheduleNextCompletion
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// currentTime - the current simulated time
// Returns the job that is being completed
job_t* schedulerSJFCompleteJob(void* schedulerInfo, scheduler_t* scheduler, uint64_t currentTime)
{
    scheduler_SJF_t* info = (scheduler_SJF_t*)schedulerInfo;
    list_t* list = info->SJF_list; 
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
