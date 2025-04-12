#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>


void show_menu() {
printf("\033[1;36m=====================================================\033[0m\n");
printf("\033[1;33m                   DEBUGMON MENU                     \033[0m\n");
printf("\033[1;36m=====================================================\033[0m\n");
printf("\033[1;31mGunakan Perintah:\033[0m ./debugmon <command> <username>\n\n");

printf("\033[1;33mCommand yang tersedia:\033[0m\n");
printf("  \033[1;34mlist\033[0m     : Menampilkan semua proses user\n");
printf("  \033[1;34mdaemon\033[0m   : Menjalankan debugmon sebagai daemon\n");
printf("  \033[1;34mstop\033[0m     : Menghentikan proses debugmon daemon\n");
printf("  \033[1;34mfail\033[0m     : Menghentikan semua proses user dan memblokir\n");
printf("  \033[1;34mrevert\033[0m   : Mengizinkan user kembali menjalankan proses\n\n");

printf("Log disimpan di file: \033[1;35mdebugmon.log\033[0m\n");
printf("\033[1;36m=====================================================\033[0m\n");
}


void write_log(const char *proc_name, const char *status) {
FILE *log = fopen("debugmon.log", "a");
if (!log) return;

time_t now = time(NULL);
struct tm *t= localtime(&now);

char timestamp[64];
strftime(timestamp, sizeof(timestamp), "[%d:%m:%Y]-[%H:%M:%S]", t);
fprintf(log, "%s_%s_%s\n", timestamp, proc_name, status);
fclose(log);
}

void list_processes(const char *username) {
struct passwd *pw = getpwnam(username);
if (pw == NULL) {
printf("User %s tidak ditemukan.\n", username);
return;
}

uid_t target_uid = pw->pw_uid;

DIR *proc = opendir("/proc");
if (!proc) {
perror("opendir");
return;
}

struct dirent *entry;

printf("PID\tCMD\t\tCPU\tMEM\n");

while ((entry = readdir(proc)) !=NULL) {
if (entry->d_type == DT_DIR) {
int is_pid = 1;
for (int i = 0; entry->d_name[i] != '\0'; i++) {
if (entry->d_name[i] < '0' || entry->d_name[i] > '9') {
is_pid = 0;
break;
}
}

if (!is_pid) continue;

char status_path[512];
snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);

FILE *status = fopen(status_path, "r");
if (!status) continue;

uid_t uid = -1;
char name [256] = "";
char line[256];

while (fgets(line, sizeof(line), status)) {
if (strncmp(line, "Uid:", 4) == 0) {
sscanf(line, "Uid:\t%d", &uid);
}
if (strncmp(line, "Name:", 5) == 0) {
sscanf(line, "Name:\t%s", name);
}
}

fclose(status);

if (uid == target_uid) {
char stat_path[512];
snprintf(stat_path, sizeof(stat_path), "/proc/%s/stat", entry->d_name);

FILE *stat = fopen(stat_path, "r");
if (!stat) continue;

int pid;
char comm[256];
char state;
long unsigned utime, stime;
fscanf(stat, "%d %s %c", &pid, comm, &state);

for (int i = 0; i < 10; i++) fscanf(stat, "%*s");
fscanf(stat, "%lu %lu", &utime, &stime);
fclose(stat);

float cpu_usage = (float)(utime + stime) / sysconf(_SC_CLK_TCK);

long mem_kb = 0;
FILE *status2 = fopen(status_path, "r");
if (status2) {
while (fgets(line, sizeof(line), status2)) {
if (strncmp(line, "VmRSS:", 6) == 0) {
sscanf(line, "VmRSS: %ld", &mem_kb);
break;
}
}
fclose(status2);
}
printf("%s\t%s\t\t%.2f\t%ld KB\n", entry->d_name, name, cpu_usage, mem_kb);
}
}
}
closedir(proc);
}


