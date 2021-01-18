/*
 * mq_client.c: Client program
 *           to demonstrate interprocess communication
 *           with POSIX message queues
 *           based on: https://www.softprayog.in/programming/interprocess-communication-using-posix-message-queues-in-linux
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#define SERVER_QUEUE_NAME   "/sp-example-server"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10


int main (int argc, char **argv) 
{   
    if (argc != 3){
        printf("Usage: ./client /client1_name /client2_name\nDon't forget the slash!\n");
        exit(1);
    }


	char client_queue_name [64];
  char *client1_name = argv[1];
  char *client2_name = argv[2];

  strcpy(client_queue_name, client1_name);

  mqd_t qd_server, qd_client, qd_client2;   // queue descriptors	

  char msg_token[32];    
	char in_buffer [MSG_BUFFER_SIZE];	
	char out_buffer [MSG_BUFFER_SIZE];
  // create the client queue for receiving messages from server
	// sprintf (client_queue_name, "/sp_example_client-%d", getpid());
	
	struct mq_attr attr;

	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_MESSAGES;
	attr.mq_msgsize = MAX_MSG_SIZE;
	attr.mq_curmsgs = 0;
    
  printf ("Open client queue..");

  if ((qd_client = mq_open (client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Client: mq_open (client)");
        exit (1);
  }
  printf ("Succesful..");

  if (mq_getattr (qd_client, &attr) == -1) {
      perror("Client: mq_getattr");
      exit(1);
  } 

  int iter = attr.mq_curmsgs;
  // flush all messages remaqining in the queue
  while (iter > 0) {
      if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
          perror("Client: mq_receive");
          exit(1);
      }

      iter--;
  }      



  if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
      perror ("Client: mq_open (server)");
      exit (1);
  }

  // Ask for token 
  if (mq_send (qd_server, client_queue_name, strlen (client_queue_name) + 1, 0) == -1) {
          perror ("Client: Not able to connect to server");
          exit(1);
  }

  // receive response from server

  if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
          perror ("Client: mq_receive");
      exit (1);
  }

  //sprintf(msg_token, "Client-%s\n\n", in_buffer);

  // display token received from server
  printf ("Client: Token received from server: Client-%s\n\n", in_buffer);

  /*
  // check if anyone is there
  if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
          perror ("Client: mq_receive");
      exit (1);
  }
  */

  // open communication with client2
  if ((qd_client2 = mq_open (client2_name, O_WRONLY | O_NONBLOCK)) == -1) {
          printf("Check if client2 queue already exists\n");
          perror ("Client: mq_open (client1)");
          exit (1);
  }


  printf ("Start conversation (Press <ENTER>): ");

  char temp_buf [1024];

  if (fgets (temp_buf, 2, stdin)) { printf("."); }
    while (1) {

        // check if the queue is empty
        if (mq_getattr (qd_client, &attr) == -1) {
            perror("Client: mq_getattr");
            exit(1);
        }

        //printf("Stan...\n");

        /*printf("flags: %ld, max_msg: %ld, max_msg_size: %ld, curmsg: %ld \n\n ", 
            attr.mq_flags,
	        attr.mq_maxmsg,
	        attr.mq_msgsize,
	        attr.mq_curmsgs);
        */
        // printf("Checking any new messages...\n\n");
        // printf("Messages waiting in queue: %ld\n\n", attr.mq_curmsgs);
        
        while (attr.mq_curmsgs != 0) {
       	    if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
                perror("Client: mq_receive");
                exit(1);
            }

            printf("%s: ", client2_name);
            printf("%s", in_buffer);
            printf("\n");
            
            //update queue info
            if (mq_getattr (qd_client, &attr) == -1) {
                perror("Client: mq_getattr");
                exit(1);
            
            }
            printf("%ld", attr.mq_curmsgs);          
        }      
            
        printf("%s: ", client1_name);
        fgets(out_buffer, MSG_BUFFER_SIZE, stdin);
        printf("\n");
        
        if (&out_buffer[0] == ">"){
            printf("Received end signal\n\n");
            break;
        }

        /*
        if ((qd_client2 = mq_open (client2_name, O_WRONLY | O_NONBLOCK)) == -1) {
       	    perror ("Client: mq_open (server)");
       	    exit (1);
        }
        */

       	// send message to client2
        //
        // printf("Sending message...\n");
       	if (mq_send (qd_client2, out_buffer, strlen (out_buffer) + 1, 0) == -1) {
           	perror ("Client: Not able to send message to server");
           	continue;
       	}
        
        // printf("A message was sent\n\n");

        // receive response
        /*
       	if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
           	perror ("Client: mq_receive");
		    exit (1);
  		 }
        */
       	// display received message
       	//printf ("%s: %s\n\n", client2_name, in_buffer);
            
       	// printf ("Ask for a token (Press ): ");
    }


	if (mq_close (qd_client) == -1) {
        perror ("Client: mq_close");
        exit (1);
    }

   	if (mq_unlink (client_queue_name) == -1) {
       	perror ("Client: mq_unlink");
       	exit (1);
   	}
    printf ("Client: bye\n");

    exit (0);
}
