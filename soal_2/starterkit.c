#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

#define STARTER_DIR "starter_kit"
#define QUARANTINE_DIR "quarantine"
#define LOG_FILE "activity.log"
#define PID_FILE "decryptor.pid"

void write_log(const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log == NULL) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "[%d-%m-%Y][%H:%M:%S]", t);

    fprintf(log, "%s - %s\n", timebuf, message);
    fclose(log);
}

char *decode_base64(const char *src) {
    FILE *fp = fopen("tmp.b64", "w");
    fprintf(fp, "%s", src);
    fclose(fp);

    system("base64 -d tmp.b64 > tmp.txt 2>/dev/null");

    fp = fopen("tmp.txt", "r");
    if (!fp) return NULL;

    static char buf[256];
    fgets(buf, sizeof(buf), fp);
    fclose(fp);

    remove("tmp.b64");
    remove("tmp.txt");

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';

    return buf;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) {
        char msg[128];
        sprintf(msg, "Successfully started decryption process with PID %d.", pid);
        write_log(msg);

        FILE *fp = fopen(PID_FILE, "w");
        if (fp) {
            fprintf(fp, "%d", pid);
            fclose(fp);
        }
        exit(EXIT_SUCCESS);
    }

    umask(0);
    setsid();
    chdir(".");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    while (1) {
        DIR *d = opendir(STARTER_DIR);
        if (!d) continue;

        struct dirent *f;
        while ((f = readdir(d)) != NULL) {
            if (f->d_type == DT_REG) {
                char path[256];
                sprintf(path, "%s/%s", STARTER_DIR, f->d_name);

                char *decoded = decode_base64(f->d_name);
                if (decoded && strlen(decoded) > 0) {
                    char new_path[256];
                    sprintf(new_path, "%s/%s", STARTER_DIR, decoded);
                    rename(path, new_path);
                }
            }
        }
        closedir(d);
        sleep(5);
    }
}

void move_files(const char *src, const char *dest, const char *log_action) {
    DIR *d = opendir(src);
    if (!d) return;

    struct dirent *f;
    while ((f = readdir(d)) != NULL) {
        if (f->d_type == DT_REG) {
            char oldpath[256], newpath[256];
            sprintf(oldpath, "%s/%s", src, f->d_name);
            sprintf(newpath, "%s/%s", dest, f->d_name);
            rename(oldpath, newpath);

            char msg[256];
            sprintf(msg, "%s - Successfully %s.", f->d_name, log_action);
            write_log(msg);
        }
    }
    closedir(d);
}

void eradicate_files() {
    DIR *d = opendir(QUARANTINE_DIR);
    if (!d) return;

    struct dirent *f;
    while ((f = readdir(d)) != NULL) {
        if (f->d_type == DT_REG) {
            char path[256];
            sprintf(path, "%s/%s", QUARANTINE_DIR, f->d_name);
            remove(path);

            char msg[256];
            sprintf(msg, "%s - Successfully deleted.", f->d_name);
            write_log(msg);
        }
    }
    closedir(d);
}

void shutdown_daemon() {
    FILE *fp = fopen(PID_FILE, "r");
    if (!fp) return;

    int pid;
    fscanf(fp, "%d", &pid);
    fclose(fp);

    if (kill(pid, SIGTERM) == 0) {
        char msg[128];
        sprintf(msg, "Successfully shut off decryption process with PID %d.", pid);
        write_log(msg);
        remove(PID_FILE);
    }
}

int main(int argc, char *argv[]) {
    mkdir(STARTER_DIR, 0777);
    mkdir(QUARANTINE_DIR, 0777);

    if (argc != 2) {
        printf("Penggunaan: ./starterkit --decrypt / --quarantine / --return / --eradicate / --shutdown\n");
        return 1;
    }

    if (strcmp(argv[1], "--decrypt") == 0) {
        daemonize();
    } else if (strcmp(argv[1], "--quarantine") == 0) {
        move_files(STARTER_DIR, QUARANTINE_DIR, "moved to quarantine directory");
    } else if (strcmp(argv[1], "--return") == 0) {
        move_files(QUARANTINE_DIR, STARTER_DIR, "returned to starter kit directory");
    } else if (strcmp(argv[1], "--eradicate") == 0) {
        eradicate_files();
    } else if (strcmp(argv[1], "--shutdown") == 0) {
        shutdown_daemon();
    } else {
        printf("Argumen tidak dikenali.\n");
    }

    return 0;
}