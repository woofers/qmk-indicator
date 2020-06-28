
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <hidapi/hidapi.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_STR 255
#define VENDOR_ID 0xCB10
#define PRODUCT_ID 0x1257
#define INTERFACE 1
#define WAIT 250 * 1000
#define PAYLOAD_SIZE 1

typedef unsigned short ushort;
typedef unsigned char byte;

enum status {
  None=1,
  Normal=10,
  Insert=11,
  Visual=12,
  Operator=13
};

static volatile int running = 1;

void exit_app() {
  running = 0;
}

void sigint_handler(int sig) {
  signal(sig, SIG_IGN);
  exit_app();
}

void sigterm_handler(int signum, siginfo_t *info, void *ptr) {
  exit_app();
}

void catch_sigterms() {
    signal(SIGINT, sigint_handler);
    static struct sigaction sigact;
    memset(&sigact, 0, sizeof(sigact));
    sigact.sa_sigaction = sigterm_handler;
    sigact.sa_flags = SA_SIGINFO;
    sigaction(SIGTERM, &sigact, NULL);
}

char* status_value(int status) {
  switch (status) {
    case Normal:
      return "normal";
    case Insert:
      return "insert";
    case Visual:
      return "visual";
    case Operator:
      return "operator";
  }
  return "none";
}

int exec_command(char* args[]) {
  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
  }
  else if (pid < 0) {
    printf("\nfork() error '%d'\n", pid);
  }
  return pid;
}

int run_command(char* args[]) {
  pid_t pid = exec_command(args);
  int status = 0;
  if (pid > 0) {
    waitpid(pid, &status, 0);
  }
  return WEXITSTATUS(status);
}

int check_status() {
  char *status[] = { "emacs-mode", NULL };
  return run_command(status);
}

hid_device* connect(ushort vendor_id, ushort product_id, int interface) {
  struct hid_device_info *devs = hid_enumerate(0x0, 0x0);
  struct hid_device_info *cur_dev = devs;
  char* path = NULL;
  hid_device* handle = NULL;
  while (cur_dev) {
    if (cur_dev->vendor_id == vendor_id
     && cur_dev->product_id == product_id
     && cur_dev->interface_number == interface) {
      path = cur_dev->path;
    }
    cur_dev = cur_dev->next;
  }
  if (path != NULL) {
    handle = hid_open_path(path);
  }
  hid_free_enumeration(devs);
  if (!handle) {
    printf(
      "Device not found with VID '0x%04x' and PID '0x%04x' on interface '%d'.  Please ensure it is plugged in.\n",
      vendor_id,
      product_id,
      interface
    );
  }
  return handle;
}

int main(int argc, char* argv[]) {
  catch_sigterms();
  wchar_t manufacturer[MAX_STR];
  wchar_t product[MAX_STR];
  byte payload[PAYLOAD_SIZE];
  hid_device *handle = connect(VENDOR_ID, PRODUCT_ID, INTERFACE);
  byte last = None;
  if (!handle) return 1;
  hid_get_manufacturer_string(handle, manufacturer, MAX_STR);
  hid_get_product_string(handle, product, MAX_STR);
  printf("%s %ls %ls", "Reading from", manufacturer, product);
  hid_set_nonblocking(handle, 1);
  while (running) {
    payload[0] = (byte)check_status();
    if (payload[0] != last) {
      last = payload[0];
      int res = hid_write(handle, payload, sizeof(payload));
      if (res < 0) {
        printf("Unable to write()\n");
        printf("Error: %ls\n", hid_error(handle));
      }
    }
    usleep(WAIT);
  }
  hid_set_nonblocking(handle, 0);
  hid_close(handle);
  hid_exit();
  return 0;
}
