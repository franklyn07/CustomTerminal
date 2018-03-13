#include <ncurses.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdio.h>
#define COLUMNS 256


void initialiser(); //initialises panels
void chdirFunction(); //chdir function
void arraycleaner(char buffer[]); //empties arrays whcih contain commands, paths texts etc.
void clean_and_update();//calls array cleaner on the arrays and updates the output panels
void finalCleanup(); //called to close all the windows
void tokenizer(); //splits commands into command/path/text
void getInput(); //reads input from user
void printvarFunct();//prints the environment string
void bufferWriter(char outputBuffers []); //writes the temporary buffer to the output buffer
void bufferReader(); //ouptus the output buffer to the output panel
void file_to_array(FILE* file); //takes pointer to a file and saves it in an array
char* dateconverter(int boolean,int toconvert);//converts days and months

//Task 2 methods & variables
void signalhandler(int signal);
void datepanelupdate();
int timer= 0; //calculates time between one signal cuaght and another

int x, y; //store max size of windows
int sizeSmallPanels_x, sizeLargePanels_x, sizeSmallPanels_y, sizeLargePanels_y, smallestPanels_y; //holds values of the sizes of diff panels

WINDOW *Main;
WINDOW *Date_Panel;
WINDOW *Output_Panel;
WINDOW *Prompt_Panel;
WINDOW *Alarm_Panel;
WINDOW *Colour_Panel;

char commandBuffer[256];
char command[16];
char filepath[240];
char output[240];
char setcommand[240];
char setinput[240];
char filename[240];//stores the name of the file to be created
char output2[240];//stores file input for print
char prompt[80] = "OK>";
char filestorage[999999];//array which stores contents of file
int row = 1; //static printing line
int col =0; //static printing column
int movValue = 0; //default value for move function

//building buffer
struct line{
    char columns [COLUMNS];
}bufferRows[9999];
int buffer_rowCounter = 0;
int userSize = 80;

//shared memory segment variables
int shmid1,shmid2,shmid3;
int *sleeptimer_1;
int *status_1;
int *column_1;

int *column;//declared global such that method get input can access it ()
int *sleeper;
int *status;
    
key_t key1 = 0x1234; //memory segment identifier
key_t key2 = 0x5678; 
key_t key3 = 0x2468;

