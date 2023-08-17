#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
//윈도우일 경우에만 돌아가는
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define PATH_SEPARATOR "\\"
#define DRIVE_PATH "C:\\"
//리눅스일 경우에만돌아가는
#else
#include <dirent.h>
#include <unistd.h>
#define PATH_SEPARATOR "/"
#define DRIVE_PATH "/"
#endif

void removeExtension(const char *path) {
    char newpath[1024];
    strcpy(newpath, path);
    char *ext = strrchr(newpath, '.');

    if (ext != NULL) {
        *ext = '\0';
        if (rename(path, newpath) == 0) {
            printf("확장자가 제거된 파일: %s\n", newpath);
        } else {
            printf("파일 이름 변경에 실패하였습니다.\n");
        }
    }
}

void exploreDirectory(const char *path, FILE *fileList) {
#ifdef _WIN32
    struct _finddata_t fileInfo;
    intptr_t handle;

    char searchPath[1024];
    snprintf(searchPath, sizeof(searchPath), "%s%s*", path, PATH_SEPARATOR);

    if ((handle = _findfirst(searchPath, &fileInfo)) == -1) {
        printf("디렉토리를 열 수 없습니다.\n");
        return;
    }

    do {
        if (strcmp(fileInfo.name, ".") != 0 && strcmp(fileInfo.name, "..") != 0) {
            char newPath[1024];
            snprintf(newPath, sizeof(newPath), "%s%s%s", path, PATH_SEPARATOR, fileInfo.name);

            if (fileInfo.attrib & _A_SUBDIR) {
                exploreDirectory(newPath, fileList); // 디렉토리인 경우 재귀적으로 탐색
            } else {
                fprintf(fileList, "%s\n", newPath); // 파일 경로를 리스트에 저장
                removeExtension(newPath); // 파일의 확장자 제거
            }
        }
    } while (_findnext(handle, &fileInfo) == 0);

    _findclose(handle);
#else
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    if ((dir = opendir(path)) == NULL) {
        printf("디렉토리를 열 수 없습니다.\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char newPath[1024];
        snprintf(newPath, sizeof(newPath), "%s%s%s", path, PATH_SEPARATOR, entry->d_name);

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (stat(newPath, &fileStat) == -1) {
            printf("파일 정보를 가져올 수 없습니다.\n");
            continue;
        }

        if (S_ISDIR(fileStat.st_mode)) {
            exploreDirectory(newPath, fileList); // 디렉토리인 경우 재귀적으로 탐색
        } else {
            fprintf(fileList, "%s\n", newPath); // 파일 경로를 리스트에 저장
            removeExtension(newPath); // 파일의 확장자 제거
        }
    }

    closedir(dir);
#endif
}

int main() {
    const char *directoryPath = DRIVE_PATH;
    FILE *fileList;

    // 파일 리스트를 저장할 파일을 생성
    fileList = fopen("file_list.txt", "w");
    if (fileList == NULL) {
        printf("파일 리스트를 저장할 수 없습니다.\n");
        return 1;
    }

    // 지정한 드라이브의 파일을 탐색하고 파일 리스트를 작성
    exploreDirectory(directoryPath, fileList);

    // 파일 리스트 파일을 닫음
    fclose(fileList);

    printf("파일 리스트를 저장했습니다.\n");

    return 0;
}
