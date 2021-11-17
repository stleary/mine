#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/**
 * create 4 threads:
 *      Thread 1: Input thread
 *          read in lines of characters from standard input
 *      Thread 2: Line separator thread
 *          replace every line separator in the input by a space (new line, enter key, \n)
 *      Thread 3: Plus sign thread
 *          replace every pair of plus signs (++) with (^)
 *      Thread 4: Output thread
 *          write processed data to standard out as lines of 80 characters
 * threads must communicate using producer-consumer approach
 * 
 * Input:
 *      line of input = seq of allowed chars that does not include a line separator, followed by a line separator
 *      line separator = new line char, \n
 *      allowed chars = ascii chars from 32 to 126 (space to ~)
 *      stop-processing line = process until receipt of input line that contains only STOP; no processing of text recieved after this
 *      no empty lines
 *      input lines no longer than 1000 chars 
 *      max num lines is 49
 *      no need for error checking input
 * 
 * Output:
 *      80 char line: 80 non-line separator chars plus a line separator
 *      do not wait to produce output only when stop processing line is received
 *          --> output line is produced whenever there is sufficient data available
 *      after STOP is rec'd, program must produce all the 80 char lines it still can based on input rec'd before STOP
 *      no part of STOP should be written to output
 *      ++ to ^ will change the number of chars
 **/



// Number of items that will be produced before the END_MARKER. Note that this number is smaller than the size of the buffer. This means that we can model the buffer as unbounded
#define LINE_LIMIT 80

// Buffer, shared resource
// 50 lines max, 1000 chars per line max
char buffer1[50000];

/*
_ _ _ _ _ _ _ _ _ _ _ _ _
t h i s\n           ^
*/

// Number of items in the buffer, shared resource
int count1 = 0;
// Index where the producer will put the next item
int prod_idx1 = 0;
// Index where the consumer will pick up the next item
int con_idx1 = 0;
// Initialize the mutex
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variables
pthread_cond_t full1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty1 = PTHREAD_COND_INITIALIZER;
int term1 = 0;

char buffer2[50000];
int count2 = 0;
int prod_idx2 = 0;
int con_idx2 = 0;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty2 = PTHREAD_COND_INITIALIZER;
int term2 = 0;

char buffer3[50000];
int count3 = 0;
int prod_idx3 = 0;
int con_idx3 = 0;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty3 = PTHREAD_COND_INITIALIZER;
int term3 = 0;

char buffer4[81];
int count4 = 0;
int idx4 = 0;

/**
 * Put value in the first buffer and increment first counter
 **/
char* put_item1(char *value) {
  strcpy(&buffer1[prod_idx1], value);
  prod_idx1 = (prod_idx1 + strlen(value));
  // TODO: is this right???
  count1++;
  //count1 = count1 + strlen(value);
  return value;
}

/**
 * Put value in the second buffer and increment second counter
 **/
char* put_item2(char *value) {
  strcpy(&buffer2[prod_idx2], value);
  prod_idx2 = (prod_idx2 + strlen(value));
  count2++;
  // TODO: check this
  //count2 = count2 + strlen(value);
  return value;
}

char* put_item3(char *value) {
  strcpy(&buffer3[prod_idx3], value);
  prod_idx3 = (prod_idx3 + strlen(value));
  count3++;
  // TODO: check this
  //count3 = count3 + strlen(value);
  return value;
}






/**
 * turn \n into a space
 **/
void newline_to_space(char *value) {
  for (int i = 0; i < strlen(value); i++) {
    if (value[i] == '\n'){
      value[i] = ' ';
    }
  }
}




/**
 * TODO 
 * turn + into a ^
 **/
void plus_to_carat(char *value) {
  for (int i = 0; i < strlen(value); i++) {
    if (value[i] == '+' && value[i+1] == '+'){
      value[i] = '^';
      for (int j = i+1; j < strlen(value); j++) {
        value[j] = value[j+1];
      }
    }
  }
}

