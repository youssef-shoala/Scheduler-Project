#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

#include "stdio.h"

// SRPT scheduler info
typedef struct {
    list_t* SRPT_list; 
    job_t* curr_job; 
    uint64_t last_working_time; 
} scheduler_SRPT_t;

int SRPT_compare(void* data1, void* data2)
{
    uint64_t proc_time1 = jobGetRemainingTime(data1);
    uint64_t proc_time2 = jobGetRemainingTime(data2);
    if (proc_time1 == proc_time2) {
        return 0;
    } else if (proc_time1 > proc_time2) {
        return -1;
    }
    return 1; 
}


// Creates and returns scheduler specific info
void* schedulerSRPTCreate()
{
    scheduler_SRPT_t* info = malloc(sizeof(scheduler_SRPT_t));
    if (info == NULL) {
        return NULL;
    }

    info->SRPT_list = list_create(SRPT_compare); 
    info->curr_job = NULL; 
    return info;
}

// Destroys scheduler specific info
void schedulerSRPTDestroy(void* schedulerInfo)
{
    scheduler_SRPT_t* info = (scheduler_SRPT_t*)schedulerInfo;

    list_destroy(info->SRPT_list); 
    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerSRPTScheduleJob(void* schedulerInfo, scheduler_t* scheduler, job_t* job, uint64_t currentTime)
{
    scheduler_SRPT_t* info = (scheduler_SRPT_t*)schedulerInfo;
    list_t* list = info->SRPT_list; 

    list_insert(list, (void *)job); 

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
        list_remove(list, list_find(list, (void *)job));
        list_insert(list, (void *)job); 

        //start new job
        uint64_t job_completion_time = currentTime + jobGetRemainingTime(job); 
        info->curr_job = job; 
        info->last_working_time = currentTime; 
        schedulerScheduleNextCompletion(scheduler, job_completion_time);
    }

    printf("schedule job - curr list time: %ld\n", currentTime);
    list_node_t* curr_node = list_head(list); 
    while(curr_node)
    {
        printf("%ld, rem: %ld \n", jobGetId(list_data(curr_node)), jobGetRemainingTime(list_data(curr_node)));
        curr_node = list_next(curr_node); 
    }
    printf("-----END LIST\n\n\n");
}

// Called to complete a job in response to an earlier call to schedulerScheduleNextCompletion
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// currentTime - the current simulated time
// Returns the job that is being completed
job_t* schedulerSRPTCompleteJob(void* schedulerInfo, scheduler_t* scheduler, uint64_t currentTime)
{
    scheduler_SRPT_t* info = (scheduler_SRPT_t*)schedulerInfo;
    list_t* list = info->SRPT_list; 
    job_t* completed_job = info->curr_job; 

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

    printf("complete job - curr list time: %ld\n", currentTime);
    list_node_t* curr_node = list_head(list); 
    while(curr_node)
    {
        printf("%ld, rem: %ld \n", jobGetId(list_data(curr_node)), jobGetRemainingTime(list_data(curr_node)));
        curr_node = list_next(curr_node); 
    }
    printf("-----END LIST\n\n\n");

    return completed_job;
}
