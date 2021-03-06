/* CMPSC 311 Project 6 starter kit, version 3
 *
 * Modify Project 6 starter kit, version 2
 *   write and install signal handlers
 *   Sleep() = sleep() with alarm()
 *   test installed handlers (follow the program's instructions)
 * 
 * Erica Hollis
 * CmpSci 311, Project 6
 * pr6.4.c
 * October 2010
 * erh5000@psu.edu
 *
 */

/*----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "pr6_ctime.h"
#include "pr6_signal.h"
#include "pr6_wait.h"

/* for use with getopt(3) */
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;

/* debug option, with -v */
int pr6_debug = 0;

/* upper limit on -c option (this is a temporary version) */
#define MAX_CHILDREN 8

/*----------------------------------------------------------------------------*/

static void usage(char *prog, int status)
{
  if (status == EXIT_SUCCESS)
    {
      printf("Usage: %s [-h] [-v] [-a n] [-b n] [-c n] [-f n] [-s n] [-t n] [-x n]\n",
        prog);
      printf("    -h      help\n");
      printf("    -v      verbose mode\n");
      printf("    -a n    child alarm time interval, default 0\n");
      printf("    -b n    parent alarm time interval, default 0\n");
      printf("    -c n    fork() n child processes, default 0, max %d\n", MAX_CHILDREN);
      printf("    -f n    fflush() before fork(), 0 = no, 1 = yes, default 1\n");
      printf("    -s n    child sleep time, default 0\n");
      printf("    -t n    parent sleep time, default 0\n");
      printf("    -x n    child exit status n, default 0\n");
    }
  else
    {
      fprintf(stderr, "%s: Try '%s -h' for usage information.\n", prog, prog);
    }

  exit(status);
}

/* conditional is expected to be true
 * return termination indicator (1) if there is a fatal problem, otherwise return 0
 */
static int check_arg(char *prog, char *option, int val, int fatal, int conditional)
{
  if (!conditional)
    {
      if (fatal)
        {
          fprintf(stderr, "%s: %s %d out of range\n", prog, option, val);
          return 1;
        }
      else
        {
          fprintf(stderr, "%s: %s %d out of range (ignored)\n", prog, option, val);
          return 0;
        }
    }
  return 0;
}

/*----------------------------------------------------------------------------*/

static void Sleep(int alarm_time_interval, int total_sleep_time)
{
  int remaining_sleep_time = total_sleep_time;

  while (remaining_sleep_time > 0)
    {
      if (alarm_time_interval > 0)
        { alarm(alarm_time_interval); }
      remaining_sleep_time = sleep(remaining_sleep_time);
    }
}

/*----------------------------------------------------------------------------*/

/* The functions in this section are intended to demonstrate various usages and
 * properties of signals and signal handlers.  They should be removed or modified
 * in the final version of the program.
 */

static void generic_signal_handler(int sig)
{
  print_msg_1("generic_signal_handler, signal", sig);
  reinstall_signal_handler(sig, generic_signal_handler);
}

static void SIGALRM_handler(int sig)
{
  if (sig == SIGALRM)
    { 
      print_msg("alarm signal received"); 
      reinstall_signal_handler(SIGALRM, SIGALRM_handler);
    }
  else
    { print_msg_1("serious error: SIGALRM_handler, received signal", sig); }
}

static void SIGCHLD_handler(int sig)
{
  if (sig == SIGCHLD)
    { 
      print_msg("child signal received - ignored"); 
      reinstall_signal_handler(SIGCHLD, SIGCHLD_handler);
    }
  else
    { print_msg_1("serious error: SIGCHLD_handler, received signal", sig); }
  
}
static void SIGINT_handler(int sig)
{
  if (sig == SIGINT)
    { 
      print_msg("interrupt signal received - ignored"); 
      reinstall_signal_handler(SIGINT, SIGINT_handler);
    }
  else
    { print_msg_1("serious error: SIGINT_handler, received signal", sig); }
}

/* NSIG should be defined in signal.h - the number of signals on this system.
 *
 * Note that NSIG is non-standard, and will not be defined when the Posix
 * standard is enforced, as with c99 -v -D_POSIX_C_SOURCE=200112L ...
 * The substitute value 64 is too large sometimes, but that is not so much
 * of a problem.
 */

#ifndef NSIG
#define NSIG 64
#endif

static int generic_signal_handler_installed[NSIG];
  /* signal 0 is reserved
   * the default handlers for SIGKILL and SIGSTOP cannot be replaced
   */

/* try to replace every default signal handler */
static void signal_setup(void)
{
  int i, ret;

  for (i = 0; i < NSIG; i++)
    {
      ret = install_signal_handler(i, generic_signal_handler);
      generic_signal_handler_installed[i] = (ret == 0);
    }

  install_signal_handler(SIGALRM, SIGALRM_handler);
  install_signal_handler(SIGINT, SIGINT_handler);
}

static void signal_test(void)
{
  print_msg("starting - the default signal handlers are installed");

  print_msg("going to sleep for 10 seconds - try control-C or control-Z (or not)");

  Sleep(0, 10);

  print_msg("please wait while the signal handlers are changed");
  signal_setup();
  print_msg("ok - the generic signal handlers are installed");

  print_msg("going to sleep for 15 seconds - try control-C or control-Z (or not)");

  Sleep(4, 15);         /* sleep for 15 seconds, alarm every 4 seconds */

  print_msg("all done!");
}