int main() {
    FILE *testing = fopen("debugging","a+");
    int closeShell = 0;

    initialiser();
        
    pid_t pid1;
    pid1 = fork();
    
    if(pid1<0){
        perror("Fork error!");
    }else if (pid1 == 0){
    //child process
        fputs("Child process:",testing);
        char tmp2[256];

	    fputs("Producer Started\n",testing);
	    // Create the segments.
	    if ((shmid1 = shmget(key1, 1, IPC_CREAT|0666)) < 0){
  		    fputs("shmid1 child",testing);
  		    exit(1);
	    }
	    
	    if ((shmid2 = shmget(key2, 1, IPC_CREAT|0666)) < 0) {
  		    fputs("shmid2 child",testing);
  		    exit(1);
	    }
	    
	    if ((shmid3 = shmget(key3, 1, IPC_CREAT|0666)) < 0) {
  		    fputs("shmid3 child",testing);
  		    exit(1);
	    }

	    // Attach to the segment.
	    if ((sleeptimer_1= shmat(shmid1, NULL, 0)) == (int *) -1) {
	 	    fputs("sleep timer attachment in child",testing);
	  	    exit(1);
	    }
	    
	    if ((status_1 = shmat(shmid2, NULL, 0)) == (int *) -1) {
	 	    fputs("status attachment in child",testing);
	  	    exit(1);
	    }
	    
   	    if ((column_1 = shmat(shmid3, NULL, 0)) == (int *) -1) {
	 	    fputs("column position attachment in child",testing);
	  	    exit(1);
	    }


	    sleeper = (int*)sleeptimer_1;
	    status = (int* )status_1;
	    column = (int* )column_1;
	    
	    status[0] = 0;
	    sleeper[0] = 1;
	    column[0] = 0;
        
	    while(status[0] == 0)
	    {
            int WhiteHouse = -5;
            int Malta = 2;
            int Japan = 9;
    
            time_t rawtime;
            struct tm *info;
    
            time(&rawtime);
            info = gmtime(&rawtime);

            mvwprintw(Date_Panel,2,0,"WHITE HOUSE [USA]: %2d:%02d:%02d -> %s,%d %s,%d",(info->tm_hour+WhiteHouse)%24, info->tm_min, info->tm_sec, dateconverter(1,info->tm_wday),info->tm_mday,dateconverter(0,info->tm_mon),info->tm_year+1900);
            mvwprintw(Date_Panel,3,0,"MSIDA [MALTA]: %2d:%02d:%02d -> %s,%d %s,%d",(info->tm_hour+Malta)%24, info->tm_min, info->tm_sec, dateconverter(1,info->tm_wday),info->tm_mday,dateconverter(0,info->tm_mon),info->tm_year+1900);
            mvwprintw(Date_Panel,4,0,"TOKYO [JAPAN]: %2d:%02d:%02d -> %s,%d %s,%d",(info->tm_hour+Japan)%24, info->tm_min, info->tm_sec, dateconverter(1,info->tm_wday),info->tm_mday,dateconverter(0,info->tm_mon),info->tm_year+1900);
            
            wmove(Prompt_Panel,row,column[0]);
            wrefresh(Date_Panel);
            wrefresh(Prompt_Panel);
            sleep(sleeper[0]);
	    }

	    fputs("Producer Ended\n",testing);

        //clearance
        shmdt(sleeptimer_1);
        shmdt(status_1);
        shmdt(column_1);
        if(shmctl(shmid1, IPC_RMID ,NULL) == -1){
		    fputs("deleting status shm",testing);
		    exit(1);
	    }
	    
	    if(shmctl(shmid2, IPC_RMID ,NULL) == -1){
		    fputs("deleting sleeptimer shm",testing);
		    exit(1);
	    }
	    
	    if(shmctl(shmid3, IPC_RMID ,NULL) == -1){
		    fputs("deleting column shm",testing);
		    exit(1);
	    }
	    
    }else{
    //parent process
        
        pid_t pid2 = fork();
        
        if (pid2>0){
        //parent
        
            fputs("Parent1 process:",testing);
            fputs("Consumer1 started",testing);
        
            sleep(2); //give enough time to producer to get memory segment
        
    	    if ((shmid1 = shmget(key1, 1,0666)) < 0) {
	            fputs("shmid1 parent",testing);
	            exit(1);
	        }
	    
	   	    if ((shmid2 = shmget(key2,1,0666)) < 0) {
	          fputs("shmid1 parent",testing);
	          exit(1);
	        }
	    
	   	    if ((shmid3 = shmget(key3,1,0666)) < 0) {
	          fputs("shmid1 parent",testing);
	          exit(1);
	        }	    
	   

	        // Attach to the segment.
	        if ((sleeptimer_1 = shmat(shmid1, NULL, 0)) == (int *) -1) {
	          fputs("sleeptimer attachment in parent",testing);
	          exit(1);
	        }
	    
	        if ((status_1 = shmat(shmid2, NULL, 0)) == (int *) -1){
	          fputs("status attachment in parent",testing);
	          exit(1);
	        }
	    
	        if ((column_1 = shmat(shmid3, NULL, 0)) == (int *) -1){
	          fputs("column attachment in parent",testing);
	          exit(1);
	        }

	        sleeper = (int*)sleeptimer_1;
	        status = (int*)status_1;
            column = (int*)column_1;
    
            do {            
                getInput();
    
                tokenizer();

                if ((strcmp(command, "chdir")) == 0) {
                    chdirFunction();
                } else if ((strcmp(command, "shdir")) == 0) {
                    if((strcmp(filepath, ">")) == 0){
                        char tmp[240]; //temporary array which stores cwd for file to save
                        FILE *file = fopen(filename,"a+");
                        if(file == NULL){
                            strcpy(tmp,"Error creating file!");
                            bufferWriter(tmp);
                            bufferReader();
                            arraycleaner(tmp);
                        }else {
                            getcwd(tmp, sizeof(tmp));
                            fwrite(tmp, 1, strlen(tmp), file);
                            fwrite("\n", 1, 1, file);
                            arraycleaner(tmp);
                            strcpy(tmp, "Saved Successfully!");
                            bufferWriter(tmp);
                            bufferReader();
                            arraycleaner(tmp);
                        }
                        fclose(file);
                    }else {
                        getcwd(filepath, sizeof(filepath));
                        bufferWriter(filepath);
                        bufferReader();
                    }
                } else if ((strcmp(command, "print")) == 0) {
                    if((strcmp(filepath, ">")) == 0){
                        char tmp[240]; //temporary array which stores user interaction output
                        FILE *file = fopen(filename,"a+");
                        if(file == NULL){
                            strcpy(tmp,"Error creating file!");
                            bufferWriter(tmp);
                            bufferReader();
                            arraycleaner(tmp);
                        }else {
                            fwrite(output2, 1, strlen(output2), file);
                            fwrite("\n", 1, 1, file);
                            arraycleaner(tmp);
                            strcpy(tmp, "Saved Successfully!");
                            bufferWriter(tmp);
                            bufferReader();
                            arraycleaner(tmp);
                        }
                        fclose(file);
                    }else{
                        bufferWriter(output);
                        bufferReader();
                    }
                } else if ((strcmp(command, "printvar")) == 0) {
                    printvarFunct();
                } else if ((strcmp(command, "set")) == 0) {
                    if ((strcmp(setcommand, "prompt")) == 0) {
                        arraycleaner(&prompt[0]);//clears previous input
                        strcpy(prompt, strcat(setinput, ">"));
                    } else if ((strcmp(setcommand, "path")) == 0) {
                        setenv("PATH",setinput, 1);
                    } else if ((strcmp(setcommand, "refresh")) == 0) {
                        sleeper[0]=atoi(setinput);
                    } else if ((strcmp(setcommand, "buffer")) == 0) {
                        userSize = atoi(setinput);
                        strcat(setinput,":New buffer size!");
                        bufferWriter(setinput);
                        bufferReader();
                    } else {
                        strcpy(setcommand,"Unkown command!");
                        bufferWriter(setcommand);
                        bufferReader();
                    }

                } else if ((strcmp(command, "move")) == 0) {
                    movValue += atoi(filepath);
                    if(movValue < 0 || movValue>userSize){
                        strcat(setinput,"Error!");//just found an empty buffer and used it as a temporary storage
                        movValue -= atoi(filepath);//revert mathematical operation
                        bufferWriter(setinput);
                        bufferReader();
                    } else {
                        bufferReader();
                    }
                } else if (strcmp(command, "exit") == 0) {
                    closeShell++;
                    
                    status[0] = 1;
                    fputs("Consumer Ended\n",testing);
                    shmdt(sleeptimer_1);
                    shmdt(status_1);
                    shmdt(column_1);
                
                } else if ((strcmp(command, "clear")) == 0){
                    buffer_rowCounter = userSize; //this will activate condition in buffer writer, that clears the buffer
                    bufferReader();
                } else {
                    FILE *f;
                    if((f = popen(commandBuffer, "r")) == NULL) {
                        fputs("External command not found!",testing);
                    }else{
                        file_to_array(f);
                        pclose(f);
                        bufferWriter(filestorage);
                        bufferReader();
                    }
                }

                clean_and_update();
            }while(closeShell == 0);
    
            fclose(testing);
            finalCleanup();
        }else if (pid2 == 0){
            pid_t pid3; 
            
            //child process
            sleep(0.5); //gives enough time to date and time panel to print the time (w/o moving cursor)
            
            //looking for shm
            fputs("Child2 process:",testing);
            fputs("Consumer2 started",testing);
        
	    
	   	    if ((shmid2 = shmget(key2,1,0666)) < 0) {
	          fputs("shmid1 parent",testing);
	          exit(1);
	        }
	    
	   	    if ((shmid3 = shmget(key3,1,0666)) < 0) {
	          fputs("shmid1 parent",testing);
	          exit(1);
	        }	    
	   

	        // Attach to the segment.
	    
	        if ((status_1 = shmat(shmid2, NULL, 0)) == (int *) -1){
	          fputs("status attachment in parent",testing);
	          exit(1);
	        }
	    
	        if ((column_1 = shmat(shmid3, NULL, 0)) == (int *) -1){
	          fputs("column attachment in parent",testing);
	          exit(1);
	        }

	        status = (int*)status_1;
            column = (int*)column_1;
            
            Alarm_Panel = newwin(sizeSmallPanels_y,2*(sizeSmallPanels_x/3),0,sizeSmallPanels_x);
            Colour_Panel = newwin(sizeSmallPanels_y,(sizeSmallPanels_x/3),0,sizeSmallPanels_x+(2*(sizeSmallPanels_x/3)));

            wprintw(Alarm_Panel,"Alarm Panel\n");

            init_pair(3,COLOR_BLACK,COLOR_BLUE);
            init_pair(8,COLOR_BLACK,COLOR_WHITE);
            init_pair(5,COLOR_BLACK,COLOR_RED);
            init_pair(6,COLOR_BLACK,COLOR_GREEN);
            init_pair(7,COLOR_BLACK,COLOR_YELLOW);
            
            wbkgd(Colour_Panel,COLOR_PAIR(8));
            wbkgd(Alarm_Panel,COLOR_PAIR(1));
            wrefresh(Colour_Panel);
            wrefresh(Alarm_Panel);
            
            pid3 = fork();
            
            if(status[0]==1){
                kill(pid3,SIGTERM);
            }
            
            if(pid3 == 0){
                //child prcess
                
                //call pressbloc
                sleep(0.5); // to wait for parent process to start before it
                int pid_curr = getppid();
                char buff [10];
                snprintf(buff,sizeof(buff),"%d",pid_curr);
                char * const currentpid_c [] ={"presblock",buff, NULL} ; 
                
                execvp("./presblock",currentpid_c);
                
            }else if (pid3>0){
                //paretn process
                
                signal(SIGALRM, signalhandler);
                while(status[0]==0){
                    sleep(1);
                    timer++;
                }
                
                shmdt(status_1);
                shmdt(column_1);
                
            }
        }
    }    
    return EXIT_SUCCESS;
}

