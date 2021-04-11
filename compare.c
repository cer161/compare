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


struct queue{
    char* items[SIZE];
    int rear;
    int front;
};
typedef struct queue queue;


struct linkedList{
    char* data;
    struct linkedList *next;
};
typedef struct linkedList linkedList;


struct args{
	queue arg1;
	queue arg2;
};
typedef struct args args;


//Insert an item into the queue -- returns 0 on success, 1 on failure
int insert(char* file, queue *q){
    char* item = file;
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


//Method to check if the char* points to a directory or a file. Return 1 if file is a directory, return 2 if file is a file.
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


char* readSource(int f)
{
                // make linked list to read from file iteratively
                linkedList* curr = malloc(sizeof(linkedList));
                linkedList* head;
                curr->data = malloc(sizeof(char*) * BUF_SIZE);
                curr->next = NULL;

                chars = 0;

                char* tempInput;
                int bytes;
                int buf = BUF_SIZE;
                int loop = 0;
                int length = 0;

                        //Read the contents of the file into a linked list and append that data into a char*
                        while(bytes != 0){
                                if(loop == 0){
                                        bytes = read(f, curr->data, BUF_SIZE);
                                        length = bytes;
                                        curr->data[bytes] = '\0';
                                        head = curr;
                                        tempInput = curr->data;
                                        loop = 1;
                                }
                                else{
                                        loop=1;
                                        curr->next = malloc(sizeof(linkedList));
                                        curr = curr->next;
                                        curr->data = malloc(sizeof(char*) * BUF_SIZE);
                                        curr->next = NULL;
                                        bytes = read(f, curr->data, BUF_SIZE);
                                        curr->data[bytes] = '\0';
                                        loop--;
                                        while(curr->data[loop] != '\0'){
                                                tempInput[length] = curr->data[loop];
                                                loop++;
                                                length++;
                                        }
                                }
                        }

                return tempInput;

}

void *analyzeWordFrequencies(void* arguments){
	 //Wait for the traverseDir method to finish finding all of the files nested in the directory
	 sleep(5);
	 args *queues = (args*) arguments;
   	 queue* files = &queues->arg1;
	 while(files->front != -1 && files->front <= files->rear){
		printf("Files to analyze: %s\n", pop(files));
	}
	
}


//Traverse the directories 
void *traverseDir(void* arguments){
	args *queues = (args*) arguments;
	queue* files = &queues->arg1;
	queue* dirs = &queues->arg2;
	char* current_dir;
    
	//entering critical section
	pthread_mutex_lock(&locked);
	//while the queue is not empty pop each directory and traverse
	int first = 0;
	
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
						printf("%s\n", path);
						insert(path, files);
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

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);

    return 0;

}
