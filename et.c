#include <err.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libnotify/notify.h>
#include <sys/time.h>

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
  (void) (sig);
  timer_state = false;
}

void status_request_handler(int sig) {
  status_state = true;
  /* Let the signal handler re-register itself. */
  if (signal(sig, status_request_handler) == SIG_ERR)
    err(EXIT_FAILURE, "signal");
}

void kill_handler(int sig) {
  (void) (sig);
  kill_state = true;
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
  GError *err = NULL;

  noti = notify_notification_new(instance_name, msg, NULL);
  notify_notification_set_urgency(noti, urgency);
  if (!notify_notification_show(noti, &err)) {
    warnx("notify_notification_show: %s", err ? err->message : "unknown error");
    g_clear_error(&err);
  }

  g_object_unref(G_OBJECT(noti));
}

/* Writes a human-readable timestamp into msg. */
void create_text_timestamp(char* msg, size_t msg_size, time_t sec) {
  struct tm *time;
  char *msg_format;

  if (!(msg_format = malloc(64)))
    err(EXIT_FAILURE, "malloc");

  /* Select a fitting strftime-format-string based on sec's value. */
  if (sec >= 60 * 60 * 24)
    (void) snprintf(msg_format, 64, "Still %%j days and %%T…");
  else if (sec >= 60 * 60)
    (void) snprintf(msg_format, 64, "Still %%T…");
  else if (sec >= 60)
    (void) snprintf(msg_format, 64, "%%M minutes and %%S seconds remaining");
  else
    (void) snprintf(msg_format, 64, "Only %%S seconds remaining");

  /* Convert sec to a timestamp and write the timestamp into msg. */
  if (!(time = gmtime(&sec)))
    err(EXIT_FAILURE, "gmtime");

  if (!strftime(msg, msg_size, msg_format, time))
    errx(EXIT_FAILURE, "strftime");

  free(msg_format);
}

/* Function to be called from within the main-loop. */
void status_request_worker(void) {
  char *msg;

  if (!(msg = malloc(96)))
    err(EXIT_FAILURE, "malloc");

  if (getitimer(ITIMER_REAL, &timer))
    err(EXIT_FAILURE, "getitimer");

  create_text_timestamp(msg, 96, timer.it_value.tv_sec);

  printf("%s\n", msg);
  show_notification(msg, NOTIFY_URGENCY_NORMAL);

  free(msg);
}

/* Function to parse h:m:s, m:s and s. */
unsigned int parse_time(char *time_str) {
  unsigned int vals[3], sum;
  int ret, i;

  ret = sscanf(time_str, "%u:%u:%u", &vals[0], &vals[1], &vals[2]);
  for (i = sum = 0; i < ret; i++)
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
  struct tm end_tm;
  time_t end_time;
  unsigned int sec;
  char *start_msg, *start_msg_time;

#ifdef __OpenBSD__
  /* libnotify calls dbus-launch and creates temp files. It's something. */
  if (unveil("/", "rwc") == -1 ||
      unveil("/usr/local/bin/dbus-launch", "rx") == -1 ||
      unveil(NULL, NULL) == -1)
    err(EXIT_FAILURE, "unveil");

  if (pledge("stdio rpath cpath inet unix proc exec", NULL) == -1)
    err(EXIT_FAILURE, "pledge");
#endif /* __OpenBSD__ */

  if (!notify_init("et") || !notify_is_initted())
    errx(EXIT_FAILURE, "initializing libnotify");

  if (argc < 2 || argc > 3)
    print_help(argv[0], EXIT_FAILURE);

  if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))
    print_help(argv[0], EXIT_SUCCESS);

  sec = parse_time(argv[1]);
  instance_name = (argc == 2) ? "egg timer" : argv[2];

  /* There may be obscure timing issues otherwise.. */
  if (sec == 0)
    exit(EXIT_SUCCESS);

  if (sec > 60 * 60 * 24 * 365)
    errx(EXIT_FAILURE, "time must be in [0sec, 365days]");

#ifdef __OpenBSD__
  /* Establish a dbus connection, then drop permissions. */
  if (!notify_get_server_info(NULL, NULL, NULL, NULL))
    errx(EXIT_FAILURE, "notify_get_server_info");

  if (pledge("stdio", NULL) == -1)
    err(EXIT_FAILURE, "pledge");
#endif /* __OpenBSD__ */

  /* Enable signal handlers */
  if (signal(SIGALRM, time_over_handler)      == SIG_ERR ||
      signal(SIGUSR1, status_request_handler) == SIG_ERR ||
      signal(SIGINT,  kill_handler)           == SIG_ERR ||
      signal(SIGHUP,  kill_handler)           == SIG_ERR)
    err(EXIT_FAILURE, "signal");

  if (!(start_msg = malloc(96)) || !(start_msg_time = malloc(110)))
    err(EXIT_FAILURE, "malloc");

  /* Start timer */
  timer = create_itimerval(sec);
  if (setitimer(ITIMER_REAL, &timer, NULL))
    err(EXIT_FAILURE, "setitimer");

  /* Calculate when the timer will end */
  end_time = time(NULL) + sec;
  end_tm = *localtime(&end_time);

  create_text_timestamp(start_msg_time, 96, timer.it_value.tv_sec);
  printf("Egg timer started!\n"
         "Name: %s, Ending at: %02d:%02d:%02d, PID: %d\n%s\n",
    instance_name,
    end_tm.tm_hour, end_tm.tm_min, end_tm.tm_sec,
    getpid(),
    start_msg_time);

  (void) snprintf(start_msg, 96, "Timer started\n%s", start_msg_time);
  show_notification(start_msg, NOTIFY_URGENCY_LOW);

  free(start_msg);
  free(start_msg_time);

  do {
    /* Pause until next signal comes by… */
    pause();

    /* Print the status to the stdout if it was requested. */
    if (status_state) {
      status_request_worker();
      status_state = false;
    }
    if (kill_state) {
      show_notification("Timer was killed!", NOTIFY_URGENCY_NORMAL);
      errx(EXIT_FAILURE, "timer was killed!");
    }
  } while (timer_state);

  show_notification("Time is up!", NOTIFY_URGENCY_CRITICAL);

  notify_uninit();

  exit(EXIT_SUCCESS);
}
