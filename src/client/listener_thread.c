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

#include <gtk/gtk.h>

#include "client.h"
#include "listener_thread.h"
#include "command_thread.h"
#include "shared.h"
#include "utils.h"
#include "gui.h"
#include "../commons.h"
#include "../net_message.h"

void *listener_handler(void *data) {
	struct listenert_data *lt_data = (struct listenert_data *)data;
	
    printf("Listener-Thread gestartet\n");
	fflush(stdout);

	int ret = 0;
    int sock = lt_data->socket;
	struct client_data *cdata = lt_data->cdata;
	struct net_header *hdr = alloc(sizeof(struct net_header));

    while (1) {
        ret = read(sock, hdr, sizeof(*hdr));
        
        printf("Typ: %i\n", hdr->type);
		printf("Länge: %i\n", ntohs(hdr->length));
		fflush(stdout);
		
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
			switch(hdr->type) {
			case m_status: {
					struct net_status *msg = alloc(sizeof(struct net_status));
					
					read(sock, &msg->role, ntohs(hdr->length));

					printf("Rolle: %i\n", msg->role);
					cdata->role = msg->role;

					printf("Client-ID: %i\n", ntohs(msg->cid));
					cdata->cid = ntohs(msg->cid);

					printf("Write-Per: %i\n", msg->write);
					cdata->write = msg->write;

					printf("Dozenten: %i\n", msg->dcount);
					cdata->dozenten = msg->dcount;

					printf("Tutoren: %i\n", msg->tcount);
					cdata->tutoren = msg->tcount;

					printf("Studenten: %i\n", ntohs(msg->scount));
					cdata->studenten = ntohs(msg->scount);
					
					fflush(stdout);

					gdk_threads_enter();
					updateGUIstate();	
					gdk_threads_leave();
					
					free(msg);
					break;
				}
			case m_error: {
					struct net_error *msg = alloc(sizeof(struct net_error)+ntohs(hdr->length)+1);
					
					read(sock, &msg->ecode, hdr->length);

					printf("Code: %i\n", msg->ecode);
					printf("Detail: %s\n", msg->detail);
					fflush(stdout);

					static char tmp[256];
					sprintf(tmp, "Fehler %i: %s", msg->ecode, msg->detail);
					
					gdk_threads_enter();
					popupMessage(tmp);
					gdk_threads_leave();

					free(msg);
					break;
				}
			case m_board: {
					struct net_board *msg = alloc(sizeof(struct net_board)+ntohs(hdr->length)+1);

					read(sock, msg->content, ntohs(hdr->length));

					printf("Inhalt: %s\n", msg->content);
					fflush(stdout);

					gdk_threads_enter();
					updateBoard(msg->content);
					gdk_threads_leave();

					free(msg);
					break;
				}
			case m_query: {
					struct net_query *msg = alloc(sizeof(struct net_query)+ntohs(hdr->length)+1);
					struct net_reply *rdata = alloc(sizeof(struct net_reply));
					
					read(sock, &msg->cid, ntohs(hdr->length));

					printf("Client-ID: %i\n", ntohs(msg->cid));
					printf("Name: %s\n", msg->name);
					fflush(stdout);

					gdk_threads_enter();
					int a = popupQuestionDialog("Sind Sie sicher?","Wollen Sie dem Benutzer wirklich Schreibrechte geben?");
					rdata->write = a;
					rdata->cid = msg->cid;
					trigger_command(m_reply, rdata);
					gdk_threads_leave();

					free(msg);
					break;
				}
			default: {
					printf("Unbekannter Datentyp!\n");
					fflush(stdout);
					break;
				}
			}
            perror("read");
        }
    }
    
    free(hdr);

    pthread_exit(0);
    return NULL;
}
