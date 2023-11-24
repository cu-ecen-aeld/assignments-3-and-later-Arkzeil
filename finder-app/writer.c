#include <stdio.h>
#include <syslog.h>

int main(int argc, char *argv[]){
    // first argument is a identifier which will be added to the front every logging message
    // second is option
    openlog("writer", 0 , LOG_USER);

    // 0 is the program name, so 1+2
    if(argc != 3){ 
        //printf("%d argements provided, expecting 2 arguments\n", argc);
        syslog(LOG_ERR, "%d argements provided, expecting 2 arguments\n", argc);
        return 1;
    }   

    char *writefile = argv[1];
    char *writestr = argv[2];

    FILE *file = fopen(writefile, "w");
    if(!file){
        syslog(LOG_ERR, "Error while creating file\n");
        //printf("Error while creating file\n");
        return 1;
    }

    fputs(writestr, file);
    fclose(file);

    syslog(LOG_DEBUG, "Writing %s to %s", argv[2], argv[1]);

    return 0;
}