int check_for_stop(char* value) {
  if (strcmp(value, "STOP\n") == 0) {
    return 1;
  }
  return 0;
}




/**
 * First producer thread
 *    reads char* from stdin
 *    stores in buffer1
 *    include \n
 **/

void *producer_stdin_to_buffer1(void *args) {
  int terminate = 0;
  while (1) {
    char* value = calloc(1000, sizeof(char));     // allocate memory for the value
    fgets(value, 1000, stdin);                    // read whatever is in stdin until the newline char into value
    
    pthread_mutex_lock(&mutex1);                  // Lock the mutex before checking where there is space in the buffer
    
    terminate = check_for_stop(value);
    if (terminate == 1) {
      term1 = 1;
    }

    while (count1 != 0) {                         // if count1 is not 0, then there is something in the buffer
      pthread_cond_wait(&empty1, &mutex1);        // Buffer is full. Wait for the consumer to signal that the buffer has space
    }                                             // TODO: BUFFER SHOULD NEVER BE FULL????
    
    put_item1(value);                             // add item to buffer

    pthread_cond_signal(&full1);                  // Signal to the consumer that the buffer is no longer empty
    pthread_mutex_unlock(&mutex1);                // Unlock the mutex
    //printf("PROD %s\n", value);
    if (terminate == 1) {
      break;
    }
  }
  // printf("producer done\n");
  return NULL;
}






/**
 * Get the next item from the first buffer
 **/
char* get_item1() {
  char* value = &buffer1[con_idx1];
  con_idx1 = (con_idx1 + strlen(value));
  count1--;
  //count1 = count1 - strlen(value);
  return value;
}

/**
 * Get the next item from the second buffer
 **/
char* get_item2() {
  char* value = &buffer2[con_idx2];
  con_idx2 = (con_idx2 + strlen(value));
  //count2 = count2 - strlen(value);
  count2--;
  return value;
}


/**
 * Get the next item from the third buffer
 **/
char* get_item3() {
  // char* value = &buffer3[con_idx3];
  // con_idx3 = (con_idx3 + strlen(value));
  // //count3 = count3 - strlen(value);
  // count3--;
  // return value;
    char* value = &buffer3[con_idx3];
  while (strlen(value) >= 80) {
    char toPrint[81];
    strncpy(toPrint, value, 80);
    toPrint[80] = 0;
    printf("%s\n", toPrint);
    con_idx3 += 80;
    value = &buffer3[con_idx3];
  }
  count3--;
  return NULL;
}





/**
 * First consumer thread/second producer thread
 *    reads from buffer1
 *    converts \n to space
 *    stores in buffer2
 **/

void *consumer_buffer1_to_buffer2(void *args) {
  char* value = calloc(1000, sizeof(char));       // TODO: TRY THIS allocate memory for the value
  int terminate = 0;
  while (1) {                            
    pthread_mutex_lock(&mutex1);                  // Lock the mutex before checking if the buffer has data  
    while (count1 == 0) {
      // Buffer is empty. Wait for the producer to signal that the buffer has data
      pthread_cond_wait(&full1, &mutex1);
    }
    if (term1 == 1) {
      terminate = 1;
    }
    value = get_item1();
    pthread_cond_signal(&empty1);                 // Signal to the producer that the buffer has space
    pthread_mutex_unlock(&mutex1);                // Unlock the mutex
    
    newline_to_space(value);                      // convert \n to space

    pthread_mutex_lock(&mutex2);                  // lock mutex2
    while (count2 != 0) {                         // do i need this? buffer will always have space
      pthread_cond_wait(&empty2, &mutex2);
    }
    if (terminate == 1) {
      term2 = 1;
    }
    put_item2(value);                             // Pput item in buffer
    pthread_cond_signal(&full2);
    pthread_mutex_unlock(&mutex2);
    if (terminate == 1) {
      break;
    }
    // Print the message outside the critical section
    //printf("\tCONS1 %s\n", value);
  }
  // printf("consumer1 done\n");
  return NULL;
}


