#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

char* removeDoubleSlash(char *string) {
	const size_t len = strlen(string);
	
	if(string[len-1] == '\\' || string[len-1] == '/') {
    	// Terminate the string earlier
    	string[len-1] = 0;
	}
	
	return string;
}

int main(int argc, char *argv[]) {
	if(argc <= 1 || argc % 2 == 0) {
    	printf("Usage : fastlistbuilder --scan <folder_to_scan> --csv <csv_destination> --exclude <file_name_to_exclude> --exclude_hidden_files <true>\n");
    	return 1;
	}

    char *folderToScan = "/tmp";
    char *csvDestination = "/tmp/out.csv";
    char *exclude = NULL;
    int excludeHiddenFiles = 1;
    
    int i = 1;
    while(i < argc) {
    	const char *argName = argv[i];
    	char *argValue = NULL;
    	
    	int len = strlen(argv[i+1]);
        argValue = malloc((len+1) * sizeof(char));
        memcpy(argValue, argv[i+1], (len+1));
    	
    	//printf("%s => %s\n", argName, argValue);
    	
    	if(strcmp(argName, "--scan") == 0) {
    		folderToScan = argValue;
    		
    	} else if(strcmp(argName, "--csv") == 0) {
    		csvDestination = argValue;
    		
    	} else if(strcmp(argName, "--exclude") == 0) {
    		exclude = argValue;
    	}
    	
    	i += 2;
    }
    
    printf("Scan : %s\n", folderToScan);
    printf("CSV : %s\n", csvDestination);
    printf("Exclude : %s\n", exclude);
    
    //return 0;
    
    FILE *csv = fopen(csvDestination, "w");
    
    if(csv == NULL) {
    	printf("Error opening csv destination file!\n");
    	return 1;
	}

    long count = extractDir(folderToScan, csv, exclude, excludeHiddenFiles);
    printf("%s contains %ld files\n", folderToScan, count);
    printf("Files list was written to : %s\n", csvDestination);
    
    fclose(csv);

    return 0;
}

char* formatdate(char* str, time_t val) {
    strftime(str, 36, "%Y-%m-%d %H:%M:%S", localtime(&val));
        
    return str;
}

int extractDir(const char *name, FILE* csv, char *exclude, int excludeHiddenFiles) {
    DIR *dir;
    struct dirent *ent;
    long count = 0;

	char date[36];
	struct stat *file_info = malloc(sizeof(struct stat));

    dir = opendir(name);

    while((ent = readdir(dir))) {
    	char path[4096];
		int len = snprintf(path, sizeof(path)-1, "%s/%s", name, ent->d_name);
		path[len] = 0;
    
		if(ent->d_type == DT_DIR) {
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
				continue;
			}
			
			if((exclude != NULL && strstr(path, exclude) != NULL) || (excludeHiddenFiles == 1 && strstr(path, "/.") != NULL) || (excludeHiddenFiles == 1 && strstr(path, "@") != NULL)) {
				printf("Excluding directory : %s\n", path);
			
				continue;
			}

			//printf("%s\n", path);

			count += extractDir(path, csv, exclude, excludeHiddenFiles);
			
		} else {
			if((exclude != NULL && strstr(path, exclude) != NULL) || (excludeHiddenFiles == 1 && strstr(path, "/.") != NULL) || (excludeHiddenFiles == 1 && strstr(path, "@") != NULL)) {
				printf("Excluding file : %s\n", path);
			
				continue;
			}
		
			++count;

    		if (lstat(path, file_info) == 0) {
    			if(exclude != NULL && strstr(path, exclude) == NULL) {
    				//printf("%s\n", path);
    				fprintf(csv, "%s;%s\n", path, formatdate(date, file_info->st_mtime));
    			} else {
    				if(count % 1000 == 0) {
    					printf(".\n");
    				}
    			}
    		}
		}
	}

    closedir(dir);

	return count;
}
