#include <stdio.h> // standard input/output
#include <dirent.h> // search directory
//#include <unistd.h>
#include <sys/stat.h> // for file stat
#include <pwd.h> // for owner user name
#include <grp.h> // for owner group name
#include <time.h> // for time stat


void printTime(time_t time){
	struct tm *date = localtime(&time);
	//printf(" %s ", asctime(date));
	printf("%d-", date->tm_year+1900);
	printf("%d-", date->tm_mon+1);
	printf("%d ", date->tm_mday);
	printf("%d:", date->tm_hour); 
	printf("%d ", date->tm_min);
}

void printGroup(gid_t gid){
	struct group *gr = getgrgid(gid);
	printf(" %s ", gr->gr_name);
}

void printUser(uid_t uid){
	struct passwd *pw = getpwuid(uid);
	printf(" %s ", pw->pw_name);
}

// "mode_t" is the type of stat.st_mode
void printMode(mode_t mode){
	printf( (S_ISDIR(mode)) ? "d" : "-" );
	printf( (mode & S_IRUSR) ? "r" : "-" );
	printf( (mode & S_IWUSR) ? "w" : "-" );
	printf( (mode & S_IXUSR) ? "x" : "-" );
	printf( (mode & S_IRGRP) ? "r" : "-" );
	printf( (mode & S_IWGRP) ? "w" : "-" );
	printf( (mode & S_IXGRP) ? "x" : "-" );
	printf( (mode & S_IROTH) ? "r" : "-" );
	printf( (mode & S_IWOTH) ? "w" : "-" );
	printf( (mode & S_IXOTH) ? "x" : "-" );
}

void myls(char *path){
	DIR *dp;
	struct dirent *ep;
	char *np;
	struct stat info;

	dp = opendir(path);
	while(1){
		ep = readdir(dp);
		stat(ep->d_name, &info); //
		
		if(ep == NULL) break;

		//printf("[%10o] ", info.st_mode); //file permissions
		printMode(info.st_mode);
		printf(" %d ", info.st_nlink); //number of links
		//printf("[%10d] ", info.st_uid); //owner name
		printUser(info.st_uid); //owner user
		//printf("[%10d] ", info.st_gid); //owner group
		printGroup(info.st_gid); //owner group
		printf(" %d ", info.st_size);//file size
		//printf("[%10d] ", info.st_mtime); //time of last modification
		printTime(info.st_mtime); //time of last modification
		printf("[%10s]\n", ep->d_name);//file/directory name
	}
	closedir(dp);
}

int main(){
	myls(".");
	return 0;
}
