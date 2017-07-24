#define _FILE_OFFSET_BITS 64

#include <linux/fs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>

#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <fts.h>
#include <stdio.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>

//void iterate_files(char * const argv[]);
//void run();
//void write_to_file(char* data);
//void initialize_log_file();
void executeD(char* file_to_process);
void flagProcess(int number_of_pairs, char **flags);
void executeF(char* file_to_process);
void executeFall(char* file_to_process);
void executeP(char * const path_to_process[]);
void executeDall(char * const path_to_process[]);
void checkFileSlackSpace(char* current_file);
void createLogFile();
void initLogEntry();
void writeResultOnLog(char* operation_name, char* file_analysed, char* data_to_write);



int log_count;
char* device_name;
int file_descriptor, device_descriptor, block_size;

long file_size;

int slack_bytes, block_count;

off_t offset;

unsigned char *block_buffer;
int block;
struct stat target_statval;

int major, minor;

struct stat *statval;
struct stat private_statval;

int x;

long long seek;

int file_counter;

int log_operation_counter;
	

int main(int argc,char **argv){

	device_name = NULL;
	file_descriptor = 0;
	device_descriptor = 0;
	block_size = 0;
	file_size = 0;
	file_counter = 0;
	log_operation_counter=1;
	log_count = 0;

	flagProcess(argc-1, argv);
	
	return 0;
}

void flagProcess(int number_of_arguments, char **flagFilePair){ //flagFilePair is [flag, file] 
	int count = 1; //To access array position
	
	if(number_of_arguments==1){ //Can be help or wrong argument
		if (strncmp(flagFilePair[number_of_arguments], "-help", 5)==0){
			printf("The following flags are available in SPA:\n\n-p path : Output the files that have info in their slack space\n-d file : Erase the slack space of file\n-delete-all : Erase skack space in all files inside current directory\n-f file : Output the info stored by bmap in file\n-f-all file : output all slack space content of file\n\n");	
		}
		else{ //One wrong argument was given
			printf("<<SPA Error: %s is not a flag accepted by SPA. \nAt least one flag should be given as the first argument. \nUse -help to see all suported flags>>\n", flagFilePair[number_of_arguments]);
		}
	}
	else if (number_of_arguments==0){ //No arguments were given
		printf("At least one flag should be given as the first argument. \nUse -help to see all suported flags>>\n");
	}

	else{ //two arguments were given
		createLogFile();
		initLogEntry();
		while(number_of_arguments%2 == 0 && (number_of_arguments!=0)){
			if(strncmp(flagFilePair[count],"-p",2)==0){
				//printf("%s\n", flagFilePair[count+1]);
				executeP(flagFilePair+1);
			}
			else if(strncmp(flagFilePair[count], "-delete-all", 11)==0){
				//printf("%s\n", flagFilePair[count+1]);	
				executeDall(flagFilePair+1);	
			}
			else if(strncmp(flagFilePair[count],"-d",2)==0){
				//printf("%s\n", flagFilePair[count+1]);
				executeD(flagFilePair[count+1]);
			}
			else if(strncmp(flagFilePair[count], "-f-all", 6)==0){
				//printf("%s\n", flagFilePair[count+1]);
				executeFall(flagFilePair[count+1]);
			}
			else if(strncmp(flagFilePair[count],"-f",2)==0){
				//printf("%s\n", flagFilePair[count+1]);
				executeF(flagFilePair[count+1]);
			}			
			else{
				printf("<<SPA Error: %s is not a flag accepted by SPA. \nAt least one flag should be given as the first argument. \nUse -help to see all suported flags>>\n", flagFilePair[number_of_arguments]);
			}
			count+=2;
			number_of_arguments-=2;
			log_operation_counter++;
		}
	}
}


