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

// free all memory allocation 
void freeAll(fileInfoList *node){
	if(node != NULL){
		freeAll(node->next);
		free(node->fileStat);
		free(node);
	}
}

// combine the path. shape is "<curPath>/<dirName>"
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
			// alphabetical sorting
			if(strcmp(newNode->dirEntry->d_name,
				  curNode->dirEntry->d_name) < 0){
				if(prevNode != NULL){
					prevNode->next = newNode;
				}
				newNode->next = curNode;
				// when prevNode is NULL, return 0
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

// print last modification time with localtime
// localtime function in <time.h>
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

// get owner group name using 'getgrgid' in <grp.h>
void printGroup(gid_t gid){
	struct group *gr = getgrgid(gid);
	printf("%s ", gr->gr_name);
}

// get owner user name using 'getpwuid' in <pwd.h>
void printUser(uid_t uid){
	struct passwd *pw = getpwuid(uid);
	printf("%s ", pw->pw_name);
}

// "mode_t" contains type and permission of file
// type distiguishes only regular, directory, symbolic link
char printMode(mode_t mode){
	char type = '-'; // meaningless file type

	// file type
	switch(mode & S_IFMT){
		case S_IFBLK:	printf("b"); break; // block special
		case S_IFCHR:	printf("c"); break; // character special
		case S_IFDIR:	printf("d"); type = 'd'; break; // directoiy
		case S_IFIFO:	printf("p"); break; // FIFO special
		case S_IFLNK:	printf("l"); type = 'l'; break; // symbolic link
		case S_IFREG:	printf("-"); break; // regular
		case S_IFSOCK:	printf("s"); break; // socket
		default:	printf("u"); break; // unknown type
	}

	// file permission
	printf( (mode & S_IRUSR) ? "r" : "-" );
	printf( (mode & S_IWUSR) ? "w" : "-" );

	// if the file is regular and has executing permission,
	// the file is executable file.
	// regular file is only can be W/R
	if(mode & S_IXUSR){
		printf("x");
		if(type == '-') type = 'e';
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

// ANSI colors in terminal
// chage color according to the file type;
void printColor(char type){
	switch(type){
		case 'd':	printf(BLUE); break; // directory
		case 'e':	printf(GREEN); break; // executable file
		case 'l':	printf(LTBLUE); break; // symbolic link file
		default:	printf(WHITE); return; // regular and the others
	}
}

// print all status of the file
// information list is same with command 'ls -l'
void printFileInfo(fileInfoList *node){
	char type = printMode(node->fileStat->st_mode);	// file mode. type and permission
	printf("%d ", node->fileStat->st_nlink);// number of links
	printUser(node->fileStat->st_uid);	// owner user name
	printGroup(node->fileStat->st_gid);	// owner group name
	printf("%10d\t", node->fileStat->st_size);	// file size
	printTime(node->fileStat->st_mtime);	// time of last modification

	printColor(type); // print different color depending on the file type
	printf("%s", node->dirEntry->d_name);	// file and directory name
	printf(WHITE);
}

// implement my ls function
// print all the files' status in the path
// function is called recursively, and can get '-R' option 
void myls(char *curPath){
	DIR *dp;
	struct dirent *ep;
	int total = 0; // total block size of all the files in current path
	fileInfoList *firstNode = NULL; // linked list for storing file info.

	printf("%s:\n", curPath); 
	dp = opendir(curPath);
	if(dp){
		while((ep = readdir(dp)) != NULL){
			if(ep->d_name[0] == '.'){ // ignoring hidden files
				continue;
			}
			struct stat *status = (struct stat *)malloc(sizeof(struct stat));
			char *filePath = pathCombine(curPath, ep->d_name);
			// using 'lstat' function to care symbolic link
			// symobolic link in 'stat' point the contents of link
			if(lstat(filePath, status) == 0){
				// summation all the file's block size
				total += status->st_blocks;

				// create new node to append into list
				fileInfoList *newNode = (fileInfoList *) malloc(sizeof(fileInfoList));
				newNode->fileStat = status;
				newNode->dirEntry = ep;
				newNode->next = NULL;

				if(firstNode == NULL){ // if list is empty
					firstNode = newNode;
				} else{
					// if newNode is ahead firstNode
					if(statListAppend(firstNode, newNode) == 0){
						firstNode = newNode;
					}
				}
			}
			free(filePath);
		}
		printf("합계: %d\n", total/2);

		if(firstNode != NULL){
			fileInfoList *loop = firstNode;
			// from first node, loop all the list
			while(loop != NULL){
				printFileInfo(loop);
				// print the contents of a symbloic link
				if(loop->dirEntry->d_type == DT_LNK){
					char* nextPath = pathCombine(curPath, loop->dirEntry->d_name);
					char fileName[256];
					readlink(nextPath, fileName, sizeof(fileName)-1);
					printf(" -> %s", fileName);
				}
				printf("\n");
				loop = loop->next;
			}

			// loop one more time for deeper directory
			loop = firstNode;
			while(loop != NULL){
				// do recursively every directory
				if(loop->dirEntry->d_type == DT_DIR){
					printf("\n");
					char *nextPath = pathCombine(curPath, loop->dirEntry->d_name);
					myls(nextPath);
					free(nextPath);
				}
				loop = loop->next;
			}
		}
		// free all memory allocation and close DIR pointer
		freeAll(firstNode);
		closedir(dp);
	}
}

// just implement command 'ls -Rl'\
// do not take argument and get file status in current working directory
int main(){
	myls(".");
	return 0;
}
