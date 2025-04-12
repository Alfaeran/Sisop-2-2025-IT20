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