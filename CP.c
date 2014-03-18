/*
 * Без имени.c
 * 
 * Copyright 2014 Lex <lex22@ubuntu>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <dirent.h>
#include <errno.h>



#define SUCCESS 0
#define DEFOLT -1

void PrintError (char* error, char* path){
	char* res = malloc(strlen(error));
	strcpy(res, error);
	strcat(res, path);
	perror(res);
	free(res);
}

int ProseccingInputDate(int* argc, char** argv){
	int i = 1, count = 0, recursive = 0;	
	for (;i < *argc; ++i){
		if ( !strcmp(argv[i], "-R") ){
				++count;
				recursive = 1;
			} 
	}
	//The Knuth-Morris-Pratt Algorithm
	int ch = 1;	
	for (;ch <= count; ++ch){
		i = 1;		
		while (strcmp(argv[i], "-R"))
			++i;
		int j = i;		
		for (; j< *argc - 1; ++j)
			argv[j]=argv[j + 1];	
	}
	*argc -= count;
	return recursive;
}

int Copy(char*, char*, int);

int CopyDirDirRecursive(char* from, char* where){
	DIR* dir_from = opendir(from);
	if (dir_from == NULL) {
		PrintError("Cann't open directory", from);
		return DEFOLT;
	}
	struct stat stat_from;
	if (stat(from, &stat_from)) {	
		PrintError("Can't get status ", from);
		return DEFOLT;
	}

	char* wherenew;
	if (mkdir(where, stat_from.st_mode)){
		char* name = strrchr(from, '/')	;
		if (name == NULL)
			name = from;
		else name = name + 1;
		wherenew = malloc((strlen(where) + strlen(name) + 2) * sizeof(char));
		strcpy(wherenew, where);
		strcat(wherenew, "/");
		strcat(wherenew, name);
		if (mkdir(wherenew, stat_from.st_mode) && errno != EEXIST) {
			PrintError("Cann't open direectory ", wherenew);
			free(wherenew);
			closedir(dir_from);
			return DEFOLT;
		}			
	}
	else{
		wherenew = malloc(strlen(where));
		strcpy(wherenew, where);		
	}
  	struct dirent* Dirent;
  	while ((Dirent = readdir(dir_from)) != NULL){
		if (Dirent->d_name[0] != '.'){ 
			int size = (strlen(from) + strlen(Dirent->d_name) + 2) * sizeof(char);							
			char* from_new = malloc(size); 
			strcpy(from_new, from);
			strcat(from_new, "/");
			strcat(from_new, Dirent->d_name);
			Copy(from_new, wherenew, 1);
			free(from_new); 
		}
	}
}

int CopyFileFile(char* from, char* where){
	int fd_from = open(from, O_RDONLY|O_APPEND);
	if (fd_from < 0){
		PrintError("Cann't open ", from);
		return DEFOLT;
	}
	struct stat stat_from;
	stat(from, &stat_from);
	int fd_where = open(where, O_WRONLY|O_TRUNC|O_CREAT, stat_from.st_mode);
	if (fd_where < 0){
		PrintError("Cann't open ", where);
		return DEFOLT;
	}
	char buff[1000];
	while(1){
		int count_read = read (fd_from, buff, 1000);
		if (count_read < 0){
			PrintError("Cann't read ", from);
			return DEFOLT;
		}
		else 
			if (count_read != 0){
				int count_write = write (fd_where, buff, count_read);
				if(count_write < 0){
					PrintError("Cann't write ", where);
					return DEFOLT;
				}			
			}
			else return SUCCESS;
	}	

}

int CopyFileDir(char* from, char* where){
	char* name = strrchr(from, '/')	;
	if(name == NULL) name = from;
	else name += 1;
	char*  wherenew = malloc(strlen(where));
	strcpy(wherenew, where);
	strcat(wherenew, "/");
	strcat(wherenew, name);
	int result = CopyFileFile(from, wherenew);
	free(wherenew);
	return result;
}

int Copy(char* from, char* where, int flagR){
	struct stat stat_from, stat_where;
	if (stat(from, &stat_from)){	
		PrintError("Cann't get status ", from);
		return DEFOLT;
	}
	if (stat_from.st_mode & S_IFDIR) {
		if (flagR)
			if (!stat(where, &stat_where) && !(stat_from.st_mode & S_IFDIR)){	
				PrintError("Cann't get write into file directory", from);
				return DEFOLT;
			}
			else
				return CopyDirDirRecursive(from, where);
		else{
			printf("Cann't copy non-recursive directory \n");//???
			return DEFOLT;
		}
	}
	else{
		if (stat(where, &stat_where)) 
			return CopyFileFile(from, where);
		else if (stat_where.st_mode & S_IFDIR)
			return CopyFileDir(from, where);
			else return CopyFileFile (from, where);
	}
}

int main(int argc, char** argv){
	int flagR = ProseccingInputDate(&argc, argv);
	if (argc < 3){
			write(2, "Too fee arguments\n", 19);
			return DEFOLT;		
		}

	int i = 1;	
	for (;i < argc - 1; ++i)
		Copy(argv[i], argv[argc - 1], flagR);	
	return 0;
}

