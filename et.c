#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libnotify/notify.h>
#include <sys/time.h>

/* Macro to "use" unused parameters */
#define UNUSED(x) (void)(x)

/* There's too much failure ;_; */
#define DIE(x) { printf(x); exit(EXIT_FAILURE); }

/* Flag variables */
volatile bool timer_state  = true;
volatile bool status_state = false;
volatile bool kill_state   = false;

/* Variable to store the timer's values. */
struct itimerval timer;

/* Optional describing name of this et-instance. */
char *instance_name;

/* Signal handler-functions */
void time_over_handler(int sig) {
  timer_state = false;
  UNUSED(sig);
}

void status_request_handler(int sig) {
  status_state = true;
  /* Let the signal handler re-register itself. */
  if(signal(sig, status_request_handler) == SIG_ERR)
    DIE("Could not re-register signal handler.\n");
}

void kill_handler(int sig) {
  kill_state = true;
  UNUSED(sig);
}

/* Function to create an itimerval for sec seconds. */
struct itimerval create_itimerval(unsigned int sec) {
  struct itimerval i;
  i.it_interval.tv_usec = 0;
  i.it_interval.tv_sec  = 0;
  i.it_value.tv_usec    = 0;
  i.it_value.tv_sec     = sec;
  return i;
}

/* Function to show a notification. */
void show_notification(char *msg, NotifyUrgency urgency) {
  NotifyNotification *noti;

  noti = notify_notification_new(instance_name, msg, NULL);
  notify_notification_set_urgency(noti, urgency);
  if(!notify_notification_show(noti, NULL))
    printf("Failed to show a NotifyNotification!\n");

  g_object_unref(G_OBJECT(noti));
}

/* Writes a human-readable timestamp into msg. */
void create_text_timestamp(char* msg, size_t msg_size, time_t sec) {
  struct tm *time;
  char *msg_format;

  if(!(msg_format = malloc(64))) 
    DIE("Could not malloc.\n");

  /* Select a fitting strftime-format-string based on sec's value. */
  if(sec >= 60 * 60 * 24)
    snprintf(msg_format, 64, "Still %%j days and %%T…");
  else if(sec >= 60 * 60)
    snprintf(msg_format, 64, "Still %%T…");
  else if(sec >= 60)
    snprintf(msg_format, 64, "%%M minutes and %%S seconds remaining");
  else
    snprintf(msg_format, 64, "Only %%S seconds remaining");

  /* Convert sec to a timestamp and write the timestamp into msg. */
  time = gmtime(&sec);
  strftime(msg, msg_size, msg_format, time);

  free(msg_format);
}

/* Function to be called from within the main-loop. */
void status_request_worker() {
  char *msg;

  if(!(msg = malloc(96)))
    DIE("Could not malloc.\n");

  if(getitimer(ITIMER_REAL, &timer))
    DIE("Could not getitimer.\n");

  create_text_timestamp(msg, 96, timer.it_value.tv_sec);

  printf("%s\n", msg);
  show_notification(msg, NOTIFY_URGENCY_NORMAL);

  free(msg);
}

/* Function to parse h:m:s, m:s and s. */
unsigned int parse_time(char *time_str) {
  unsigned int vals[3], ret, sum, i;

  ret = sscanf(time_str, "%u:%u:%u", &vals[0], &vals[1], &vals[2]);
  for(i = sum = 0; i < ret; i++)
    sum = sum * 60 + vals[i];

  return sum;
}

/* Prints help to stdout and exits. */
void print_help(char *et_name, int exit_status) {
  printf("Usage: %s TIME [NAME]\n", et_name);
  printf("  TIME is a timestamp formated like H:M:S, M:S or S.\n");
  printf("  NAME is an optional identifier.\n\n");
  exit(exit_status);
}

int main(int argc, char *argv[]) {
  unsigned int sec;
  char *start_msg;

  if(!notify_init("et") || !notify_is_initted())
    DIE("Could not initialize libnotify.\n");

  if(argc < 2 || argc > 3)
    print_help(argv[0], EXIT_FAILURE);

  if(!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
    print_help(argv[0], EXIT_SUCCESS);

  sec = parse_time(argv[1]);
  instance_name = (argc == 2) ? "egg timer" : argv[2];

  /* There may be obscure timing issues otherwise.. */
  if(sec == 0)
    exit(EXIT_SUCCESS);

  if(sec > 60 * 60 * 24 * 365)
    DIE("Time must be in [0sec, 365days]\n");

  /* Enable signal handlers */
  if(signal(SIGALRM, time_over_handler)      == SIG_ERR ||
     signal(SIGUSR1, status_request_handler) == SIG_ERR ||
     signal(SIGINT,  kill_handler)           == SIG_ERR ||
     signal(SIGHUP,  kill_handler)           == SIG_ERR)
    DIE("Could not register signal handlers.\n");

  if(!(start_msg = malloc(96)))
    DIE("Could not malloc.\n");

  /* Start timer */
  timer = create_itimerval(sec);
  if(setitimer(ITIMER_REAL, &timer, NULL))
    DIE("Could not setitimer.\n");

  create_text_timestamp(start_msg, 96, timer.it_value.tv_sec);
  printf("Egg timer started!\nName: %s, PID: %d\n%s\n",
    instance_name, getpid(), start_msg);
  show_notification("Timer started", NOTIFY_URGENCY_LOW);

  do {
    /* Pause until next signal comes by… */
    pause();

    /* Print the status to the stdout if it was requested. */
    if(status_state) {
      status_request_worker();
      status_state = false;
    }
    if(kill_state) {
      show_notification("Timer was killed!", NOTIFY_URGENCY_NORMAL);
      DIE("Timer was killed!");
    }
  } while(timer_state);

  show_notification("Time is up!", NOTIFY_URGENCY_CRITICAL);

  notify_uninit();

  exit(EXIT_SUCCESS);
}