void executeD(char* file_to_process){
	file_descriptor = lstat(file_to_process, &target_statval);
	file_descriptor = open(file_to_process, O_RDONLY,0);

	ioctl(file_descriptor, FIGETBSZ,&block_size); 

	major = target_statval.st_dev>>8;
	minor = target_statval.st_dev&0xff;

	if(major== 8){
		
		if(minor ==1)
			device_name = "/dev/sda1";
		else if(minor ==2)
			device_name = "/dev/sda2";
	}

	device_descriptor = open(device_name, O_WRONLY,0);

	statval = &target_statval;

	statval = &private_statval;

	fstat(file_descriptor,(struct stat*)statval); 

	block_count = target_statval.st_size/block_size;
	if(target_statval.st_size%block_size > 0){
		block_count++;
	}

	file_size = target_statval.st_size;
	x = 0;

	for(x=0; x<block_count; x++){
		block=x;
		ioctl(file_descriptor,FIBMAP, &block);
	}

	offset=((long long)block)*block_size+(file_size%block_size);
	
	if(!(file_size%block_size))
		slack_bytes=0;
	else
		slack_bytes=block_size-file_size%block_size;


	block_buffer = (unsigned char*)malloc(slack_bytes);

	seek= 0;

	if((seek=lseek(device_descriptor,offset,SEEK_SET))!=offset){
		
		printf("Name of file: %s", file_to_process);
		printf("\nName of device: %s\n", device_name); 
		perror("lseek");
		exit(-1);	
	}


	
	memset(block_buffer,0,slack_bytes);

	if(write(device_descriptor,block_buffer,slack_bytes)<0){
		perror("write error ");
	}
		writeResultOnLog("-d", file_to_process, "file wiped");
	close(file_descriptor);
	close(device_descriptor);
	/*read(device_descriptor,block_buffer,slack_bytes);
	printf("%s\n",block_buffer);*/
	
}
void executeDall(char * const path_to_process[]){

		char* file_name = NULL;
	FTS *ftsp;
	FTSENT *p, *chp;
	int fts_options = FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR;
	int rval = 0;

	if ((ftsp = fts_open(path_to_process, fts_options, NULL)) == NULL) {
		warn("fts_open");
		exit(-1);
	}

	chp = fts_children(ftsp, 0);

	if (chp == NULL) {
		exit(0);               /* no files to traverse */
	}

	while ((p = fts_read(ftsp)) != NULL) {
		switch (p->fts_info) {
			case FTS_D:
					
					//do nothing
					break;
			case FTS_F:
			
					file_name = p->fts_path;
					printf("%s\n",file_name );
					//checkFileSlackSpace(file_name);
					executeD(file_name);
					break;
			default:
					break;
					}
			}
	fts_close(ftsp);
}



void executeF(char* file_to_process){
	file_descriptor = lstat(file_to_process, &target_statval);
	file_descriptor = open(file_to_process, O_RDONLY,0);

	ioctl(file_descriptor, FIGETBSZ,&block_size); 

	major = target_statval.st_dev>>8;
	minor = target_statval.st_dev&0xff;

	if(major== 8){
		
		if(minor ==1)
			device_name = "/dev/sda1";
		else if(minor ==2)
			device_name = "/dev/sda2";
	}

	device_descriptor = open(device_name, O_RDONLY,0);

	statval = &target_statval;

	statval = &private_statval;

	fstat(file_descriptor,(struct stat*)statval); 

	block_count = target_statval.st_size/block_size;
	if(target_statval.st_size%block_size > 0){
		block_count++;
	}

	file_size = target_statval.st_size;
	x = 0;

	for(x=0; x<block_count; x++){
		block=x;
		ioctl(file_descriptor,FIBMAP, &block);
	}

	offset=((long long)block)*block_size+(file_size%block_size);
	
	if(!(file_size%block_size))
		slack_bytes=0;
	else
		slack_bytes=block_size-file_size%block_size;


	block_buffer = (unsigned char*)malloc(slack_bytes);

	seek= 0;

	if((seek=lseek(device_descriptor,offset,SEEK_SET))!=offset){
		
		printf("Name of file: %s", file_to_process);
		printf("\nName of device: %s\n", device_name); 
		perror("lseek");
		exit(-1);	
	}

	read(device_descriptor,block_buffer,slack_bytes);
	//printf("%s\n",block_buffer);
	writeResultOnLog("-f", file_to_process, block_buffer);
	close(file_descriptor);
	close(device_descriptor);
}

void executeFall(char* file_to_process){
	file_descriptor = lstat(file_to_process, &target_statval);
	file_descriptor = open(file_to_process, O_RDONLY,0);

	ioctl(file_descriptor, FIGETBSZ,&block_size); 

	major = target_statval.st_dev>>8;
	minor = target_statval.st_dev&0xff;

	if(major== 8){
		
		if(minor ==1)
			device_name = "/dev/sda1";
		else if(minor ==2)
			device_name = "/dev/sda2";
	}

	device_descriptor = open(device_name, O_RDONLY,0);

	statval = &target_statval;

	statval = &private_statval;

	fstat(file_descriptor,(struct stat*)statval); 

	block_count = target_statval.st_size/block_size;
	if(target_statval.st_size%block_size > 0){
		block_count++;
	}

	file_size = target_statval.st_size;
	x = 0;

	for(x=0; x<block_count; x++){
		block=x;
		ioctl(file_descriptor,FIBMAP, &block);
	}

	offset=((long long)block)*block_size+(file_size%block_size);
	
	if(!(file_size%block_size))
		slack_bytes=0;
	else
		slack_bytes=block_size-file_size%block_size;

	block_buffer = (unsigned char*)malloc(slack_bytes);

	seek= 0;

	if((seek=lseek(device_descriptor,offset,SEEK_SET))!=offset){
		
		printf("Name of file: %s", file_to_process);
		printf("\nName of device: %s\n", device_name); 
		perror("lseek");
		exit(-1);	
	}
	

	read(device_descriptor,block_buffer,slack_bytes);
	
	int i;
	
	for(i = 0; i < slack_bytes; i++){
		
		printf("%x",block_buffer[i]);
		
	}
	
	writeResultOnLog("-f-all", file_to_process, block_buffer);
	close(file_descriptor);
	close(device_descriptor);
}

