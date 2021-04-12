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

#define MAX_LENGTH 1000
#define BUF_SIZE 20
#define SIZE 1000

pthread_mutex_t locked;
static int chars;
//The jensen-shannon-distance 
static int jsd;
static int error_checker;

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


//Linked list to store words and their corresponding frequencies
struct wordsList{
	char word[1024];
	int wordFrequency;
	struct wordsList *next;
};
typedef struct wordsList wordsList;


//Structure to store all file wordsLists
struct filesList{
	wordsList* wordData;
	char* fileName;
	struct filesList *next;
};
typedef struct filesList filesList;


//Arguments to be read by methods called by pthread_create()
struct args{
	queue arg1;
	queue arg2;
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




//Method to read each word of the file and store them in the wordLists
wordsList* readSource(FILE *file){
	wordsList* list = malloc(sizeof(wordsList));
	wordsList* head;
	list->wordFrequency = 0;
	list->next = NULL;
	char x[1024];
	int i = 0;
	while(fscanf(file, " %1023s", x) == 1){
		if(i == 0){
			strcpy(list->word, x);
			//printf("%s\n", list->word);
			head = list;
		}
		else{
			list->next = malloc(sizeof(wordsList));
			list = list->next;
			list->wordFrequency = 0;
			list->next = NULL;
			strcpy(list->word, x);
			//printf("%s\n", list->word);
		}
		i++;
	}
	return head;

}

//Method to compute the JSD between a pair of files -- returns the Jensen-Shannon-Distance
int computeJSD(char* curr_file, char* file_to_compare){

	return 0;
}

//Count the word frequencies for each file
void *analyzeWordFrequencies(void* arguments){
	 //Wait for the traverseDir method to finish finding all of the files nested in the directory
	 sleep(3);
	 FILE* file;
	 char* input;
	 args *queues = (args*) arguments;
   	 queue* files = &queues->arg1;
	 char* curr_file;
	 char* file_to_compare;
	 queue copy;

	 filesList* curr2 = malloc(sizeof(filesList));
         filesList* head;
         curr2->wordData = malloc(sizeof(wordsList));
	 curr2->fileName = malloc(sizeof(char)*100);
         curr2->next = NULL;

	 int i = 0;
	 char buf[1000];
	 //Store the filenames with their corresponding wordsList
	 queue copy2 = *files;
	 while(copy2.front != -1 && copy2.front <= copy2.rear){
		 curr_file = pop(&copy2);
		 //realpath(curr_file, buf);
		 file = fopen(curr_file, "r");
		 //file = fopen(buf, "r");
		 wordsList* curr = malloc(sizeof(wordsList));
	 	 curr->wordFrequency = 0;
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
		printf("File:\n");
		while(head->wordData != NULL){
			printf("%s\n", head->wordData->word );
			head->wordData = head->wordData->next;
		}
		head = head->next;
	 }


	 int jsd;
	 while(files->front != -1 && files->front <= files->rear){
		curr_file = pop(files);
		copy = *files;
		while(copy.front != -1 && copy.front <= copy.rear){
			file_to_compare = pop(&copy);
			jsd = computeJSD(curr_file, file_to_compare);
			printf("%d %s %s\n", jsd, curr_file, file_to_compare);
		}
	}	
}


//Traverse the directories 
void *traverseDir(void* arguments){
	args *queues = (args*) arguments;
	queue* files = &queues->arg1;
	queue* dirs = &queues->arg2;
	
    
	//entering critical section
	pthread_mutex_lock(&locked);
	//while the queue is not empty pop each directory and traverse
	int first = 0;
	char* current_dir;
	while(dirs->front != -1 && dirs->front <= dirs->rear){
			char* head;
			char* path;
       			current_dir = pop(dirs);
			//Save the directory entered by the user to return the path
			if(first == 0){
				head = current_dir;
				first = 1;
			}
			//traverse directory -- add any directories to the directories queue and any files to the files queue
                        char* preFix = ".";
			char* postFix = "txt";
			char* ext;
			char buf[1000];
			struct dirent *file;
                        chdir(current_dir);
                        // open current directory after cd
                        DIR *cd = opendir(".");
			//Cd out of the current directory
			file = readdir(cd);
			file = readdir(cd);
                        // loop through files in directory
                        while((file = readdir(cd)) != NULL)
                        {
				//If the entry is a file and does not begin with ".", add it to the files queue
                                if((file->d_type == DT_REG) && !(strncmp(preFix, file->d_name, strlen(preFix))==0))
                                {
					ext = strchr(file->d_name, '.') + 1;
					if(strcmp(ext, postFix) == 0){
						realpath(file->d_name, buf);
						path = strstr(buf, head);	
						insert(path, files);
						//printf("path: %s\n", path);
						//printf("buf: %s\n", buf);	
						//insert(file->d_name, files);
					}
                                }
				//If the file is a directory, add it to the directories queue
				else if(file->d_type == DT_DIR){
					insert(file->d_name, dirs);
				}
                        }
	
        // close directory when finished
        closedir(cd);
       // printf("%s\n", current_dir);
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
    pthread_t th1, th2;
    //Initialize mutex
    err = pthread_mutex_init(&locked, NULL);
    //Check if there was a problem creating the mutex and print to perror
    if(err != 0){
        errno = err;
        perror("pthread_mutex_init");
        exit(1);
    }

    //initialize queues
    queue queue_files;
    queue_files.front = -1;
    queue_files.rear = -1;
    queue queue_dirs;
    queue_dirs.front = -1;
    queue_dirs.rear = -1;


    //Get command line arguments
    char* input;
    for(int i=1; i<argc ; i++){
        input = argv[i];
        //optional argument was entered
        if(input[0] == '-'){
        	continue;  
        }
        //user entered a file
        if(isDir(input) == 2){
        	insert(input, &queue_files);
        }
        //user entered a directory
        if(isDir(input) == 1){
        	insert(input, &queue_dirs);  
		
        }
    }

    //initialize arguments struct
    args arguments;
    arguments.arg1 = queue_files;
    arguments.arg2 = queue_dirs;

    //create a thread to read directory entries
    err = pthread_create(&th1, NULL, &traverseDir, (void*)&arguments);
    //Check if there was a problem creating thread1 and print to perror
    if(err != 0){
         errno = err;
         perror("pthread_create");
         exit(1);
    }

    //create a thread to analyze word frequencies
    err = pthread_create(&th2, NULL, &analyzeWordFrequencies, (void*)&arguments);
    //Check if there was a problem creating thread2 and print to perror
    if(err != 0){
        errno = err;
        perror("pthread_create");
        exit(1);
    }
    if(error_checker == 1){
	return EXIT_FAILURE;
    }

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    return 0;

}
