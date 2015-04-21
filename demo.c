/*COPYRIGHT (C) 2014 - 2020, piggy_xrh */

#include <stdio.h>

#include "stpool.h"

#ifdef _WIN32
#include <Windows.h>
#pragma comment(lib, "stpool.lib")
#define msleep Sleep
#else
#define msleep(x) usleep(x * 1000)
#endif

static int g_test_reschedule = 0;

int task_run(struct sttask_t *ptsk) {
	/* TO DO */
	printf("@task_run ...:%d\n", *(int *)ptsk->task_arg);	

	++ *(int *)ptsk->task_arg;
	//msleep(3000);
	
	return 0;
}

void task_complete(struct sttask_t *ptsk, long vmflags , int task_code) {		
	/* NOTE:
	 * 	   If vmflags has been marked with STTASK_VMARK_DONE, it indicates that 
	 * 	 the @task_run has been excuted by the pool.
	 */
	if (!(STTASK_VMARK_DONE & vmflags)) {
		printf("@task_run is not executed: 0x%lx-code:%d\n",
			   vmflags, task_code /* STPOOL_XX */);	
		
		return;
	}
		
	if (g_test_reschedule) {
		struct schattr_t attr;
		/* We can adjust the task's priority */
		stpool_task_getschattr(ptsk, &attr);
		if (!attr.permanent) {
			attr.permanent = 1;
			attr.sche_pri  = 80;
			stpool_task_setschattr(ptsk, &attr);
		}
		
		/* Reschedule the task */
		stpool_add_task(ptsk->hp_last_attached, ptsk);
		
		/* We sleep for a while to slow down the test */
		msleep(1500);
	}	
}

static int counter = 0;

int task_run2(struct sttask_t *ptsk) {
	static int i=0;

	printf("@task_run2: %d\n", ++i);
	return 0;
}

void task_complete2(struct sttask_t *ptsk, long vmflags, int task_code) {	
	if (!(STTASK_VMARK_DONE & vmflags)) {
		printf("@task_run2 is not executed: 0x%lx-code:%d\n",
			vmflags, task_code);
		return;
	}
	
	if (g_test_reschedule) {
		stpool_add_task(ptsk->hp_last_attached, ptsk);
		msleep(1500);
	}
}

long mark_walk(struct stpool_tskstat_t *stat, void *arg) {
	/* If you want to stop walking the task, you should return -1 */
	//return -1;
	
	/* If you just want to walk the tasks, you should return 0 */
	//return 0;

	/* Return the marks */
	return STTASK_VMARK_REMOVE_BYPOOL /* Remove the task */
		   ;
}

int main() 
{
	int i, error;
	HPOOL hp;
	struct schattr_t attr = {
		0, 1, STP_SCHE_TOP
	};	
	struct sttask_t *ptsk;
	
	/* NO buffer */
	setbuf(stdout, 0);

	/* Create a pool */
	hp = stpool_create(20, /*limited threads number*/
				       0,  /*number of threads reserved to waiting for tasks*/
				       0,  /*do not suspend the pool */
				       1   /*priority queue num */
					   );
	
	/* Set the sleep time for the threads (10s + random() % 25s)*/
	stpool_set_activetimeo(hp, 10);
	
	/* Print the status of the pool */
	printf("@tpool_create(20, 0, 0, 10)\n%s\n", stpool_status_print(hp, NULL, 0));
		
	/************************************************************/
	/********************Test @stpool_adjust(_abs)****************/
	/************************************************************/
	printf("\nPress any key to test the @tpool_adjust(abs) ....\n");
	getchar();
	stpool_adjust_abs(hp, 300, 1);
	printf("@tpool_adjust_abs(pool, 300, 1)\n%s\n", stpool_status_print(hp, NULL, 0));
	
	/* We call @stpool_adjust to recover the pool env */
	stpool_adjust(hp, -280, -1);
	stpool_adjust_wait(hp);
	printf("@tpool_adjust(pool, -280, -1)\n%s\n", stpool_status_print(hp, NULL, 0));
	
	/*******************************************************************/
	/********************Test the throttle******************************/
	/*******************************************************************/
	printf("\nPress any key to test the throttle ....\n");
	getchar();
	/* Turn the throttle on */
	stpool_throttle_enable(hp, 1);
	ptsk = stpool_task_new("test", task_run, task_complete, (void *)&counter);
	error = stpool_add_task(hp, ptsk);
	if (error)
		printf("***@stpool_add_task error:%d\n", error);
	/* Turn the throttle off */
	stpool_throttle_enable(hp, 0);
	
	/*******************************************************************/
	/******************Test the priority********************************/
	/*******************************************************************/
	printf("\nPress any key to test the priority ....\n");
	getchar();
	
	stpool_suspend(hp, 0);
	/* Add a task with zero priority, and the task will be pushed into the 
	 * lowest priority queue. 
	 */
	stpool_add_routine(hp, "test", task_run, task_complete, (void *)&counter, NULL);
		
	/* @task_run2 will be scheduled prior to the @task_run since the @task_run2 has
	 * a higher priority.
	 */
	stpool_add_routine(hp, "routine", task_run2, task_complete2, NULL, &attr); 
	
	/* Wake up the pool to schedule the tasks */
	stpool_resume(hp);

	/* Wait for all tasks' being done completely */
	stpool_task_wait(hp, NULL, -1);
	
	/******************************************************************/
	/****************Test rescheduling task****************************/
	/******************************************************************/
	printf("\nPress any key to test reschedule task. <then press key to stop testing.>\n");
	getchar();
	g_test_reschedule = 1;
	stpool_add_task(hp, ptsk);
	stpool_add_routine(hp, "routine", task_run2, task_complete2, NULL, NULL);
	
	getchar();
	g_test_reschedule = 0;	
	stpool_task_wait(hp, NULL, -1);
	
	/******************************************************************/
	/***************Test running amount of tasks***********************/
	/******************************************************************/
	printf("\nPress any key to add tasks ... <then can press any key to remove them.>\n");
	getchar();
	
	/* We can suspend the pool firstly, and then resume the pool after delivering our
	 * tasks into the pool, It'll be more effecient to do it like that if there are 
	 * a large amount of tasks that will be added into the pool.
	 */
	/* NOTE: We can add the same task into the pool one more times */
	for (i=0; i<4000; i++) {
		stpool_add_routine(hp, "test", task_run, task_complete, (void *)&counter, NULL);
		stpool_add_routine(hp, "routine", task_run2, task_complete2, NULL, NULL);
	}
	
	/****************************************************************/
	/*************Test stoping all tasks fastly**********************/
	/****************************************************************/
	printf("\nPress any key to test stoping all tasks fastly.\n");
	getchar();

	/* Remove all pending tasks by calling @stpool_mark_task_ex,
	 * We can also call @stpool_remove_pending_task to reach our 
	 * goal, But we call @stpool_mark_task_ex here for showing 
	 * how to use @stpool_mark_task to do the customed works.
	 */
	stpool_mark_task_ex(hp, mark_walk, NULL);
	stpool_throttle_enable(hp, 1);

	/* Wait for all tasks' being done */
	stpool_task_wait(hp, NULL, -1);
	printf("---------------------------tasks have been finished.\n");
	
	/* Free the task object */
	stpool_task_delete(ptsk);
		
	/* Release the pool */
	printf("%s\n", stpool_status_print(hp, NULL, 0));
	stpool_release(hp);
	printf("Press any key to exit ...\n");
	getchar();
	
	return 0;
}