void checkFileSlackSpace(char* current_file){ //old Run
	
	if(log_count==15){
		log_count = 0;
		sleep(2);
	}
	char * bad_file = "/usr/bin/gpp-decrypt";
	
	if(strcmp(bad_file, current_file) != 0){
			
			file_descriptor = lstat(current_file, &target_statval);
			file_descriptor = open(current_file, O_RDONLY,0);

			ioctl(file_descriptor, FIGETBSZ,&block_size); 

			major = target_statval.st_dev>>8;
			minor = target_statval.st_dev&0xff;

			if(major== 8){
				if(minor ==1)
					device_name = "/dev/sda1";
				else if(minor ==2)
					device_name = "/dev/sda2";
			}

			device_descriptor = open(device_name, O_RDONLY,0);

			statval = &target_statval;

			statval = &private_statval;

			fstat(file_descriptor,(struct stat*)statval); 

			block_count = target_statval.st_size/block_size;
			if(target_statval.st_size%block_size > 0){
				block_count++;
			}

			file_size = target_statval.st_size;
			x = 0;

			for(x=0; x<block_count; x++){
				block=x;
				ioctl(file_descriptor,FIBMAP, &block);
			}

			offset=((long long)block)*block_size+(file_size%block_size);
			
			if(!(file_size%block_size))
				slack_bytes=0;
			else
				slack_bytes=block_size-file_size%block_size;


			block_buffer = (unsigned char*)malloc(slack_bytes);

			seek= 0;

			if((seek=lseek(device_descriptor,offset,SEEK_SET))!=offset){
				
				printf("Name of file: %s", current_file);
				printf("\nName of device: %s\n", device_name); 
				perror("lseek");
				exit(-1);	
			}

			read(device_descriptor,block_buffer,slack_bytes);
			
			char * aux = block_buffer;
			int block_length = 0;
			
			while (*(aux++)) {
				block_length++;
			}

			if(block_length!= 0){
				
				if(file_counter==0)  
					writeResultOnLog("hiddenInit", "", "");

				char info[10];
				sprintf(info, "%d", block_length);
				writeResultOnLog("fileDataFound", current_file, info); 

				file_counter++;
			}
			
			log_count++;
			close(file_descriptor);
			close(device_descriptor);
	}
}

void executeP(char * const path_to_process[]){ //Old iterate_files

	char* file_name = NULL;
	FTS *ftsp;
	FTSENT *p, *chp;
	int fts_options = FTS_COMFOLLOW | FTS_LOGICAL | FTS_NOCHDIR;
	int rval = 0;

	if ((ftsp = fts_open(path_to_process, fts_options, NULL)) == NULL) {
		warn("fts_open");
		exit(-1);
	}

	chp = fts_children(ftsp, 0);

	if (chp == NULL) {
		exit(0);               /* no files to traverse */
	}

	while ((p = fts_read(ftsp)) != NULL) {
		switch (p->fts_info) {
			case FTS_D:
					
					//do nothing
					break;
			case FTS_F:
			
					file_name = p->fts_path;
					checkFileSlackSpace(file_name);
					break;
			default:
					break;
					}
			}
	fts_close(ftsp);
}

void createLogFile(){
	
	FILE *fptr;

	if(fopen("log.txt","r")==NULL){
		fptr = fopen("log.txt","w+");

		if(fptr == NULL){
			printf("<<SPA Error: log.txt file could not be created>>");   
			exit(1);             
		}
		fclose(fptr);
	}
}

void initLogEntry(){
	FILE *fptr;
		
	fptr = fopen("log.txt","a");

	if(fptr == NULL){
		printf("<<SPA Error: log.txt file could not be opened for log entry initialization>>");   
		exit(1);             
	}

	//Get current time
	time_t t;
	struct tm * timeinfo;

	time(&t);
	timeinfo = localtime(&t);
	
	fprintf(fptr,"--------------------------------------------------------------------------------------------\nNew log file entry at %s\n\n", asctime(timeinfo));
	fclose(fptr);
}

void writeResultOnLog(char* operation_name, char* file_analysed, char* data_to_write){
	FILE *fptr;
	
	fptr = fopen("log.txt","a");

	if(fptr == NULL){
		printf("<<SPA Error: log.txt file could not be opened for log result write>>");
		fclose(fptr);   
		exit(1);             
	}
	if(strcmp(operation_name, "-f-all")==0){ // Writes data in octal
		fprintf(fptr, "Input #%d\nRequested operation %s on %s\nResult was:\n%o\n\n",log_operation_counter, operation_name, file_analysed, data_to_write);
	}
	else if(strcmp(operation_name, "hiddenInit")==0){
		fprintf(fptr,"########################## FILES FOUND WITH DATA IN SLACK SPACE ##########################\n\n");
	}	
	else if(strcmp(operation_name, "fileDataFound")==0){
		fprintf(fptr,"FILE:  %s >> %s slack space being used\n", file_analysed, data_to_write);
	}	
	else{
		fprintf(fptr, "Input #%d\nRequested operation %s on %s\nResult was:\n%s\n",log_operation_counter, operation_name, file_analysed, data_to_write);
	}
	fclose(fptr);
}

