#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h> /* PATH_MAX */

#define MAX_LENGTH 1000
#define BUF_SIZE 20
#define SIZE 1000

pthread_mutex_t locked;
static int chars;
static int error_checker;
static int totalWords;
char* file_suffix;

//Queue to retrieve/store files to compare
struct queue{
    char* items[SIZE];
    int rear;
    int front;
};
typedef struct queue queue;


//Linked list to store file contents
struct linkedList{
    char* data;
    struct linkedList *next;
};
typedef struct linkedList linkedList;


//Word Frequency Distribution Structure (WFD)  
struct wordsList{
	char* word;
	int occurences;
	double frequency;
	struct wordsList *next;
};
typedef struct wordsList wordsList;


//Structure to store all file WFD's
struct filesList{
	wordsList* wordData;
	char* fileName;
	struct filesList *next;
};
typedef struct filesList filesList;


//Arguments to be read by methods called by pthread_create()
struct args{
	queue* arg1;
	queue* arg2;
};
typedef struct args args;


//Insert an item into the queue -- returns 0 on success, 1 on failure
int insert(char* file, queue *q){
	char* item = (char*) malloc(sizeof(char) * 100);
	strcpy(item, file);
	if(q->rear == SIZE-1){
       		printf("No room in the queue\n");
        	return 1;
    	}
	if(q->front == -1){
        	q->front = 0;        
        }
	q->rear++;
	q->items[q->rear] = item;
	return 0;
}

//Pop an item from the queue -- returns the item popped on success, NULL on failure
char* pop(queue *q){
    char* item;
    if(q->front == -1 || q->front > q->rear){
        printf("queue is empty\n");
        return NULL;
    }
    item = q->items[q->front];
    q->front++;
    return item;
}


//Method to check if the char* points to a directory or a file -- returns 1 if file is a directory, 2 if file is a file.
int isDir(char *fileName){
        struct stat data;
        int err = stat(fileName, &data);
        if(err){
                perror(fileName);
                return 0;
        }
        if(S_ISDIR(data.st_mode)){
                //S_ISDIR macro is true if file is a directory
                return 1;
        }
        if(S_ISREG(data.st_mode)){
                //S_ISREG macro is true if the file is a regular file
                return 2;
        }

        return 0;
}


char* standardizeString(char* string){
	char* input = string;
	char* output = malloc(sizeof(char) * 100);
	int j = 0;
	int length = strlen(input);
	for(int i = 0; i<length; i++){
		if(ispunct(input[i]) && input[i] != '-'){
			continue;
		}
		else{
			output[i] = tolower(input[i]);
			j++;
		}
	}
	return output;
}

//Method to read each word of the file and store them in the wordLists
wordsList* readSource(FILE *file){
	wordsList* list = malloc(sizeof(wordsList));
	list->word = malloc(sizeof(char)*100);
	wordsList* head;
	list->occurences = 1;
	list->frequency = 0;
	list->next = NULL;
	char x[1024];
	int i = 0;
	int new;
	char* holder;
	totalWords = 0;
	while(fscanf(file, " %1023s", x) == 1){
		new = 0;
		//Take out punctuations and uppercase
		holder = standardizeString(x);
		if(i == 0){
			strcpy(list->word, holder);
			//printf("%s\n", list->word);
			head = list;
		}
		else{
			wordsList* temp = head;
			while(temp != NULL){
				if(strcmp(temp->word, holder) == 0){
					temp->occurences++;	
					new = 1;
					break;
				}
				else{
					temp = temp->next;
				}	
			}
			if(new == 0){
				list->next = malloc(sizeof(wordsList));
				list = list->next;
				list->word = malloc(sizeof(char)*100);
				list->occurences = 1;
				list->frequency = 0;
				list->next = NULL;
				strcpy(list->word, holder);
				//printf("%s\n", list->word);
			}
		}
		totalWords++;
		i++;
	}	
	//Get the frequency for each word
	wordsList* t = head;
	while(t != NULL){
		t->frequency = t->occurences/(double)totalWords;
		t = t->next;
	}

	printf("Total words: %d\n", totalWords);
	return head;

}

//Method to compute the JSD between a pair of files -- returns the Jensen-Shannon-Distance
double computeJSD(char* curr_file, char* file_to_compare){

	return 0.25;
}

