#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

#include<stdio.h>

// PSJF scheduler info
typedef struct {
    list_t* PSJF_list; 
    job_t* curr_job; 
    uint64_t last_working_time; 
} scheduler_PSJF_t;

int PSJF_compare(void* data1, void* data2)
{
    uint64_t job_time1 = jobGetRemainingTime(data1);
    uint64_t job_time2 = jobGetRemainingTime(data2);
    if (job_time1 == job_time2) {
        return 0;
    } else if (job_time1 > job_time2) {
        return -1;
    }
    return 1; 
}

// Creates and returns scheduler specific info
void* schedulerPSJFCreate()
{
    scheduler_PSJF_t* info = malloc(sizeof(scheduler_PSJF_t));
    if (info == NULL) {
        return NULL;
    }

    info->PSJF_list = list_create(PSJF_compare); 
    info->curr_job = NULL; 
    return info;
}

// Destroys scheduler specific info
void schedulerPSJFDestroy(void* schedulerInfo)
{
    scheduler_PSJF_t* info = (scheduler_PSJF_t*)schedulerInfo;

    list_destroy(info->PSJF_list); 
    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerPSJFScheduleJob(void* schedulerInfo, scheduler_t* scheduler, job_t* job, uint64_t currentTime)
{
    scheduler_PSJF_t* info = (scheduler_PSJF_t*)schedulerInfo;
    list_t* list = info->PSJF_list; 

    list_insert(list, (void *)job); 

    printf("Job being scheduled: %ld jobtime: %ld remtime:%ld\n", jobGetId(job), jobGetJobTime(job), jobGetRemainingTime(job)); 
    //print_linked_list(info->PLCFS_list);

    //if no other job is being done, start the incoming job
    if(list_next(list_head(list)) == NULL || info->curr_job == NULL)
    {
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(job); 
        info->curr_job = job; 
        info->last_working_time = currentTime; 
        schedulerScheduleNextCompletion(scheduler, job_completion_time);
    }
    //otherwise there is currently a job being run. If job_time(new_job)<job_remaining_time(old_job) then we switch jobs
    else if(jobGetRemainingTime(job) < jobGetRemainingTime(info->curr_job))
    {
        //update current jobs remaining time and cancel its completion 
        jobSetRemainingTime(info->curr_job, jobGetRemainingTime(info->curr_job)-(currentTime - info->last_working_time)); 
        schedulerCancelNextCompletion(scheduler); 

        //start new job
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(job); 
        info->curr_job = job; 
        info->last_working_time = currentTime; 
        schedulerScheduleNextCompletion(scheduler, job_completion_time);
    }
}

// Called to complete a job in response to an earlier call to schedulerScheduleNextCompletion
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// currentTime - the current simulated time
// Returns the job that is being completed
job_t* schedulerPSJFCompleteJob(void* schedulerInfo, scheduler_t* scheduler, uint64_t currentTime)
{
    scheduler_PSJF_t* info = (scheduler_PSJF_t*)schedulerInfo;
    list_t* list = info->PSJF_list; 
    job_t* completed_job = info->curr_job; 

    printf("Job being completed: %ld \n", jobGetId(completed_job)); 

    list_remove(list, list_find(list, completed_job));
    if(list_head(list) != NULL)
    {
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(list_data(list_tail(list))); 
        info->curr_job = list_data(list_tail(list)); 
        info->last_working_time = currentTime; 
        schedulerScheduleNextCompletion(scheduler, job_completion_time); 
    }
    else
    {
        info->curr_job = NULL; 
    }
    return completed_job;
}
