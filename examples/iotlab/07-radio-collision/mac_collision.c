#include "contiki.h"
#include <stdio.h>
#include "net/rime.h"
#include "energest.h"
#include "dev/serial-line.h"

/*---------------------------------------------------------------------------*/

/* This structure holds information about energy consumption*/
struct energy_time {
  long cpu;
  long lpm;
  long transmit;
  long listen;
  long consumption_CPU;
  long consumption_LPM;
  long consumption_TX;
  long consumption_RX;
};

static struct energy_time last;
static struct energy_time diff;

/*These hold the broadcast and unicast structures*/
static struct broadcast_conn broadcast; 
static struct unicast_conn unicast;

int send = 0;
static rimeaddr_t k;

/*---------------------------------------------------------------------------*/

PROCESS(broadcast_process, "Broadcast process");
PROCESS(unicast_process, "Unicast process");

AUTOSTART_PROCESSES(&broadcast_process,&unicast_process);
/*---------------------------------------------------------------------------*/

static void start () {

 process_post(&unicast_process,PROCESS_EVENT_CONTINUE,"received_unicast");
}

static void unicast_message_resend () {

        int val = 0;

        packetbuf_copyfrom("pong",5);
        printf(" sending unicast to %d.%d\n",k.u8[0],k.u8[1]);
                   
        val = unicast_send(&unicast,&k);

        if (val!=0){
               printf(" unicast message sent \n");
	}     
        else {
                printf(" unicast message not sent \n");
	 }

}

/*---------------------------------------------------------------------------*/
/*This function is called for every incoming broadcast packet for the source node, it shows the energy values */

 void abc_recv(struct broadcast_conn *c)
{
	struct energy_time *incoming= (struct energy_time *)packetbuf_dataptr();
	printf(" Energy consumption (Time): CPU: %lu LPM: %lu TRANSMIT: %lu LISTEN: %lu \n", incoming->cpu, incoming->lpm,incoming->transmit, incoming->listen);
}

/*---------------------------------------------------------------------------*/
/*This function is called for every incoming broadcast packet */

static void  broadcast_recv(struct broadcast_conn *c,const rimeaddr_t * from)
{
 
         rimeaddr_copy(&k,from);
     
  printf("%d.%d;broadcast message received from %d.%d: '%s'\n", rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],k.u8[0], k.u8[1], (char *)packetbuf_dataptr());	


/* Asynchronous event to start the unicast process */
       
	process_post(&unicast_process,PROCESS_EVENT_CONTINUE,"send_unicast");
               
   }
static const struct broadcast_callbacks broadcast_callbacks = {broadcast_recv,abc_recv};

/*---------------------------------------------------------------------------*/
/*This function is called for every incoming unicast packet */

static void recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{

printf("%d.%d;unicast message received from %d.%d: '%s'\n", rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1],from->u8[0], from->u8[1], (char *)packetbuf_dataptr());

}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};

/*---------------------------------------------------------------------------*/

/* This function sends a broadcast message */

static void broadcast_message() {

	   int ret;

           /* Energy consumption start */                                                                 
           last.cpu = energest_type_time(ENERGEST_TYPE_CPU);
           last.lpm=energest_type_time(ENERGEST_TYPE_LPM);
           last.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
           last.listen = energest_type_time(ENERGEST_TYPE_LISTEN);
          
  	   /* Send a broadcast message */
           packetbuf_copyfrom("ping",5);
	
 	  printf(" rimeaddr_node_addr: %d.%d \n", rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
  	  ret = broadcast_send(&broadcast);

           if (ret!=0){
                printf(" broadcast message sent \n");
		}
           else {
	          printf(" broadcast message not sent!\n");
                }

           /* Energy consumption diff */
		
           diff.cpu = energest_type_time(ENERGEST_TYPE_CPU) - last.cpu;
           diff.lpm = energest_type_time(ENERGEST_TYPE_LPM) - last.lpm;
           diff.transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT) -last.transmit;
           diff.listen = energest_type_time(ENERGEST_TYPE_LISTEN) - last.listen;
	
}
/*---------------------------------------------------------------------------*/
/* This function prints the help to the users*/

static void print_usage()
{
	printf("Contiki program\n");
	printf("Type command\n");
	printf("\th:\tprint this help\n");
	printf("\tb:\tsend a broadcast message\n");
	printf("\n Type Enter to stop printing this help\n");

}


/*---------------------------------------------------------------------------*/
/*Broadcast process */

PROCESS_THREAD(broadcast_process, ev, data)  

{
  	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)
 
        PROCESS_BEGIN();

	broadcast_open(&broadcast,129, &broadcast_callbacks);

	// printf address nodes 
	printf(" rimeaddr_node_addr: %d.%d\n", rimeaddr_node_addr.u8[0],rimeaddr_node_addr.u8[1]);
	print_usage();         

        while(1){

	/*Check the character on the serial line */
        PROCESS_WAIT_EVENT_UNTIL (ev == serial_line_event_message);	

	const char *c = (char *)data; 
 
      	switch(*c){

           case 'b':
                broadcast_message();
           break;

           case '\n':
                printf("\ncmd > ");
           break;
        
 	   case 'h':
                print_usage();
           break;
	
}}
	PROCESS_END();
}


/*---------------------------------------------------------------------------*/
/* Unicast process */

PROCESS_THREAD(unicast_process,ev,data)    
{

	PROCESS_EXITHANDLER(unicast_close(&unicast);)
	
	PROCESS_BEGIN();

	unicast_open(&unicast, 146, &unicast_callbacks);

	while(1){
         
  	PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_CONTINUE);
	
	if ( send == 0){
       
        unicast_message_resend();
	start(); }
	
        if ((send == 1) && (char *)data == "send_unicast")
{
	unicast_message_resend();
	}
	send = 1;
} 
	PROCESS_END();
}
