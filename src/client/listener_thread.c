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
#include "command_handler.h"
#include "shared.h"
#include "utils.h"
#include "gui.h"
#include "../commons.h"

void *listener_handler(void *data) {
	struct listenert_data *lt_data = (struct listenert_data *)data;
	
    printf("Listener-Thread gestartet\n");
	fflush(stdout);

	int ret = 0;
    int sock = lt_data->socket;
	struct CLIENT_DATA *cdata = lt_data->cdata;
	struct NET_HEADER *hdr = alloc(sizeof(struct NET_HEADER));

    while (1) {
        ret = read(sock, hdr, sizeof(*hdr));
        
        printf("Typ: %i\n", hdr->type);
		printf("Länge: %i\n", ntohs(hdr->lenght));
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
			case NET_TYPE_STATUS_MSG: {
					struct NET_MESSAGE_STATUS *msg = alloc(sizeof(struct NET_MESSAGE_STATUS));
					
					read(sock, &msg->role, ntohs(hdr->lenght));

					printf("Rolle: %i\n", msg->role);
					cdata->role = msg->role;

					printf("Client-ID: %i\n", ntohs(msg->client_id));
					cdata->client_id = ntohs(msg->client_id);

					printf("Write-Per: %i\n", msg->write_per);
					cdata->write_per = msg->write_per;

					printf("Dozenten: %i\n", msg->dozenten);
					cdata->dozenten = msg->dozenten;

					printf("Tutoren: %i\n", msg->tutoren);
					cdata->tutoren = msg->tutoren;

					printf("Studenten: %i\n", ntohs(msg->studenten));
					cdata->studenten = ntohs(msg->studenten);
					
					fflush(stdout);

					gdk_threads_enter();
					updateGUIstate();	
					gdk_threads_leave();
					
					free(msg);
					break;
				}
			case NET_TYPE_ERROR_MSG: {
					struct NET_MESSAGE_ERROR *msg = alloc(sizeof(struct NET_MESSAGE_ERROR));
					
					read(sock, &msg->error_code, sizeof(uint8_t));

					msg->detail = alloc(ntohs(hdr->lenght)-sizeof(uint8_t)+1);
		
					read(sock, msg->detail, ntohs(hdr->lenght)-sizeof(uint8_t));

					printf("Code: %i\n", msg->error_code);
					printf("Detail: %s\n", msg->detail);
					fflush(stdout);

					static char tmp[256];
					sprintf(tmp, "Fehler %i: %s", msg->error_code, msg->detail);
					
					gdk_threads_enter();
					popupMessage(tmp);
					gdk_threads_leave();

					free(msg->detail);
					free(msg);
					break;
				}
			case NET_TYPE_BOARD_CONTENT: {
					struct NET_MESSAGE_BOARD_CONTENT *msg = alloc(sizeof(struct NET_MESSAGE_BOARD_CONTENT));
					
					msg->board_content = alloc(ntohs(hdr->lenght)+1);	
		
					read(sock, msg->board_content, ntohs(hdr->lenght));

					printf("Inhalt: %s\n", msg->board_content);
					fflush(stdout);

					gdk_threads_enter();
					updateBoard(msg->board_content);
					gdk_threads_leave();

					free(msg->board_content);
					free(msg);
					break;
				}
			case NET_TYPE_REQUESTED: {
					struct NET_MESSAGE_REQUESTED *msg = alloc(sizeof(struct NET_MESSAGE_REQUESTED));
					
					read(sock, &msg->client_id, sizeof(uint16_t));

					msg->name = alloc(ntohs(hdr->lenght)-sizeof(uint16_t)+1);
					
					read(sock, msg->name, ntohs(hdr->lenght)-sizeof(uint16_t));

					printf("Client-ID: %i\n", ntohs(msg->client_id));
					printf("Name: %s\n", msg->name);
					fflush(stdout);

					gdk_threads_enter();
					send_reply(sock, popupQuestionDialog("Sind Sie sicher?","Wollen Sie dem Benutzer wirklich Schreibrechte geben?"), htons(msg->client_id));
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
