// System headers
#include <pthread.h>
#include <stdio.h>
#include <sys/signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <limits.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

// GTK header
#include <gtk/gtk.h>

// User headers
#include "listener_thread.h"
#include "client.h"
#include "gui.h"
#include "command_thread.h"
#include "utils.h"
#include "shared.h"
#include "../commons.h"
#include "../net_message.h"

/*
 * Listener-thread receives messages from the server.
 */
void *listener_handler(void *data) {
	struct listenert_data *lt_data = (struct listenert_data *)data;
	struct client_data *cdata = lt_data->cdata;
	int socket = lt_data->socket;
	int ret = 0;
	
    printf("Listener-Thread gestartet\n");
	fflush(stdout);

	// Allocate disk space for message-head    
	struct net_header *hdr = alloc(sizeof(struct net_header));

    while(1) {
    	// Read message-head
        ret = read(socket, hdr, sizeof(*hdr));
        
        /*
        printf("Typ: %i\n", hdr->type);
		printf("Länge: %i\n", ntohs(hdr->length));
		fflush(stdout);
		*/
		
		// Check if connection is closed
        if (ret == 0) {
			gdk_threads_enter();
			popupMessage("Verbindung wurde getrennt, Client wird beendet!");
			gtk_main_quit();
			gdk_threads_leave();
            break;
        }
        if (ret < 0) {
            perror("read");
            break;
        }
        if (ret > 0) {
        	// Switch to different message-types
			switch(hdr->type) {
				// Receive status update
				case m_status: {
					// Allocate disk space for status message
					struct net_status *msg = alloc(sizeof(struct net_status));
					
					// Read socket and save data
					read(socket, &msg->role, ntohs(hdr->length));

					printf("Rolle: %i\n", msg->role);
					printf("Client-ID: %i\n", ntohs(msg->cid));
					printf("Write-Per: %i\n", msg->write);
					printf("Dozenten: %i\n", msg->dcount);
					printf("Tutoren: %i\n", msg->tcount);
					printf("Studenten: %i\n", ntohs(msg->scount));
					fflush(stdout);
					
					// Write result in client-data
					cdata->role = msg->role;
					cdata->cid = ntohs(msg->cid);
					cdata->write = msg->write;
					cdata->dozenten = msg->dcount;
					cdata->tutoren = msg->tcount;
					cdata->studenten = ntohs(msg->scount);

					// Send command to gui thread
					gdk_threads_enter();
					updateGUIstate();	
					gdk_threads_leave();
					
					// Release allocated disk space
					free(msg);
					break;
				}
				// Receive error message
				case m_error: {
					// Allocate disk space for error message
					struct net_error *msg = alloc(sizeof(struct net_error)+ntohs(hdr->length)+1);
					
					// Read socket and save data
					read(socket, &msg->ecode, hdr->length);

					printf("Code: %i\n", msg->ecode);
					printf("Detail: %s\n", msg->detail);
					fflush(stdout);

					// Create string with received data
					static char tmp[256];
					sprintf(tmp, "Fehler %i: %s", msg->ecode, msg->detail);
					
					// Call popup in gui-thread
					gdk_threads_enter();
					popupMessage(tmp);
					gdk_threads_leave();

					// Release allocated disk space
					free(msg);
					break;
				}
				// Receive blackboard
				case m_board: {
					// Allocate disk space for blackboard
					struct net_board *msg = alloc(sizeof(struct net_board)+ntohs(hdr->length)+1);

					// Read socket and save data
					read(socket, msg->content, ntohs(hdr->length));

					printf("Inhalt: %s\n", msg->content);
					fflush(stdout);

					// Send command to gui-thread
					gdk_threads_enter();
					updateBoard(msg->content);
					gdk_threads_leave();

					// Release allocated disk space
					free(msg);
					break;
				}
				// Receive query for write permisson
				case m_query: {
					// Allocated disk space for query and return data
					struct net_query *msg = alloc(sizeof(struct net_query)+ntohs(hdr->length)+1);
					struct net_reply *rdata = alloc(sizeof(struct net_reply));
					
					// Read socket and save data
					read(socket, &msg->cid, ntohs(hdr->length));

					printf("Client-ID: %i\n", ntohs(msg->cid));
					printf("Name: %s\n", msg->name);
					fflush(stdout);
	
					// Send reply to server
					gdk_threads_enter();
					int a = popupQuestionDialog("Sind Sie sicher?","Wollen Sie dem Benutzer wirklich Schreibrechte geben?");
					rdata->write = a;
					rdata->cid = ntohs(msg->cid);
					trigger_command(m_reply, rdata);
					gdk_threads_leave();

					// Release allocated disk space
					free(rdata);
					free(msg);
					break;
				}
				// Receive unknown datatyp
				default: {
					printf("Unbekannter Datentyp!\n");
					fflush(stdout);
					break;
				}
			}
            perror("read");
        }
    }
    
    // Release allocated disk space
    free(hdr);

    pthread_exit(0);
    return NULL;
}