//Get the Word Frequency Distribution for each file
void *getWFD(void* arguments){
	 //Wait for the traverseDir method to finish finding all of the files nested in the directory
	 sleep(6);
	 FILE* file;
	 char* input;
	 args *queues = (args*) arguments;
   	 queue* files = queues->arg1;
	 char* curr_file;
	 char* file_to_compare;
	 queue copy;

	 filesList* curr2 = malloc(sizeof(filesList));
         filesList* head;
         curr2->wordData = malloc(sizeof(wordsList));
	 curr2->fileName = malloc(sizeof(char)*100);
         curr2->next = NULL;

	 int i = 0;
	// char* temp = malloc(sizeof(char)*100);
	 char* search;
	 //Store the filenames with their corresponding wordsList
	 queue copy2 = *files;

	char cwd[1000];
	if(getcwd(cwd, sizeof(cwd)) != NULL){
		printf("Current working directory: %s\n", cwd);
	}

	 while(copy2.front != -1 && copy2.front <= copy2.rear){
		 curr_file = pop(&copy2);
		 search = strchr(curr_file, '/');
		 if(search == NULL){
			 if((file = fopen(curr_file, "r")) == NULL){
				perror("file could not be opened");
				printf("error: file could not be opened\n");
			 }
			//printf("%s\n", curr_file);
		}
		else{
		//file came from inside a directory
			//strcpy(temp,"/");
			//printf("%s\n", curr_file);
			//strcat(temp, curr_file);
			printf("Value of curr_file: %s\n", curr_file);
			if((file = fopen(curr_file, "r")) == NULL){
				perror("file could not be opened");
				printf("error: file could not be opened\n");
			 }
		}
		 wordsList* curr = malloc(sizeof(wordsList));
	 	 curr->occurences = 0;
		 curr->frequency = 0;
         	 curr->next = NULL;
		 curr = readSource(file);

		 if(i == 0){
		 	curr2->wordData = curr;
	 	 	curr2->fileName = curr_file;
			head = curr2;
		}
		else{
			curr2->next = malloc(sizeof(filesList));
			curr2 = curr2->next;
			curr2->wordData = curr;
			curr2->fileName = malloc(sizeof(char)*100);
	 	 	curr2->fileName = curr_file;
			curr2->next = NULL;
		}
		
		i++;
		
	 }
	
	 while(head!=NULL){
		printf("File:\n\n");
		while(head->wordData != NULL){
			printf("%s %0.2f\n", head->wordData->word, head->wordData->frequency);
			head->wordData = head->wordData->next;
		}
		head = head->next;
	 }


	 double jsd;
	 while(files->front != -1 && files->front <= files->rear){
		curr_file = pop(files);
		copy = *files;
		while(copy.front != -1 && copy.front <= copy.rear){
			file_to_compare = pop(&copy);
			jsd = computeJSD(curr_file, file_to_compare);
			printf("%0.2f %s %s\n", jsd, curr_file, file_to_compare);
		}
	}	
}




//Traverse the directories 
void *traverseDir(void* arguments){
	args *queues = (args*) arguments;
	queue* files = queues->arg1;
	queue* dirs = queues->arg2;
    
	//entering critical section
	pthread_mutex_lock(&locked);
	//while the queue is not empty pop each directory and traverse
	int first = 0;
	char* current_dir;
	while(dirs->front != -1 && dirs->front <= dirs->rear){
			char head[100];
       			current_dir = pop(dirs);
			//Save the directory entered by the user to return the path
			if(first == 0){
				strcpy(head,current_dir);
				first = 1;
			}
			//traverse directory -- add any directories to the directories queue and any files to the files queue
                        char* preFix = ".";
			char* ext;
			struct dirent *file;
                        DIR *cd = opendir(current_dir);
			if(cd == NULL){
				perror("the directory does not exist");
				exit(1);
			}
			file = readdir(cd);
			char buf[PATH_MAX];
			char path2[PATH_MAX];
			char parent_dir[PATH_MAX];
			//char* res = malloc(sizeof(char)*1000);
                        while(file != NULL)
                        {
				//If the entry is a file and does not begin with "." and contains the specified file suffix, add it to the files queue
                                if((file->d_type == DT_REG) && !(strncmp(preFix, file->d_name, strlen(preFix))==0))
                                {
					ext = strchr(file->d_name, '.');
					if(strcmp(ext, file_suffix) == 0){
					//NEED TO EDIT THIS LOGIC FOR NESTED DIRECTORIES
						strcpy(parent_dir, head);
						strcpy(path2, "/");
						strcat(path2, file->d_name);
						strcat(parent_dir, path2);
						insert(parent_dir, files);
					}
                                }
				//If the file is a directory, add it to the directories queue
				else if(file->d_type == DT_DIR && (strcmp(file->d_name, ".") != 0) && (strcmp(file->d_name, "..") != 0)){
					char path[100] = {0};
					strcat(path, current_dir);
					strcat(path, "/");
					strcat(path, file->d_name);
					insert(path, dirs);
				}
				file = readdir(cd);
                        }
	
       		//close directory when finished
        	closedir(cd);
       		//printf("%s\n", current_dir);
    }
     pthread_mutex_unlock(&locked);
     //left critical section
}