void start_daemon(const char *username) {
pid_t pid = fork();
if (pid < 0) exit(EXIT_FAILURE);
if (pid > 0) exit(EXIT_SUCCESS);

setsid();
fclose(stdin);
fclose(stdout);
fclose(stderr);

FILE *pidfile = fopen("debugmon.pid", "w");
fprintf(pidfile, "%d\n", getpid());
fclose(pidfile);

while (1) {
DIR *proc = opendir("/proc");
if (!proc) exit(EXIT_FAILURE);
struct dirent *entry;

struct passwd *pw = getpwnam(username);
if (!pw) exit(EXIT_FAILURE);
uid_t target_uid = pw->pw_uid;

while ((entry = readdir(proc)) != NULL) {
if (entry->d_type != DT_DIR) continue;

int is_pid = 1;
for (int i = 0; entry->d_name[i] != '\0'; i++) {
if (!isdigit(entry->d_name[i])) {
is_pid = 0;
break;
}
}

if (!is_pid) continue;

char status_path[512];
snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);
FILE *status = fopen(status_path, "r");
if (!status) continue;

uid_t uid = -1;
char name[256] = "";
char line[256];

while (fgets(line, sizeof(line), status)) {
if (strncmp(line, "Uid:", 4) == 0) sscanf(line, "Uid:\t%d", &uid);
if (strncmp(line, "Name:", 5) == 0) sscanf(line, "Name:\t%s", name);
}
fclose(status);

if (uid == target_uid) {
write_log(name, "RUNNING");
}
}
closedir(proc);
sleep(5);
}
}


void stop_daemon(const char *username) {
FILE *pidfile = fopen("debugmon.pid", "r");
if (!pidfile) {
printf("PID file tidak ditemukan. Apakah daemon sedang berjalan?\n");
return;
}

int pid;
fscanf(pidfile, "%d", &pid);
fclose(pidfile);

if (kill(pid, SIGTERM) == 0) {
printf("Daemon dihentikan (PID: %d).\n", pid);
write_log("debugmon", "RUNNING");
remove("debugmon.pid");
} else {
perror("Gagal menghentikan daemon");
}
}


void fail_user(const char *username) {
struct passwd *pw = getpwnam(username);
if (!pw) {
printf("User %s tidak ditemukan.\n", username);
return;
}

uid_t target_uid = pw->pw_uid;

char blockfile[256];
snprintf(blockfile, sizeof(blockfile), ".debugmon_blocked_%s", username);
FILE *f = fopen(blockfile, "w");
if (f) fclose(f);

DIR *proc = opendir("/proc");
if (!proc) {
perror("opendir");
return;
}

struct dirent *entry;

while ((entry = readdir(proc)) != NULL) {
if (entry->d_type != DT_DIR) continue;

int is_pid = 1;
for (int i = 0; entry->d_name[i]; i++) {
if (!isdigit(entry->d_name[i])) {
is_pid = 0;
break;
}
}

if (!is_pid) continue;

char status_path[512];
snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);

FILE *status = fopen(status_path, "r");
if (!status) continue;

uid_t uid = -1;
char name[256] = "";
char line[256];

while (fgets(line, sizeof(line), status)) {
if (strncmp(line, "Uid:", 4) == 0)
sscanf(line, "Uid:\t%d", &uid);
if (strncmp(line, "Name:", 5) == 0)
sscanf(line, "Name:\t%s", name);
}
fclose(status);

if (uid == target_uid) {
pid_t pid = atoi(entry->d_name);
kill(pid, SIGKILL);
write_log(name, "FAILED");
}
}

closedir(proc);
printf("Semua proses milik %s dihentikan dan user diblokir.\n", username);
}


void revert_user(const char *username) {
struct passwd *pw = getpwnam(username);
if (!pw) {
printf("User %s tidak ditemukan.\n", username);
return;
}

uid_t target_uid = pw->pw_uid;

char blockfile[256];
snprintf(blockfile, sizeof(blockfile), ".debugmon_blocked_%s", username);

if (remove(blockfile) == 0) {
printf("User %s dapat kembali menjalankan proses.\n", username);
write_log("debugmon", "RUNNING");
} else {
printf("Tidak dapat menghapus file blokir untuk user %s.\n", username);
}
}


int main(int argc, char *argv[]) {
if (argc < 3) {
show_menu();
return 1;
}

if (strcmp(argv[1], "list") == 0) {
list_processes(argv[2]);
} else if (strcmp(argv[1], "daemon") == 0) {
start_daemon(argv[2]);
} else if (strcmp(argv[1], "stop") == 0) {
stop_daemon(argv[2]);
} else if (strcmp(argv[1], "fail") == 0) {
fail_user(argv[2]);
} else if (strcmp(argv[1], "revert") == 0) {
revert_user(argv[2]);
} else {
printf("Perintah tidak dikenal.\n");
show_menu();
}
return 0;
}
