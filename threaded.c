/*
** This is an example of how to use MY-BASIC with multiple threads.
**
** I have disabled MB_ENABLE_UNICODE and MB_ENABLE_UNICODE_ID, otherwise calling
** "setlocale" in "_print_string" may cause deadlocks.
**
** For info about the interpreter itself, see https://github.com/paladin-t/my_basic/
*/

#define  THREAD_IMPLEMENTATION
#ifndef MB_CP_VC
#	include <stdint.h>
#endif /* MB_CP_VC */
#include <stdio.h>
#include <string.h>
#include "core/my_basic.h"
#include "libs/thread.h"

#define THREAD_COUNT 10

// Delay for a certain period.
static int delay(struct mb_interpreter_t* s, void** l) {
	int result = MB_FUNC_OK;
	int_t ms = 0;
	thread_timer_t tmr;

	mb_check(mb_attempt_func_begin(s, l));

	mb_check(mb_pop_int(s, l, &ms));

	mb_check(mb_attempt_func_end(s, l));

	thread_timer_init(&tmr);
	thread_timer_wait(&tmr, (THREAD_U64)ms * 1000000);
	thread_timer_term(&tmr);

	return result;
}

// Print something.
static int talk(struct mb_interpreter_t* s, void** l) {
	int result = MB_FUNC_OK;
	void* data = NULL;

	mb_check(mb_attempt_func_begin(s, l));

	mb_check(mb_attempt_func_end(s, l));

	mb_get_userdata(s, &data);

	printf("THREAD %d\n", (int)(intptr_t)data);

	return result;
}

// Thread procedure for fully created interpreter instance.
static int threaded_full(void* data) {
	struct mb_interpreter_t* bas = NULL;
	int n = (int)(intptr_t)data;
	char code[32] = { '\0' };

	sprintf(code, "delay %d\ntalk", n * 1000);

	// A whole open/reg/load/run/close workflow. Each instance has its own context
	// as an isolated one.
	mb_open(&bas);
	mb_set_userdata(bas, data);
	mb_reg_fun(bas, delay);
	mb_reg_fun(bas, talk);
	mb_load_string(bas, code, true);
	mb_run(bas, true);
	mb_close(&bas);

	return 0;
}

// Thread procedure for forked interpreter instance.
static int threaded_fork(void* data) {
	struct mb_interpreter_t* bas = NULL;
	struct mb_interpreter_t* src = NULL;

	src = (struct mb_interpreter_t*)data;

	// A forked workflow. Shares the same registered functions, parsed code, etc.
	// but uses its own running context.
	// It's not fully supported to run forked instances with multiple threads,
	// cannot use referenced GC types in code, but simple data types are OK.
	mb_fork(&bas, src);
	// IMPORTANT: pass "false" to "clear_parser" to avoid different threads
	// writing to same memory, it will be cleared when calling "mb_close" later.
	mb_run(bas, false);
	mb_join(&bas);

	return 0;
}

int main(int argc, char* argv[]) {
	struct mb_interpreter_t* bas = NULL;
	thread_ptr_t threads[THREAD_COUNT];
	int i = 0;

	// Call this only once per progress.
	mb_init();

	// Prepare an interpreter instance for further forking.
	mb_open(&bas);
	mb_set_userdata(bas, (void*)(intptr_t)-1);
	mb_reg_fun(bas, delay);
	mb_reg_fun(bas, talk);
	mb_load_string(bas, "delay 100\nprint \"FORKED\"+chr(10)", true);

	// Fork from a preloaded interpreter instance.
	for (i = THREAD_COUNT / 2; i < THREAD_COUNT; ++i)
		threads[i] = thread_create(threaded_fork, (void*)bas, "THREADED FORK", 0);
	// Create new interpreter instances.
	for (i = 0; i < THREAD_COUNT / 2; ++i)
		threads[i] = thread_create(threaded_full, (void*)(intptr_t)i, "THREADED FULL", 0);

	// Wait until all threads terminated.
	for (i = 0; i < THREAD_COUNT; ++i)
		thread_join(threads[i]);

	// Close the shared interpreter instance after all forked ones are joined.
	mb_close(&bas);

	// Call this only once per progress.
	mb_dispose();

	return 0;
}
