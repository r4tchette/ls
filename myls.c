#include <stdio.h>
#include <dirent.h>

void myls(char *path){
	DIR *dp;
	struct dirent *ep;
	char *np;

	dp = opendir(path);
	while(1){
		ep = readdir(dp);
		if(ep == NULL) break;
		//np = ep->d_name;
		printf("%s\n", ep->d_name);
	}
	closedir(dp);
}

int main(){
	myls(".");
	return 0;
}