/**
 * Second consumer thread/third producer thread
 *    reads from buffer2
 *    converts + to ^ 
 *    TODO: convert ** to ^
 *    stores in buffer3
 **/

void *consumer_buffer2_to_buffer3(void *args) {
  int terminate = 0;
  char* value = calloc(1000, sizeof(char));       // TODO: TRY THIS allocate memory for the value
  while (1) {                            
    pthread_mutex_lock(&mutex2);                  // Lock the mutex before checking if the buffer has data  
    while (count2 == 0) {
      // Buffer is empty. Wait for the producer to signal that the buffer has data
      pthread_cond_wait(&full2, &mutex2);
    }
    if (term2 == 1) {
      terminate = 1;
    }
    value = get_item2();
    pthread_cond_signal(&empty2);                 // Signal to the producer that the buffer has space
    pthread_mutex_unlock(&mutex2);                // Unlock the mutex
    
    //newline_to_space(value);                      // convert \n to space
    plus_to_carat(value);

    pthread_mutex_lock(&mutex3);                  // lock mutex2
    while (count2 != 0) {                         // do i need this? buffer will always have space
      pthread_cond_wait(&empty2, &mutex2);
    }
    if (terminate == 1) {
      term3 = 1;
    }
    put_item3(value);                             // Pput item in buffer
    pthread_cond_signal(&full3);
    pthread_mutex_unlock(&mutex3);

    // Print the message outside the critical section
    //printf("\tCONS2 %s\n", value);
    if (terminate == 1) {
      break;
    }
  }
  // printf("consumer2 done\n");
  return NULL;
}



/**
 * Third consumer thread
 *    reads from buffer3
 *    writes to stdout
 *    TODO: only print when 80 chars available
 **/


void *consumer_buffer3_to_stdout(void *args) {
  int terminate = 0;
  char* value = calloc(1000, sizeof(char));       // TODO: TRY THIS allocate memory for the value
  while (1) {                            
    pthread_mutex_lock(&mutex3);                  // Lock the mutex before checking if the buffer has data  
    while (count3 == 0) {
      // Buffer is empty. Wait for the producer to signal that the buffer has data
      pthread_cond_wait(&full3, &mutex3);
    }
    if (term3 == 1) {
      terminate = 1;
      // printf("ok\n");
    }
    value = get_item3();
    pthread_cond_signal(&empty3);                 // Signal to the producer that the buffer has space
    pthread_mutex_unlock(&mutex3);                // Unlock the mutex
  
    if (terminate == 1) {
      break;
    }

    // strcpy(&buffer4[idx4], value);
    // idx4 = (idx4 + strlen(value));


    // // Print the message outside the critical section
    // // TODO: remove idx4 from output
    // if(idx4 > 9) {
    //   // printf("%s\n%d\n", buffer4, idx4);
    //   printf("%s\n", buffer4)
    //   idx4=0;
    // }
  }
  // printf("consumer3 done\n");
  return NULL;
}


int main() {
  pthread_t p1, c1, c2, c3;
  // thread to read from stdin to buffer1
  pthread_create(&p1, NULL, producer_stdin_to_buffer1, NULL);
  
  // thread to read from buffer1, convert \n to space, and store in buffer2
  pthread_create(&c1, NULL, consumer_buffer1_to_buffer2, NULL);
  
  // thread to read from buffer2, convert ++ to ^ (TODO), and store in buffer3
  pthread_create(&c2, NULL, consumer_buffer2_to_buffer3, NULL);

  // thread to read from buffer3 and print to stdout 80 chars at a time (TODO)
  pthread_create(&c3, NULL, consumer_buffer3_to_stdout, NULL);

  pthread_join(p1, NULL);
  pthread_join(c1, NULL);
  pthread_join(c2, NULL);
  pthread_join(c3, NULL);

  return 0;
}