void initialiser(){
    if((Main = initscr()) == NULL){
        fprintf(stderr, "Error initialising ncurses.\n");
        exit(EXIT_FAILURE);
    };


    getmaxyx(Main,y,x);
    sizeSmallPanels_x = x/2;
    sizeSmallPanels_y = y/4;
    sizeLargePanels_x = x;
    sizeLargePanels_y = (13*y)/20;
    smallestPanels_y = y/10;
    echo();
    start_color();
    init_color(COLOR_WHITE,450,450,450);
    init_color(COLOR_BLACK,0,0,0);
    init_color(COLOR_YELLOW,235,127,80);//constructing orange
    init_pair(1,COLOR_GREEN,COLOR_BLACK);
    init_pair(2,COLOR_BLACK,COLOR_WHITE);
    init_pair(4,COLOR_BLACK,COLOR_CYAN);
    Date_Panel = newwin(sizeSmallPanels_y,sizeSmallPanels_x,0,0);
    Output_Panel = newwin(sizeLargePanels_y,sizeLargePanels_x,sizeSmallPanels_y,0);
    Prompt_Panel = newwin(sizeSmallPanels_y,sizeLargePanels_x,sizeLargePanels_y+sizeSmallPanels_y,0);
    wprintw(Date_Panel,"Date Panel\n");
    wprintw(Output_Panel,"Output Panel\n");
    wprintw(Prompt_Panel,"Prompt Panel\n");
    wbkgd(Date_Panel,COLOR_PAIR(2));
    wbkgd(Output_Panel,COLOR_PAIR(4));
    wbkgd(Prompt_Panel,COLOR_PAIR(1));

    wrefresh(Date_Panel);
    wrefresh(Output_Panel);
    wrefresh(Prompt_Panel);
}

