#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

#include "stdio.h"

// PS scheduler info
typedef struct {
    list_t* PS_list;
    uint64_t time_to_run;
    uint64_t remainder_time; 
    uint64_t last_job_run_time;
    uint64_t num_jobs;
    int running_jobs; 

    list_t* completed_jobs; 
    job_t* completed_job; 
} scheduler_PS_t;

// Creates and returns scheduler specific info
void* schedulerPSCreate()
{
    scheduler_PS_t* info = malloc(sizeof(scheduler_PS_t));
    if (info == NULL) {
        return NULL;
    }

    info->PS_list = list_create(NULL); 
    info->time_to_run = 0;
    info->remainder_time = 0; 
    info->last_job_run_time = 0; 
    info->num_jobs = 0; 
    info->running_jobs = 0; 

    info->completed_jobs = list_create(NULL); 
    info->completed_job = NULL; 
    return info;
}

// Destroys scheduler specific info
void schedulerPSDestroy(void* schedulerInfo)
{
    scheduler_PS_t* info = (scheduler_PS_t*)schedulerInfo;

    list_destroy(info->PS_list); 
    list_destroy(info->completed_jobs);
    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerPSScheduleJob(void* schedulerInfo, scheduler_t* scheduler, job_t* job, uint64_t currentTime)
{
    /*
     * Init some func vars
     */
    scheduler_PS_t* info = (scheduler_PS_t*)schedulerInfo;
    list_t* list = info->PS_list; 

    /*
     * Loop through PS_list, update each job's rem time 
     * Set = (currentTime - last_job_run_time)/num_jobs = total time taken since last job / total num jobs
     * EDIT: added cases when remainder > 0
     */
    list_node_t* curr_node = list_head(list); 
    int i = 0;
    while(curr_node != NULL)
    {
        uint64_t time_proccessed;
        if(info->remainder_time == 0)
        {
            time_proccessed = ((currentTime - info->last_job_run_time)/info->num_jobs); 
            jobSetRemainingTime(curr_node->data, jobGetRemainingTime(curr_node->data) - time_proccessed); 
            curr_node = list_next(curr_node); 
        }
        else
        {
            //first part of list will already be accoutned for in rem time - do normally
            if(i < info->remainder_time)
            {
                time_proccessed = ((currentTime - info->last_job_run_time)/info->num_jobs) + 1; 
                i++; 
            }
            //items after index [remaining time] will need a + 1 so that work time is accoutned for
            else
            {
                time_proccessed = ((currentTime - info->last_job_run_time)/info->num_jobs); 
            }

            jobSetRemainingTime(curr_node->data, jobGetRemainingTime(curr_node->data) - time_proccessed); 
            curr_node = list_next(curr_node); 
        }
    }

    /*
     * Insert new job into PS_list and update num_jobs 
     * Also init jobGetRemainingTime so that it is greater than 0
     */
    list_insert(list, job); 
    (info->num_jobs)++; 
    info->time_to_run = jobGetRemainingTime(job); 

    /*
     * Loop trhough PS_list, update time_to_run to be equal to the least job rem time of all available jobs
     */
    curr_node = list_head(list); 
    while(curr_node != NULL)
    {
        //set time_to_run to the shortest remaining job time
        if(jobGetRemainingTime(curr_node->data) < info->time_to_run)
        {
            info->time_to_run = jobGetRemainingTime(curr_node->data);
        }
        curr_node = list_next(curr_node); 
    }

    /*
     * If jobs were previously running, cancel next completion
     */
    if(info->running_jobs == 1)
    {
        schedulerCancelNextCompletion(scheduler); 
    }

    /*
     * Schedule next job for completion time: currentTime + (time to run * num jobs) + remainder time
     * Each job will run for time_to_run via processor sharing the remainder time accounts for new jobs that break the sequence
     */
    info->remainder_time = (currentTime - info->last_job_run_time) % (info->num_jobs);
    info->last_job_run_time = currentTime; 
    info->running_jobs = 1; 
    uint64_t time_to_completion = currentTime + (info->time_to_run * info->num_jobs); 
    //uint64_t time_to_completion = currentTime + (info->time_to_run * info->num_jobs) + (info->remainder_time); 
    schedulerScheduleNextCompletion(scheduler, time_to_completion);

    /*
     * Prints current PS_list
     */
    printf("schedule job - curr list time: %ld\n", currentTime);
    curr_node = list_head(list); 
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
job_t* schedulerPSCompleteJob(void* schedulerInfo, scheduler_t* scheduler, uint64_t currentTime)
{
    /*
     * Init some func vars
     */
    scheduler_PS_t* info = (scheduler_PS_t*)schedulerInfo;
    list_t* list = info->PS_list; 


    //this sets time to run to the max time to run so the min can be found later
    list_node_t* curr_node = list_head(list); 
    while(curr_node != NULL)
    {
        if(jobGetRemainingTime(curr_node->data) > info->time_to_run)
        {
            info->time_to_run = jobGetRemainingTime(curr_node->data);
        }
        
        curr_node = list_next(curr_node); 
    }
    /*
     * Loop through PS_list, update each job's rem time 
     * Set = (currentTime - last_job_run_time)/num_jobs = total time taken since last job / total num jobs
     * 
     * Loop trhough PS_list, get all completed jobs and insert them into completed_jobs
     * 
     * Also: update time_to_run to be equal to the least job rem time of all available jobs
     */
    curr_node = list_head(list); 
    while(curr_node != NULL)
    {
        uint64_t time_proccessed = ((currentTime - info->last_job_run_time)/info->num_jobs); 
        jobSetRemainingTime(curr_node->data, jobGetRemainingTime(curr_node->data) - time_proccessed); 

        if(jobGetRemainingTime(curr_node->data) == 0 && list_find(info->completed_jobs, curr_node->data) == NULL)
        {
            list_insert(info->completed_jobs, curr_node->data);
        }

        //init info->time_to_run  
        else if(jobGetRemainingTime(curr_node->data) < info->time_to_run && jobGetRemainingTime(curr_node->data) > 0)
        {
            info->time_to_run = jobGetRemainingTime(curr_node->data); 
        }

        curr_node = list_next(curr_node); 
    }
    
    /*
     * If completed_jobs, this func will collect the first completed job and return it, it will also set the next completion for the curr time 
     * if more jobs need to be returned
     */
    if(list_head(info->completed_jobs) != NULL)
    {
        list_node_t* comnpleted_job_node = list_head(info->completed_jobs); 

        info->completed_job = list_data(comnpleted_job_node); 

        list_remove(list, list_find(list, info->completed_job)); 
        info->num_jobs--;

        list_remove(info->completed_jobs, comnpleted_job_node); 

        if(list_head(info->completed_jobs) != NULL)
        {
            schedulerScheduleNextCompletion(scheduler, currentTime); 
            //schedulerPSCompleteJob(schedulerInfo, scheduler, currentTime); 
        }
    }

    /*
     * Schedule next job for completion time: currentTime + (time to run * num jobs)
     * Each job will run for time_to_run via processor sharing
     * 
     * If there are no jobs or the time to run = 0 then we will not run any jobs and update running jobs accordingly
     */
    uint64_t time_to_completion = currentTime + (info->time_to_run * info->num_jobs); 
    info->last_job_run_time = currentTime; 
    if(info->num_jobs != 0)
    {
        //run batch of jobs (running_jobs = 1)
        info->running_jobs = 1;
        info->remainder_time = 0;
        schedulerScheduleNextCompletion(scheduler, time_to_completion);
    }
    else
    {
        //no jobs being run (running_jobs = -1)
        info->running_jobs = -1; 
        info->remainder_time = 0;
    }

    /*
     * Prints current PS_list
     */
    printf("complete job - curr list time: %ld\n", currentTime);
    curr_node = list_head(list); 
    while(curr_node)
    {
        printf("%ld, rem: %ld \n", jobGetId(list_data(curr_node)), jobGetRemainingTime(list_data(curr_node)));
        curr_node = list_next(curr_node); 
    }
    printf("-----END LIST\n\n\n");

    return info->completed_job; 
}
