/*
   Descr.exe
   Copyright 2018 Sauli Hirvi

   An utility that helps the creation of 4DOS descript.ion files.

   Reads file names from current directory into an array.
   Compares contents of descript.ion file and file list, and find files
   that lack a description.

   Prompts user to enter descriptions for the missing files.

   Changelog:
   2018-05-02 Initial Release.
   2018-05-09 Added dynamic memory allocation for string arrays.

*/

#include <alloc.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

// Max length of description text
#define MAX_LINE 80

// File name length (8.3 + null)
#define FILENAME_LEN 13

char *descfn(char *);
char *lowercase(char *);
int count_descfile();
int read_descfile(char **);
int count_files();
int read_dir(char **);
int has_desc(char *, char **);
int find_missing(char **, char **, char **);
int get_descriptions(char **);
void copyright_header();

//
// Version number string
//
const char VERSION[] = "1.0.1 (2018-05-09)";

int main() {
    char **files;
    char **descs;
    char **missing;

    int i, ret;
    int file_count, line_count, missing_count;

    copyright_header();

    // Count number of files in directory.
    // Used for memory allocation.
    file_count = count_files() + 1;
    line_count = count_descfile() + 1;
    missing_count = file_count - line_count + 1;

    // Allocate memory for directory and description array pointers.
    files = (char **)calloc(file_count, sizeof(char *));
    descs = (char **)calloc(line_count, sizeof(char *));
    missing = (char **)calloc(missing_count, sizeof(char *));

    if (files == NULL || descs == NULL || missing == NULL) {
        printf("Error allocating memory.\n");
        exit(-1);
    }

    // Allocate memory for array contents.
    for (i = 0; i < file_count; i++) {
        files[i] = (char *)calloc(FILENAME_LEN, sizeof(char));
        if (files[i] == NULL) {
            printf("Error allocating memory.\n");
            exit(-1);
        }
    }

    for (i = 0; i < line_count; i++) {
        descs[i] = (char *)calloc(MAX_LINE, sizeof(char));
        if (descs[i] == NULL) {
            printf("Error allocating memory.\n");
            exit(-1);
        }
    }

    // Read contents of descrpt.ion into **descs
    read_descfile(descs);
    // Read current directory listing into **files
    read_dir(files);
    // Compare descs and files. Store undescribed files into **missing
    find_missing(files, descs, missing);
    // Get descriptions for missing files from the user
    get_descriptions(missing);

    return 0;
}

//
// Extract filename from descript.ion line
//
char *descfn(char *descline) {
    int i;

    // Find first whitespace
    for (i = 0; i < strlen(descline); i++) {
        if (descline[i] == ' ' || descline[i] == '\t') {
            // End string at whitespace and return
            descline[i] = '\0';
            return descline;
        }
    }
    return descline;
}

//
// Convert a string to lowercase.
// Returns the string pointer.
//
char *lowercase(char *string) {
    int i = 0;
    while (string[i]) {
        string[i] = tolower(string[i]);
        i++;
    }
    return string;
}

//
// Count lines in descript.ion file
//
int count_descfile() {
    FILE *fp;
    int i;
    char *temp;

    fp = fopen("descript.ion", "r");

    if (fp == NULL) {
        return 1;
    }

    // Allocate memory for temporary line
    temp = (char *)calloc(MAX_LINE, sizeof(char));

    i = 0;

    while (fgets(temp, MAX_LINE, fp)) {
        i++;
    }

    fclose(fp);
    return i;
}

//
// Read descript.ion file to destination.
//
int read_descfile(char **destination) {
    FILE *fp;
    int i;
    char *temp;

    fp = fopen("descript.ion", "r");

    if (fp == NULL) {
        return 1;
    }

    // Allocate memory for temporary line
    temp = (char *)calloc(MAX_LINE, sizeof(char));

    i = 0;

    while (fgets(temp, MAX_LINE, fp)) {
        if (strlen(temp) > 1) {
            strcpy(destination[i++], temp);
        }
    }

    fclose(fp);
    free(temp);
    return 0;
}

//
// Count number of files
//
int count_files() {
    DIR *dp;
    int i;

    dp = opendir(".");

    i = 0;

    // Directory open failed.
    if (dp == NULL) {
        return 1;
    }

    while ((readdir(dp)) != NULL) {
        i++;
    }
    closedir(dp);

    return i;
}

//
// Read directory into destination
//
int read_dir(char **destination) {
    struct dirent *dent;
    DIR *dp;
    int i;

    dp = opendir(".");

    i = 0;

    // Directory open failed.
    if (dp == NULL) {
        return 1;
    }

    while ((dent = readdir(dp)) != NULL) {
        if (dent->d_name[0] != '.') {
            strcpy(destination[i++], dent->d_name);
        }
    }
    closedir(dp);

    return 0;
}

//
// Check if a filename has a description.
// Returns 0 if no description found.
// Returns 1 if description found.
//
int has_desc(char *filename, char **descs) {
    int i;
    int cmp;
    int descfound = 0;
    char *temp;

    for (i = 0;; i++) {
        if (!descs[i][0]) {
            break;
        }
        temp = lowercase(descfn(descs[i]));
        cmp = strcmp(lowercase(filename), temp);
        if (cmp == 0)
            descfound = 1;
    }
    return descfound;
}

//
// Find which files are missing descriptions and
// store them in missing.
//
int find_missing(char **files, char **descs, char **missing) {
    int i, j;
    j = 0;

    for (i = 0;; i++) {
        if (!strlen(files[i])) {
            break;
        }

        // Skip descript.ion file
        if (!strcmp("DESCRIPT.ION", files[i]))
            continue;

        if (!has_desc(files[i], descs)) {
            missing[j++] = files[i];
        }
    }

    return 0;
}

//
// Get missing descriptions from the user.
//
int get_descriptions(char **missing) {
    FILE *fp;
    int i;
    char *buffer;
    buffer = (char *)calloc(MAX_LINE, sizeof(char));

    fp = fopen("descript.ion", "a");

    for (i = 0;; i++) {
        if (!strlen(missing[i])) {
            break;
        }
        printf("Enter description for %s (ENTER to skip):\n> ", missing[i]);
        fgets(buffer, MAX_LINE, stdin);

        if (strlen(buffer) > 1) {
            fprintf(fp, "%s %s", missing[i], buffer);
        }
    }

    fclose(fp);
    free(buffer);
    return 0;
}

//
// Prints out copyright info.
//
void copyright_header() {
    printf("Descr - 4DOS descript.ion generator.\n");
    printf("Version: %s\n", VERSION);
    printf("Copyright 2018 Sauli Hirvi\n\n");
}