void arraycleaner(char buffer[]){
    memset(&buffer[0],0,strlen(buffer));
}

void clean_and_update(){
    arraycleaner(&commandBuffer[0]);
    arraycleaner(&command[0]);
    arraycleaner(&filepath[0]);
    arraycleaner(&output[0]);
    arraycleaner(&setcommand[0]);
    arraycleaner(&setinput[0]);
    arraycleaner(&filestorage[0]);
    arraycleaner(&filename[0]);
    arraycleaner(&output2[0]);

    wclrtobot(Prompt_Panel);
    wnoutrefresh(Prompt_Panel);
    wnoutrefresh(Output_Panel);
    wnoutrefresh(Date_Panel);
    doupdate();
}

void chdirFunction(){
    if (filepath[0] == '\0'){
        wprintw(Output_Panel,"expected path : /home/franklyn07/...");
    }else {
        if(chdir(filepath)!=0){
            wprintw(Output_Panel,"%s: No such file or directory!",filepath);
        }
    }
}

void finalCleanup(){
    delwin(Prompt_Panel);
    delwin(Alarm_Panel);
    delwin(Date_Panel);
    delwin(Output_Panel);
    delwin(Main);
    endwin();
    refresh();
}

void tokenizer(){
    int counter_commandBuff = 0;
    int counter_command = 0;
    int counter_output = 0;
    int counter_filepath = 0;
    int counter_setcommand = 0;
    int counter_setinput = 0;
    int counter_filename = 0;
    int counter_output2 = 0;//counter for buffer which will be filled only by text file for the print function 
    int spaces = 0;
    bool isSetCommand = false;
    while(commandBuffer[counter_commandBuff] != NULL){
        if(commandBuffer[counter_commandBuff] == 32){
            spaces++;
        }
        if(spaces == 0){
            command[counter_command] = commandBuffer[counter_commandBuff];
            counter_command++;
        }else if(spaces == 1 && commandBuffer[counter_commandBuff] != 32){
            filepath[counter_filepath] = commandBuffer[counter_commandBuff];
            output[counter_output] = commandBuffer[counter_commandBuff];
            counter_filepath++;
            counter_output++;
            //taking this approach avoids saving the'=' : 61
            if(commandBuffer[counter_commandBuff] == 61){
                isSetCommand = true;
                goto Escape;
            }
            if(!isSetCommand) {
                setcommand[counter_setcommand] = commandBuffer[counter_commandBuff];
                counter_setcommand++;
            }else{
                setinput[counter_setinput] = commandBuffer[counter_commandBuff];
                counter_setinput++;
            }
        }else if (spaces > 1) {//saves spaces as well
            output[counter_output] = commandBuffer[counter_commandBuff];
            counter_output++;
            if (spaces == 2){//saves the new file name
                filename[counter_filename] = commandBuffer[counter_commandBuff];
                counter_filename++;
            }else if (spaces>2){
                int localcounter_commandBuff = counter_commandBuff;
                localcounter_commandBuff++;
                //the previous two lines avoid saving the first space by moving the commandbuff by 1 offset
                output2[counter_output2] = commandBuffer[localcounter_commandBuff];
                counter_output2++;
            }
        }
        Escape:
        counter_commandBuff++;
    }
}

