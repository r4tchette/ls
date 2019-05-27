#include <stdio.h> // standard input/output
#include <dirent.h> // search directory
#include <sys/stat.h> // for file stat
#include <pwd.h> // for owner user name
#include <grp.h> // for owner group name
#include <time.h> // for time stat
#include <malloc.h> // dynamic memory allocation
#include <string.h> // using strcmp function for alpabetical sorting 

// ANSI colors in terminal
#define GREEN	"\x1b[1;32m" // executable file
#define LTBLUE	"\x1b[1;36m" // symbloic link file
#define BLUE	"\x1b[1;34m" // directory
#define WHITE	"\x1b[0;37m" // regular file

// linked list structure for storing file info.
typedef struct fileInfoList{
	struct stat *fileStat;
	struct dirent *dirEntry;
	struct fileInfoList *next;
} fileInfoList;

void freeAll(fileInfoList *node){
	if(node != NULL){
		freeAll(node->next);
		free(node->fileStat);
		free(node);
	}
}

char* pathCombine(char* curPath, char* dirName){
	char *newPath = (char*)malloc((strlen(curPath) + strlen(dirName) + 2) * sizeof(char));
	sprintf(newPath, "%s/%s", curPath, dirName);
	return newPath;
}

// append new node into linked list with alphabetical order
int statListAppend(fileInfoList *curNode, fileInfoList *newNode){
	if(curNode == NULL){
	 	return 1;
	} else{
		fileInfoList *prevNode = NULL;
		while(curNode != NULL){
			if(strcmp(newNode->dirEntry->d_name,
				  curNode->dirEntry->d_name) < 0){
				if(prevNode != NULL){
					prevNode->next = newNode;
				}
				newNode->next = curNode;
				if(prevNode == NULL) return 0;
				else return 1;
			}
			prevNode = curNode;
			curNode = curNode->next;
		}
		prevNode->next = newNode;
	}
	return 1;
}


void printTime(time_t time){
	struct tm *date = localtime(&time);
	//printf(" %s ", asctime(date));
	printf("%d-", date->tm_year+1900);
	if(date->tm_mon+1 < 10) printf("0%d-", date->tm_mon+1);
	else printf("%d-", date->tm_mon+1);
	if(date->tm_mday < 10) printf("0%d ", date->tm_mday);
	else printf("%d ", date->tm_mday);
	if(date->tm_hour < 10) printf("0%d:", date->tm_hour);
	else printf("%d:", date->tm_hour);
	if(date->tm_min < 10) printf("0%d ", date->tm_min);
	else printf("%d ", date->tm_min);
}


void printGroup(gid_t gid){
	struct group *gr = getgrgid(gid);
	printf("%s ", gr->gr_name);
}


void printUser(uid_t uid){
	struct passwd *pw = getpwuid(uid);
	printf("%s ", pw->pw_name);
}

// "mode_t" contains type and permission of file
char printMode(mode_t mode){
	char type = 'r';
	// file type
	switch(mode & S_IFMT){
		case S_IFBLK:	printf("b"); break;
		case S_IFCHR:	printf("c"); break;
		case S_IFDIR:	printf("d"); type = 'd'; break;
		case S_IFIFO:	printf("p"); break;
		case S_IFLNK:	printf("l"); type = 'l'; break;
		case S_IFREG:	printf("-"); break;
		case S_IFSOCK:	printf("s"); break;
		default:	printf("u"); break; // unknown type
	}
	//printf( (S_ISDIR(mode)) ? "d" : "-" );

	// file permission
	printf( (mode & S_IRUSR) ? "r" : "-" );
	printf( (mode & S_IWUSR) ? "w" : "-" );
//	printf( (mode & S_IXUSR) ? "x" : "-" );
	if(mode & S_IXUSR){
		printf("x");
		if(type == 'r') type = 'e';
	} else printf("-");
	printf( (mode & S_IRGRP) ? "r" : "-" );
	printf( (mode & S_IWGRP) ? "w" : "-" );
	printf( (mode & S_IXGRP) ? "x" : "-" );
	printf( (mode & S_IROTH) ? "r" : "-" );
	printf( (mode & S_IWOTH) ? "w" : "-" );
	printf( (mode & S_IXOTH) ? "x" : "-" );
	printf(" ");

	return type;
}


void printFileInfo(fileInfoList *node){
//	node->dirEntry
//	node->fileStat
	char type = printMode(node->fileStat->st_mode);	// file mode. type and permission
	printf("%d ", node->fileStat->st_nlink);// number of links
	printUser(node->fileStat->st_uid);	// owner user name
	printGroup(node->fileStat->st_gid);	// owner group name
	printf("%10d\t", node->fileStat->st_size);	// file size
	printTime(node->fileStat->st_mtime);	// time of last modification

//	if(node->dirEntry->d_type == DT_DIR)
	if(type == 'd') printf(BLUE);  // directory
	else if(type == 'e') printf(GREEN); // executable ile
	else if(type == 'l') printf(LTBLUE); // symbolic link file
	//else printf(WHITE);
	printf("%s", node->dirEntry->d_name);	// file and directory name
	printf(WHITE);
}

void myls(char *curPath){
	DIR *dp;
	struct dirent *ep;
	int total = 0;
	fileInfoList *fileInfo = NULL;

	printf("%s:\n", curPath);
	dp = opendir(curPath);
	if(dp){
		while((ep = readdir(dp)) != NULL){
			if(ep->d_name[0] == '.'){
				continue;
			}
			struct stat *info = (struct stat *)malloc(sizeof(struct stat));
			char *filePath = pathCombine(curPath, ep->d_name);
			if(lstat(filePath, info) == 0){
				total += info->st_blocks;

				fileInfoList *newNode = (fileInfoList *) malloc(sizeof(fileInfoList));
				newNode->fileStat = info;
				newNode->dirEntry = ep;
				newNode->next = NULL;

				if(fileInfo == NULL){
					fileInfo = newNode;
				} else{
					if(statListAppend(fileInfo, newNode) == 0){
						fileInfo = newNode;
					}
				}
			}
			free(filePath);
		}
		printf("합계: %d\n", total/2);

		if(fileInfo != NULL){
			fileInfoList *loop = fileInfo;
			while(loop != NULL){
				printFileInfo(loop);
				if(loop->dirEntry->d_type == DT_LNK){
					char* nextPath = pathCombine(curPath, loop->dirEntry->d_name);
					char buf[256];
					readlink(nextPath, buf, sizeof(buf)-1);
					printf(" -> %s", buf);
				}
				printf("\n");
				loop = loop->next;
			}
			loop = fileInfo;
			while(loop != NULL){
				if(loop->dirEntry->d_type == DT_DIR){
					printf("\n");
					char *nextPath = pathCombine(curPath, loop->dirEntry->d_name);
					myls(nextPath);
					free(nextPath);
				}
				loop = loop->next;
			}
		}
		freeAll(fileInfo);
		closedir(dp);
	}
}


int main(){
	myls(".");
	return 0;
}
