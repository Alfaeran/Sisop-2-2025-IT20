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

# Soal 4