/*----------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  /* for use with getopt(3) */
  int ch;

  /* option flags and option-arguments set from the command line */
  int verbose = 0;
  int child_alarm_time = 0;
  int parent_alarm_time = 0;
  int child_processes = 0;
  int flush_before_fork = 1;
  int child_sleep_time = 0;
  int parent_sleep_time = 0;
  int child_exit_status = EXIT_SUCCESS;

  printf("CMPSC 311 Project 6, version 3\n");

  while ((ch = getopt(argc, argv, ":hva:b:c:f:s:t:x:")) != -1)
    {
      switch (ch) {
        case 'h':
          usage(argv[0], EXIT_SUCCESS);
          break;
        case 'v':
          verbose = 1;
          pr6_debug = 1;
          break;
        case 'a':
          child_alarm_time = atoi(optarg);
          break;
        case 'b':
          parent_alarm_time = atoi(optarg);         
          break;
        case 'c':
          child_processes = atoi(optarg);
          break;
        case 'f':
          flush_before_fork = atoi(optarg);
          break;
        case 's':
          child_sleep_time = atoi(optarg);
          break;
        case 't':
          parent_sleep_time = atoi(optarg);
          break;
        case 'x':
          child_exit_status = atoi(optarg);
          break;
        case '?':
          printf("%s: invalid option '%c'\n", argv[0], optopt);
          usage(argv[0], EXIT_FAILURE);
          break;
        case ':':
          printf("%s: invalid option '%c' (missing argument)\n", argv[0], optopt);
          usage(argv[0], EXIT_FAILURE);
          break;
        default:
          usage(argv[0], EXIT_FAILURE);
          break;
      }
    }

  if (verbose == 1 || child_alarm_time != 0)
    { printf("  child_alarm_time = %d\n", child_alarm_time); }
  if (verbose == 1 || parent_alarm_time != 0)
    { printf("  parent_alarm_time = %d\n", parent_alarm_time); }
  if (verbose == 1 || child_processes != 0)
    { printf("  child_processes = %d\n", child_processes); }
  if (verbose == 1 || flush_before_fork != 1)
    { printf("  flush_before_fork = %d\n", flush_before_fork); }
  if (verbose == 1 || child_sleep_time != 0)
    { printf("  child_sleep_time = %d\n", child_sleep_time); }
  if (verbose == 1 || parent_sleep_time != 0)
    { printf("  parent_sleep_time = %d\n", parent_sleep_time); }
  if (verbose == 1 || child_exit_status != 0)
    { printf("  child_exit_status = %d\n", child_exit_status); }

  int fatal = 1;
  int terminate = 0;

  terminate += check_arg(argv[0], "-a", child_alarm_time,   fatal,
                         0 <= child_alarm_time);
  terminate += check_arg(argv[0], "-b", parent_alarm_time,  fatal,
                         0 <= parent_alarm_time);
  terminate += check_arg(argv[0], "-c", child_processes,    fatal,
                         0 <= child_processes && child_processes <= MAX_CHILDREN);
  terminate += check_arg(argv[0], "-s", child_sleep_time,   fatal,
                         0 <= child_sleep_time);
  terminate += check_arg(argv[0], "-t", parent_sleep_time,  fatal,
                         0 <= parent_sleep_time);
  terminate += check_arg(argv[0], "-x", child_exit_status, !fatal,
                         0 <= child_exit_status && child_exit_status < 256);

  if (terminate > 0)
    {
      fprintf(stderr, "%s: giving up\n", argv[0]);
      exit(EXIT_FAILURE);
    }

  init_process_table();

  // Install the signal handlers for alarm() and sigchld
  install_signal_handler(SIG_ALRM,SIGALRM_handler);
  install_signal_handler(SIGCHLD,SIGCHLD_handler);

   pid_t child_pid[child_processes];
// Fork
  while(child_processes > 0)
  {
    if (flush_before_fork)
    { 
      fflush(stdout); 
      fflush(stderr); 
    }
    child_pid[child_processes-1] = fork();
  }

  int i;
  for(i=0;i<child_processes;i++)
  {
    int child_found;
    pid_t *wait_pid;
    int *child_status;

    /* There is (should be) one more process running the same program.
     * Both processes have returned from fork(), but with different
     * values assigned to child_pid.
     */

    if (child_pid[i] == (pid_t)(-1))
      { /* This is the parent process.  The fork failed, there is no child. */
        print_msg_error("fork()", strerror(errno));

        exit(EXIT_FAILURE);
      }

    else if (child_pid[i] == 0)
      { /* This is the child process.  The fork succeeded. */

        //Send the alarm time and sleep times to the Sleep function
        if(child_alarm_time>0 || child_sleep_time>0)
          Sleep(child_alarm_time,child_sleep_time);

        insert_process_table(getpid());
        print_msg("here is child %d\n",i);

        exit(child_exit_status);
       /* this will also send a SIGCHLD signal to the parent process */
    }

    else
      { /* This is the parent process.  The fork succeeded. */

        //Send the alarm time and sleep times to the Sleep function
        if(parent_alarm_time>0 || parent_sleep_time>0)
          Sleep(parent_alarm_time,parent_sleep_time);
        
        insert_process_table(getpid());
        print_msg("Here is the parent.  All children created.\n");
      }
    }
    // wait for 1 or more child processes to finish
    while(child_process>0)
    {
      if ((child_found = wait_any_child(&wait_pid,&child_status)) != 1)
        print_msg("no child processes found.\n);
      else
      {
        print_msg("child finished",wait_pid,child_status);
        update_process_table(getpid(wait_pid),child_status);
        child_process--;
      }
    }

  if verbose == 1
    print_process_table("Test");
  return EXIT_SUCCESS;
}