void getInput(){
    int tmp = 0;//index used to update last entry in array of chars with a null to terminate input
    char c;
    mvwprintw(Prompt_Panel,row,col,prompt);
    col+=strlen(prompt);
    column[0] = col;
    while((c = (char)mvwgetch(Prompt_Panel,row,col))!= 10){
        if(c==127 && col >strlen(prompt)) {
            mvwprintw(Prompt_Panel, row, col, " ");
            mvwprintw(Prompt_Panel, row, col - 1, " ");
            mvwprintw(Prompt_Panel, row, col + 1, " ");
            col -= 1;
            wrefresh(Prompt_Panel);
            commandBuffer[tmp - 1] = '\0';
            tmp--;
        }else{
            commandBuffer[tmp] = c;
            col++;
            tmp++;
        }
        column[0] = col;
    }
    col =0;
    column[0] = 0;
    commandBuffer[tmp] = NULL;
}

void printvarFunct(){
    int i = 0;
    char local_buff[256];
    arraycleaner(local_buff);
    
    if((strcmp(filepath, ">")) == 0){
        strcpy(local_buff,output2);
        //the function only takes variables in capslock so we convert each character toupper
        while(i<strlen(local_buff)){
            local_buff[i] = (char) toupper(local_buff[i]);
            i++;
        };
        char tmp[240]; //temporary array which stores user interaction output
        FILE *file = fopen(filename,"a+");
        if(file == NULL){
            strcpy(tmp,"Error creating file!");
            bufferWriter(tmp);
            bufferReader();
            arraycleaner(tmp);
        }else {
            if(getenv(local_buff)!=NULL){
                fwrite(getenv(local_buff), 1, strlen(getenv(local_buff)), file);
                fwrite("\n", 1, 1, file);
                arraycleaner(tmp);
                strcpy(tmp, "Saved Successfully!");
                bufferWriter(tmp);
                bufferReader();
                arraycleaner(tmp);
            }else{
                arraycleaner(local_buff);//clears local buffer to be able to write errror prompt in it
                strcpy(local_buff,"Error occurred!");
                bufferWriter(local_buff);
            };
            fclose(file);
        }
    }else{
        strcpy(local_buff,filepath);
        //the function only takes variables in capslock so we convert each character toupper
        while(i<strlen(local_buff)){
            local_buff[i] = (char) toupper(local_buff[i]);
            i++;
        };
        if(getenv(local_buff)!=NULL){
            bufferWriter(getenv(local_buff));
        }else{
            arraycleaner(local_buff);//clears local buffer to be able to write errror prompt in it
            strcpy(local_buff,"Error occurred!");
            bufferWriter(local_buff);
        };
        bufferReader();
    }
}

