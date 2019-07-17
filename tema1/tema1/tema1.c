#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
int mystat(const char* path)
{
    struct stat fileStat;
    if(stat(path,&fileStat) < 0)    
	{
		fprintf(stderr,"eroare myfind");
		return 0;
		}
    printf("\n\nInformation for %s\n",path);
    printf("---------------------------\n");
    printf("File Size: \t\t%d bytes\n",fileStat.st_size);
    printf("Number of Links: \t%d\n",fileStat.st_nlink);
    printf("File inode: \t\t%d\n",fileStat.st_ino);
    printf("File Permissions: \t");
    printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
    printf("\n\n");
    printf("The file %s a symbolic link\n", (S_ISLNK(fileStat.st_mode)) ? "is" : "is not");
 
    return 1;
}
int myfind(const char *name, const char *search)
{
	DIR *dir;
	struct dirent *entry;
	int found=0;
	if (!(dir = opendir(name)))
	{
		fprintf(stder,"eroare myfind");
		return 0;}
	if (!(entry = readdir(dir)))
		{
		fprintf(stder,"eroare myfind");
		return 0;}
	do {
		if (entry->d_type == DT_DIR) {
			char path[1024];
			int len = snprintf(path, sizeof(path) - 1, "%s/%s", name, entry->d_name);
			path[len] = 0;
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			if(strcmp(search,entry->d_name)==0);
			{
				found=mystat(path);
			}
			myfind(path, search);
		}
		else
			if(strcmp(search,entry->d_name)==0)
			{
				char* temp;
				sprintf(temp,"%s/%s",path,entry->d_name);
				found=mystat(temp);
			}
	} while (entry = readdir(dir));
	closedir(dir);
	return found;
}

int find(const char *name, const char *search)
{
	DIR *dir;
	struct dirent *entry;
	int found=0;
	if (!(dir = opendir(name)))
		return 0;
	if (!(entry = readdir(dir)))
		return 0;

	do {
		if (entry->d_type == DT_DIR) {
			char path[1024];
			int len = snprintf(path, sizeof(path) - 1, "%s/%s", name, entry->d_name);
			path[len] = 0;
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;
			if(strcmp(search,entry->d_name)==0);
			{
				found=1;
				printf("\n%s\n",path);
			}
			find(path, search);
		}
		else
			if(strcmp(search,entry->d_name)==0)
			{
				printf("\n%s\%s\n", name,entry->d_name);
				found=1;
			}
	} while (entry = readdir(dir));
	closedir(dir);
	return found;
}
int login(const char *user){
	FILE f*=fopen("username.txt","r");
	char* token=strtok(f,'\n');
	while(token!=null)
	{
		token[strlen(token)-1]='\0';
		if(strcmp(user,token)==0)
		{
			return 1;
		}
		token=strtok(NULL,'\n');
	}
	close(f);
	return 0;
	
}
int quit()
{
	printf("\n Cya later alig8r\n");
	return 5;
}
int main(int argc,char**argv)
{
	int p2c[2],c2p[2];
	pid_t   childpid;
	char* commands[]={"login","find","myfind","mystat","help","quit"};
	char* input;
	if(pipe(p2c)==-1||pipe(c2p)==-1)
	{
		fprintf(stderr,"eroare pipe");
		exit(1);
	}
	
	if((childpid = fork()) == -1)
	{
			perror("fork");
			exit(1);
	}
void help()
{
	printf("Commands you can use are: login, find, myfind, mystat, help, quit");
}
	if(childpid != 0)
	{//parent
		close(p2c[0]);
		close(c2p[1]);
		help();
		int check_end=1,check_login=0,right_cmd=0,check_cmd=0;
			do
			{
				scanf("%s",input);
				if(check_login==0)
					{
						if(strcmp(input,"login")==0)
						{
							write(p2c[1],0,sizeof(int));
							kill(childpid,SIGINT);
							pause();
							read(c2p[0],check_login,sizeof(int));
						}
						else
							printf("\n you first gotta login, baakaa~~~~~");
					}
					else
					{	
						check_cmd=0,right_cmd=-1;
						for(int i=1;i<=5;i++)
						{
							if(strcmp(input,commands[i])==0)
							{
								write(p2c[1],i,sizeof(int));
								kill(childpid,SIGINT);
								pause();
								read(c2p[0],right_cmd,sizeof(int));
								check_cmd=1;
								break:
							}
						}
						if(i!=5)
						{if(check_cmd==0)
									printf("\n that's not a command,lad\n");
							else
								if(rightcmd!=0)
									printf("\n command %s is wrongly used \n",commands[right_cmd]);
						}
						else
						{
							pause();
							read(c2p[0],check_end,sizeof(int));
						}
					}

			}while(check_end==0)
			wait(NULL);
		exit(1);
	}
	else
	{ //child
		//login, find, myfind, mystat, help, quit
		close(p2c[1]);
		close(c2p[0]);
		char* name,*search;
		int check,fail;
		do{
			pause();
			read(p2c[0],check,sizeof(int))
			switch(check)
			{
				case 0:
							fail=0;
							scanf("%s",name);
							fail=login(name);
							write(c2p[1],fail,sizeof(int));
							kill(getppid(),SIGINT);
							if(fail==0)
								printf("\nName not found\n");
				break;
				
				case 1:
							scanf("%s",name);
							scanf("%s",search);
							if(find(name,search)==1);
							{
								write(c2p[1],0,sizeof(int));
								kill(getppid(),SIGINT);
							}
							else
							{
								write(c2p[1],check,sizeof(int));
								kill(getppid(),SIGINT);
							}
				break;
				
				case 2:
						scanf("%s",name);
						scanf("%s",search);
						if(myfind(name,search)==1);
						{
								write(c2p[1],0,sizeof(int));
								kill(getppid(),SIGINT);
							}
							else
							{
								write(c2p[1],check,sizeof(int));
								kill(getppid(),SIGINT);
							}
				break;
				
				case 3:
						scanf("%s",name);
						if(mystat(name)==1);
						{
								write(c2p[1],0,sizeof(int));
								kill(getppid(),SIGINT);
							}
							else
							{
								write(c2p[1],check,sizeof(int));
								kill(getppid(),SIGINT);
							}
				break;
				
				case 4:
						help();
						write(c2p[1],0,sizeof(int));
						kill(getppid(),SIGINT);
				break;
				
				case 5:
						write(c2p[1],quit(),sizeof(int));
						kill(getppid(),SIGINT);
				break;
			}
		}while(check!=5)
	}
	exit(1);
}









