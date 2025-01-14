#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    char *src = user_str;
    char *dest = buff;
    int count = 0;
    int space_added = 0;

    while (*src && count < len) {
        if (*src != ' ' && *src != '\t') {
            *dest++ = *src++;
            count++;
            space_added = 0;
        } else if (!space_added) {
            *dest++ = ' ';
            count++;
            space_added = 1;
            src++;
        } else {
            src++;
        }
    }

    if (*src) {
        return -1;
    }

    while (count < len) {
        *dest++ = '.';
        count++;
    }

    return count;
}

void print_buff(char *buff, int len){
    if (!buff) {
        printf("Error: Buffer is NULL.\n");
        return;
    }
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT
    int count = 0;
    int in_word = 0;

    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) != ' ' && *(buff + i) != '.') {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }

    return count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

void reverse_string(char *buff, int str_len) {
    char *start = buff;
    char *end = buff + str_len - 1;
    char temp;

    while (start < end) {
        temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }

    printf("Reversed String: ");
    for (int i = 0; i < str_len; i++) {
        putchar(*(buff + i));
    }
    printf("\n");
}

void print_words(char *buff, int str_len) {
    printf("Word Print\n----------\n");
    int word_start = 0, word_len = 0, word_num = 1;

    for (int i = 0; i <= str_len; i++) {
        if (*(buff + i) != ' ' && *(buff + i) != '.' && *(buff + i) != '\0') {
            if (word_len == 0) {
                word_start = i;
            }
            word_len++;
        } else if (word_len > 0) {
            printf("%d. ", word_num++);
            for (int j = word_start; j < word_start + word_len; j++) {
                putchar(*(buff + j));
            }
            printf(" (%d)\n", word_len);
            word_len = 0;
        }
    }
}

int replace_word(char *buff, int str_len, char *target, char *replacement) {
    char *pos = buff;
    int target_len = 0;
    int replacement_len = 0;

    while (*(target + target_len) != '\0') target_len++;
    while (*(replacement + replacement_len) != '\0') replacement_len++;

    while (pos < buff + str_len - target_len + 1) {
        if (*pos == *target && strncmp(pos, target, target_len) == 0) {
            int new_str_len = str_len - target_len + replacement_len;

            if (new_str_len > BUFFER_SZ) {
                printf("Warning: Replacement truncated to fit buffer size.\n");
                int temp_replacement_len = BUFFER_SZ - (str_len - target_len);
                memmove(pos + temp_replacement_len, pos + target_len, str_len - (pos - buff) - target_len);
                memcpy(pos, replacement, temp_replacement_len);
                return BUFFER_SZ;
            }

            memmove(pos + replacement_len, pos + target_len, str_len - (pos - buff) - target_len);
            memcpy(pos, replacement, replacement_len);
            return new_str_len;
        }
        pos++;
    }

    printf("Error: Target string not found in buffer.\n");
    return -1;
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    buff = (char *)malloc(BUFFER_SZ);
    if (!buff) {
        printf("Error: Memory allocation failed.\n");
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            reverse_string(buff, user_str_len);
            break;

        case 'w':
            print_words(buff, user_str_len);
            break;
        case 'x':
            if (argc != 5) {
                printf("Error: Incorrect number of arguments for -x\n");
                free(buff);
                exit(1);
            }

            rc = replace_word(buff, user_str_len, argv[3], argv[4]);
            if (rc < 0) {
                free(buff);
                exit(2);
            }

            user_str_len = rc;

            printf("Modified String: ");
            for (int i = 0; i < user_str_len; i++) {
                putchar(*(buff + i));
            }
            printf("\n");
            break;


       	default:
       	    usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff, BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
