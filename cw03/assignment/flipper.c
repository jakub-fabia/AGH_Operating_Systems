#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

void reverse_line(char *str){
    int len = strlen(str);
    int i, j;
    char temp;

    for (i = 0, j = len - 1; i < j; i++, j--) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

void process_file(const char *file_in, const char *file_out){
    printf("Input file: %s\n", file_in);

    FILE *in_file = fopen(file_in, "r");
    if (in_file == NULL) {
        perror("Error opening input file");
        return;
    }

    FILE *out_file = fopen(file_out, "w");
    if (out_file == NULL) {
        perror("Error opening output file");
        fclose(in_file);
        return;
    }

    char *str = NULL;
    size_t size = 0;
    ssize_t len = 0;
    int lines_cnt = 0;
    // If run on Windows it wont work beacause getline() is only available in POSTIX.
    while ((len = getline(&str, &size, in_file)) != -1) {
        reverse_line(str);
        fprintf(out_file, "%s", str);
        lines_cnt++;
    }
    printf("Written %d lines in: %s\n", lines_cnt, file_out);

    free(str);
    fclose(in_file);
    fclose(out_file);
}

void process_directory(const char *dir_in, const char *dir_out){
    struct dirent *entry;
    DIR *dir = opendir(dir_in);
    if (!dir) {
        perror("Error opening source directory");
        return;
    }

    mkdir(dir_out, 0755);
    char file_in[1024], file_out[1024];

    while ((entry = readdir(dir)) != NULL) {

        char *ext = strrchr(entry->d_name, '.');
        if (ext == NULL || strcmp(ext, ".txt") != 0) {
            continue;
        }
        
        snprintf(file_in, sizeof(file_in), "%s/%s", dir_in, entry->d_name);
        snprintf(file_out, sizeof(file_out), "%s/%s", dir_out, entry->d_name);

        process_file(file_in, file_out);
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("./flipper <input_directory> <output_directory>\n");
        return 1;
    }

    process_directory(argv[1], argv[2]);
    printf("Finished processing files.\n");
    return 0;
}