int main(int argc, char** argv){
    if(argc <= 1){
        return EXIT_FAILURE;
    }
    error_checker = 0;
    int err;
    pthread_t file_thread, dir_thread;
    //Initialize mutex
    err = pthread_mutex_init(&locked, NULL);
    //Check if there was a problem creating the mutex and print to perror
    if(err != 0){
        errno = err;
        perror("pthread_mutex_init");
        exit(1);
    }

    //initialize queues
    queue* queue_files = malloc(sizeof(queue));
    queue_files->front = -1;
    queue_files->rear = -1;
    queue* queue_dirs =  malloc(sizeof(queue));
    queue_dirs->front = -1;
    queue_dirs->rear = -1;

    //initialize arguments struct
    args arguments;
    arguments.arg1 = queue_files;
    arguments.arg2 = queue_dirs;

    //Get command line arguments
    file_suffix = malloc(sizeof(char)*50);
    int directory_threads = 1;
    int file_threads = 1;
    int analysis_threads = 1;
    //require all optional arguments to be given at the beginning for simplicity
    int reg_argument = 0;

    char* input;
    char* optionValue = malloc(sizeof(char)*50);
    char* ext = malloc(sizeof(char)*50);
    for(int i=1; i<argc ; i++){
        input = argv[i];
        //optional argument was entered
        if(input[0] == '-'){
		if(reg_argument != 0){
			perror("optional arguments must be entered before regular arguments");
			continue;
		}
		char option = input[1];
		int j = 0;
		for(int i=2; i<strlen(input); i++){
			optionValue[j] = input[i];
			j++;
		}
        	switch(option){
			//directory threads
			case 'd':
				if(input[2]<=0){
					error_checker = 1;
					perror("invalid option");
				}
				if(directory_threads == 1){
					directory_threads = atoi(optionValue);
				}
				else{
					error_checker = 1;
					perror("option already specified");
				}
				break;
			//file threads
			case 'f':
				if(input[2]<=0){
					error_checker = 1;
					perror("invalid option");
				}
				if(file_threads == 1){
					file_threads = atoi(optionValue);
				}
				else{
					error_checker = 1;
					perror("option already specified");
				}
				break;
			//analysis threads
			case 'a':
				if(input[2]<=0){
					error_checker = 1;
					perror("invalid option");
				}
				if(analysis_threads == 1){
					analysis_threads = atoi(optionValue);
				}
				else{
					error_checker = 1;
					perror("option already specified");
				}
				break;
			//file name suffix
			case 's':
				if(strcmp(file_suffix, "") == 0){ 
					strcpy(file_suffix, optionValue);
				}
				else{
					error_checker = 1;
					perror("option already specified");
				}
				break;
			default:
				error_checker = 1;
				perror("invalid option");
		}  
        }
        //user entered a file -- spawn a file thread -- finish when the queue is empty and all directory threads are finished
        else if(isDir(input) == 2){
		reg_argument = 1;
		if(strcmp(file_suffix, "") == 0) file_suffix = ".txt";
			ext = strchr(input, '.');
			if(strcmp(ext, file_suffix) == 0){
        			insert(input, queue_files);
			}
        }
        //user entered a directory -- spawn a directory thread
        else if(isDir(input) == 1){
		reg_argument = 1;
		if(strcmp(file_suffix, "") == 0) file_suffix = ".txt";
        	insert(input, queue_dirs); 
        }
    }


	err = pthread_create(&dir_thread, NULL, &traverseDir, (void*)&arguments);
	if(err != 0){
       		errno = err;
        	perror("pthread_create");
        	exit(1);
    	}

	err = pthread_create(&file_thread, NULL, &getWFD, (void*)&arguments);
	if(err != 0){
        	errno = err;
        	perror("pthread_create");
        	exit(1);
   	 }

    if(error_checker == 1){
	return EXIT_FAILURE;
    }

    pthread_join(file_thread, NULL);
    pthread_join(dir_thread, NULL);

    return 0;

}
