// This function is run from  the client application(at one or various
// times) to initiate requests and get responses from an Arduino motor
// control  Server,  called  arduinoServer(as).  This  function/client
// connects to the arduinoServer via tcpip sockets.
//
// The  arduinoServer   controls  a  servo  motor,  thus   it  is  the
// servoServer.  It also controls the  a stepper motor, thus it is the
// stepperServer  in  this case.  Both  the  seperate servoServer  and
// stepperServer   can   be  generated   from   the   one  source   or
// arduinoServer.ino.

// INPUTS: -------------------------------------------------------------------

// dirRotation - motor rotation direction(clockwise=1, counterclockwise=0)
// numRotation - number of motor rotations required; irrelivant for contiuous
//		 servo, only valid for a stepper motor.
// serverID    - is the IPaddr or Hostname of the arduinoServer(servoServer
//		 or stepperServer)
//
// handShake='0'; // to be fushed in server/end comms???

// ---------------------------------------------------------------------------

// OUTPUTS: ------------------------------------------------------------------
// returns 0 all ok

/* NOTE: perhaps need to change these exits to return's so client can
   take more approprite acton for it's application */

// exits (1) - fatal error while trying to open a socket for this client
// exits (2) - fator error while converting hostname to network byte order
// exits (3) - fatal error while trying to connect to server
// exits (4) - fatal error while trying to write to server 
// exits (5) - fatal error while trying to read from server
// exits (6) - fatal error while closing this client's socket fd
// exits (7) - fatal error, invalid numRotation value

// ---------------------------------------------------------------------------
// History:

// 27aug2016	Simone Magri	Original routine
// 05sep2016	Simone Magri	Original code turned into xfer2as() fn
// 07sep2016	Simone Magri	xfer2as() using pass by value
// 07sep2016	Simone Magri	tidy up docs and variables
// 20sep2016	Simone Magri	changes for new arduinoStepperServer

// End History.
 // --------------------------------------------------------------------------

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>




int xfer2as(unsigned char dirRotation, unsigned int numRotation,
	    char *serverID){


  // NOTE:
  // server in varible names means arduinoServer() or as().
  // client in variable names means the program calling this function.
  //
  int client_fd = -1; // this clients' socket file descriptor
  struct sockaddr_in server_sockaddr; // server listen socket address
  const unsigned short int server_port = 23; // server listen socket port
  struct hostent *server_host; // contains server host address in
			       // network byte order, that is, binary
  ssize_t retval;
  unsigned char numRotationByte[4]; // numRotation unsigned integer
				    // broken up into bytes(8bit)/chars.
  int i; // random integer
  

  fprintf(stderr,"xfer2as: dbg: dirRotation=%c, numRotation=%u, serverID=%s\n",
	  dirRotation, numRotation, serverID);


  // Open a tcp socket to allow us to make connections from the client
  // to the server.
  if ( (client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("xfer2as");
    fprintf(stderr, "%s: Error in socket call to server@%s.\n",
	    "xfer2as", serverID);
    exit(1);
  }


  // Specify the server's socket address(TCP IP address and port number)
  // that this client will connect to.
  memset( ((void *) &server_sockaddr), 0, sizeof(server_sockaddr) );
  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_port = htons(server_port); // convert port number
						 // to network byte order
  // Convert server host name to host address in network byte order
  server_host = gethostbyname(serverID);
  if (server_host == ((struct hostent *) NULL)){
    fprintf(stderr, "%s: Error in gethostbyname for server@%s.\n",
	    "xfer2as", serverID);
    exit(2);
  }
  memcpy(&server_sockaddr.sin_addr,server_host->h_addr,server_host->h_length);

  // Connect client's socket to the server's listening socket.
  // (blocks till a connection is made or there is an error)
  if (connect(client_fd, ((struct sockaddr *)&server_sockaddr),
	      sizeof(server_sockaddr)) <0){
    // Connection failed or in progress
    perror("xfer2as");
    fprintf(stderr, "%s: Error in connect to server@%s.\n.",
	    "xfer2as", serverID);
    close(client_fd);
    exit(3);
  }
  

  // Send char/s to server.
  //
  // dirRotation is the required rotation direction of motor.
  //
  // numRotation is the number of rotations required or the motor;
  // irrelivant for a servo motor.
  //
  if (numRotation == 0){
    // servo motor communicating

    // xmit rotation direction
    if ( (retval=write(client_fd, &dirRotation, sizeof(dirRotation)) == -1) ){
      perror("xfer2as");
      fprintf(stderr, "%s: Error in writing to server@%s.\n",
	      "xfer2as", serverID);
      exit(4);
    }
  }
  else if (numRotation > 0){
    // Stepper motor comunicating.

    // xmit rotation direction
    if ( (retval=write(client_fd, &dirRotation, sizeof(dirRotation)) == -1) ){
      perror("xfer2as");
      fprintf(stderr, "%s: Error in writing to server@%s.\n",
	      "xfer2as", serverID);
      exit(4);
    }

    // xmit number of rotations
    //
    // Convert the  number of rotations, an integer,  to four individual
    // bytes.  The arduinoStepperServer.ino can  only accept one byte at
    // a time.
    //
    // This is done  using the most portable method,  were there is no
    // need  to  worry about  size(number  of  bytes)  of an  int  and
    // endianness, BTW arduino is little endian(MSB<-LSB).
    //
    numRotationByte[0] = ((numRotation & 0x000000FF) >> 0);
    numRotationByte[1] = ((numRotation & 0x0000FF00) >> 8);
    numRotationByte[2] = ((numRotation & 0x00FF0000) >> 16);
    numRotationByte[3] = ((numRotation & 0xFF000000) >> 24);

    fprintf(stderr,
	    "dbg: xfer2as: numRotationByte MSB->LSB: 0x%x 0x%x 0x%x 0x%x\n",
	    numRotationByte[3], numRotationByte[2],
	    numRotationByte[1], numRotationByte[0]);

    // Send the four bytes to the server, arduinoStepperServer.  With
    // the least significant byte first.
    for (i=0; i<4; i++){
      if ( (retval=write(client_fd, &numRotationByte[i],
			 sizeof(numRotationByte[0])) == -1) ){
	perror("xfer2as");
	fprintf(stderr, "%s: Error in writing to server@%s.\n",
		"xfer2as", serverID);
	exit(4);
      }
    }
  }
  else{
    fprintf(stderr, "numRotations has an invalid value, ie a -ve value.\n");
    exit(7);
  }
  
  
  if (close(client_fd) == -1){
    perror("xfer2as");
    fprintf(stderr, "%s: Error in closing socket fd\n.", "xfer2as");
    exit(6);
  }
  
  
  return (0); /* all ok, no values need to be returned to the client */
  
} /* end: main() */	   