void bufferWriter(char outputBuffers []){
    int counter_i = 0; //will count the outputBuffers index
    int counter_j = 0; //will count the columns of buffer

    if(buffer_rowCounter>=userSize) {
        for (int i = 0; i < userSize; i++) {
            arraycleaner(bufferRows[i].columns);
        }
        bufferReader();//clears screen
        buffer_rowCounter = 0;//resets index of buffer to 0
    }
    while (counter_i < strlen(outputBuffers)) {
        if (counter_i > COLUMNS || outputBuffers[counter_i] == '\n') { // if horizontal buffer size is exceeded or a new line character is encountered, it will continue to print in a new line
            counter_j = 0;
            buffer_rowCounter++;
            counter_i++;
        } else {
            bufferRows[buffer_rowCounter].columns[counter_j] = outputBuffers[counter_i];
            counter_i++;
            counter_j++;
        }
    }
    buffer_rowCounter++;
}

void bufferReader(){
    int localrowcounter = 1;//determine where buffer reader will print
    int localcolumncounter = 0;
    werase(Output_Panel);
    wprintw(Output_Panel, "Output Panel");
    for(int i = movValue; i <buffer_rowCounter; i++){
        mvwprintw(Output_Panel,localrowcounter,localcolumncounter,bufferRows[i].columns);
        localrowcounter++;
    }
}

void file_to_array(FILE* file){
    int i = 0;
    while((filestorage[i] = (char)fgetc(file))!=EOF){
        i++;
    }
    filestorage[i] = '\0';
    for(int j = 0; j<strlen(filestorage);j++){
        if(filestorage[j] == '\n'){
            filestorage[j] = 32;
        }
    }
}

char * dateconverter(int boolean, int toconvert){
    // 1 will mean its a day of the week
    // 0 will mean its a month
    
    static char* months[] ={"January","February","March","April","May","June","July","August","September","October","November","December"};
    static char* days[] ={"Sunday","Monday","Teusday","Wednesday","Thursday","Friday","Saturday"};
    
    if (boolean == 1){
        return days[toconvert];
    }else if (boolean ==0){
        return months[toconvert];
    }else{
        return 1;
    }
}

void signalhandler(int signal){
    if(signal == SIGALRM){
        wmove(Alarm_Panel,1,0);
        mvwprintw(Alarm_Panel,1,0,"%d seconds from last signal!",timer);
        wrefresh(Alarm_Panel);
        datepanelupdate();
        timer = 0; //if signal is caught we will reset timer to 0.
    }
}

void datepanelupdate(){
    wmove(Colour_Panel,0,0);
    if(timer>20){
        wbkgd(Colour_Panel,COLOR_PAIR(3));
        wrefresh(Colour_Panel);
    }else if(timer <= 20 && timer >= 16){
        wbkgd(Colour_Panel,COLOR_PAIR(6));
        wrefresh(Colour_Panel);
    }else if(timer <= 15 && timer >= 11){
        wbkgd(Colour_Panel,COLOR_PAIR(7));
        wrefresh(Colour_Panel);
    }else if(timer <= 9 && timer >= 5){
        wbkgd(Colour_Panel,COLOR_PAIR(5));
        wrefresh(Colour_Panel);
    }else{
        wbkgd(Colour_Panel,COLOR_PAIR(8));
        wrefresh(Colour_Panel);
    }
    wmove(Prompt_Panel,row,column[0]);
    wrefresh(Prompt_Panel);  
}

