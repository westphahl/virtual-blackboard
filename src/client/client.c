#define _POSIX_SOURCE 1

/* System */
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

/* User */
#include "../commons.h"
#include "../net_message.h"
#include "gui.h"
#include "client.h"
#include "shared.h"
#include "listener_thread.h"
#include "liveagent_thread.h"
#include "command_thread.h"
#include "utils.h"

/* globale Variable */
struct client_data cdata;
int sock;
static char *blackboard;

 /**************************************************************************************\
 * Funktion:            updateGUIstate                                                  *
 *                                                                                      *
 * Kurzbeschreibung:  	Helperfunktion                                                  *
 *                      Aktualisiert die Widgets der Gui in Abhängigkeit der globalen	*
 *                      Variablen.                                                      *
 *                                                                                      *
 * Ergebnis:            int: Erfolgsstatus                                              *
 *                                                                                      *
 * Import-Parameter:    myRole, myName, myClientID                                      *
 *                      currentBoardMode                                                *
 *                      haveWritePermission                                             *
 *                      currentBoardTimestamp                                           *
 *                                                                                      *
 * Export-Parameter:    -                                                               *
 *                                                                                      *
 \**************************************************************************************/
int updateGUIstate()
{
	static char tmp[256];
	GdkColor tmpColor;

	//gdk_color_parse ("#385030", &tmpColor);
	gdk_color_parse ("#386038", &tmpColor); 
	gtk_widget_modify_base(GTK_WIDGET(textview), GTK_STATE_NORMAL, &tmpColor);

	/* Statusleiste */
	sprintf(tmp,"Tafelrechte\n%s", cdata.write ? "[lesen/schreiben]" : "[lesen]");
	gtk_label_set_text(statusWrite, tmp);

	sprintf(tmp, "Name: \t[%s]\nRolle: \t[%s]\nCID:\t[%i]",
			cdata.name, cdata.role ? "Dozent" : (cdata.write ? "Tutor" : "Student"), cdata.cid);
	gtk_label_set_text(statusRole, tmp);

	sprintf(tmp, "Zeit der letzten Änderung\n[%li]", time(NULL));
	gtk_label_set_text(statusVersion, tmp);

	sprintf(tmp, "Dozenten: \t[%i]\nTutoren: \t[%i]\nStudenten: \t[%i]", cdata.dozenten, cdata.tutoren, cdata.studenten);
	gtk_label_set_text(statusUsers, tmp);

	/* Buttons */
	sprintf(tmp,"Schreibrecht %s", cdata.write ? "abgeben" : "anfragen");
	gtk_button_set_label(GTK_BUTTON(requestWrite),tmp);

	/* Disable illegal buttons */
	gtk_widget_set_sensitive(GTK_WIDGET(requestWrite), cdata.role==2 ? 0 : 1);
	gtk_widget_set_sensitive(GTK_WIDGET(removePermissions), cdata.role==2 ? 1 : 0);
	gtk_widget_set_sensitive(GTK_WIDGET(logBlackboard), cdata.role==2 ? 1 : 0);
	gtk_widget_set_sensitive(GTK_WIDGET(quitAll), cdata.role==2 ? 1 : 0);
	
	/* Textview */
	gtk_text_view_set_editable(textview, cdata.write);

	return 0;
}

void updateBoard(char* text) {
	static char tmp[256];
	sprintf(tmp, "Zeit der letzten Änderung\n[%li]", time(NULL));
	gtk_label_set_text(statusVersion, tmp);

	GtkTextBuffer *gtkbuf = gtk_text_view_get_buffer(textview);
	gtk_text_buffer_set_text(gtkbuf, text, strlen(text));
	
	pthread_mutex_unlock(&mutex_board);
	printf("Mutex TAFEL entsperrt\n");
}
 /**************************************************************************************\
 * Funktion:            on_application_exit                                             *
 *                                                                                      *
 * Kurzbeschreibung:    Callback                                                        *
 *                      Wird beim betätigen des X-Buttons(oben rechts) aufgerufen.      *
 *                      Beendet eventuell (Benutzerentscheidung) den Client             *
 *                                                                                      *
 * Ergebnis:            gint: FALSE "destroy" einleiten; TRUE nichts machen             *
 *                                                                                      *
 * Import-Parameter:    widget: Referenz auf das auslösende Widget                      *
 *                      event: 	Eventreferenz                                           *
 *                      daten:  Zusatzinformationen                                     *
 *                                                                                      *
 * Export-Parameter:    -                                                               *
 *                                                                                      *
 \**************************************************************************************/
gint on_application_exit(GtkWidget * widget, GdkEvent event, gpointer daten)
{
	if(popupQuestionDialog("Sind Sie sicher?","Wollen Sie das Programm wirklich beenden?"))
	{
		gtk_main_quit();
		return FALSE;  /* Event "destroy" einleiten */
	}
	return TRUE;
}


 /**************************************************************************************\
 * Funktion:                    on_button_request_write_clicked                         *
 \**************************************************************************************/
void on_button_request_write_clicked (GtkButton * button, gpointer user_data)
{
	trigger_command(m_request, NULL);
}


 /**************************************************************************************\
 * Funktion:                    on_button_quit_all_clicked                              *
 \**************************************************************************************/
void on_button_quit_all_clicked (GtkButton * button, gpointer user_data)
{
	if(popupQuestionDialog("Sind Sie sicher?","Wollen Sie das Programm und den Server wirklich beenden?")) {
		trigger_command(m_shutdown, NULL);
		gtk_main_quit();
	}
}


 /**************************************************************************************\
 * Funktion:                    on_button_quit_clicked                                  *
 \**************************************************************************************/
void on_button_quit_clicked ( GtkButton * button, gpointer user_data )
{
	if(popupQuestionDialog("Sind Sie sicher?","Wollen Sie das Programm wirklich beenden?"))
		gtk_main_quit();
}

 /**************************************************************************************\
 * Funktion:                    on_button_log_clicked                                   *
 \**************************************************************************************/
void on_button_log_clicked ( GtkButton * button, gpointer user_data )
{
	trigger_command(m_clear, NULL);
}


 /**************************************************************************************\
 * Funktion:          on_button_change_mode_clicked                                     *
 \**************************************************************************************/
void on_button_remove_permissions_clicked ( GtkButton * button, gpointer user_data )
{
	trigger_command(m_request, NULL);
}


 /**************************************************************************************\
 * Funktion:           on_text_buffer_changed                                           *
 \**************************************************************************************/
void on_text_buffer_changed (GtkTextBuffer *textbuffer, gpointer user_data)
{
	if(cdata.write == 1) {
		char *buffer;
		GtkTextIter startIter, endIter;
		gtk_text_buffer_get_start_iter(textbuffer, &startIter);
		gtk_text_buffer_get_end_iter(textbuffer, &endIter);

		buffer = gtk_text_buffer_get_text(textbuffer, &startIter, &endIter, FALSE);
		
		blackboard = buffer;

		static char tmp[256];
		sprintf(tmp, "Zeit der letzten Änderung\n[%li]", time(NULL));
		gtk_label_set_text(statusVersion, tmp);
		
		static int counter = 0;
		static struct itimerval itimer;

		/* Request SIGPROF */
		itimer.it_interval.tv_sec= 500/1000;
		itimer.it_interval.tv_usec= 0;
		itimer.it_value.tv_sec= 500/1000;
		itimer.it_value.tv_usec= 500*1000;
		
		if(counter < 6) {
			/* Start timer */
			setitimer(ITIMER_REAL, &itimer, NULL);
			counter++;
		} else {
			printf("Sende Tafel\n");
			counter = 0;	
		}
	}
}

 /**************************************************************************************\
 * Funktion: main                                                                       *
 \**************************************************************************************/
int main (int argc, char **argv)
{
    /* socket variables */
	char *listen_port = DEFAULT_PORT;
    char *server_addr = NULL;
    struct addrinfo *addr_info, *p, hints;
    int ret;
    int opt;

    /* pthreads */
    pthread_t listener_tid;
	struct listenert_data lt_data;
	
	pthread_t command_tid;
	struct commandt_data cm_data;
	
	pthread_t liveagent_tid;
	struct liveagentt_data la_data;

	/* fill cdata with NULL */
	cdata.cid = 0;
	cdata.role = 0;
	cdata.write = 0;
	cdata.dozenten = 0;
	cdata.tutoren = 0;
	cdata.studenten = 0;
	cdata.name = "Nobody";

    /* parse command-line options */
    while ((opt = getopt(argc, argv, "hs:p:u:r:")) != -1) {
        switch (opt) {
        case 's': {
	            server_addr = optarg;
	            fprintf(stdout, " %s", server_addr);
	            fflush(stdout);
	            break;
			}
        case 'p': {
            	listen_port = optarg;
            	if((strtol(listen_port, NULL, 10) < PORT_RANGE_MIN) || 
                	    (strtol(listen_port, NULL, 10) > PORT_RANGE_MAX)) {
               		fprintf(stderr,
                	    "Invalid port range: must be between %i and %i.\n" \
                	    "Falling back to default (%s)\n", 
                	    PORT_RANGE_MIN, PORT_RANGE_MAX, DEFAULT_PORT);
                	listen_port = DEFAULT_PORT;
            	}
            	break;
			}
		case 'u': {
				cdata.name = optarg;
				break;
			}
		case 'r': {
				cdata.role = atoi(optarg);
				break;
			}
        }
    }

    /* clear struct */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    /* getaddrinfo */
    ret = getaddrinfo(server_addr, listen_port, &hints, &addr_info);
    if(ret) {
		printf("getaddrinfo: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}

	printf("\n");
    p = addr_info;
    
    while (p) {
		char dst[INET6_ADDRSTRLEN]; 

		/* create socket for found family */		
        sock = socket(p->ai_family, p->ai_socktype, 0);

		/* getnameinfo */
		getnameinfo(p->ai_addr,
			p->ai_addrlen,
			dst,
			sizeof(dst),
			NULL,
			0,
			NI_NUMERICHOST);

		printf("Trying %s:%s ... ", dst, listen_port);
		fflush(stdout);

		/* Try to connect */
        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0) {
			fprintf(stdout, "Connected\n");

			/* GTK threading aktivieren */
	        g_thread_init(NULL);
	        gdk_threads_init();            

	        /* GUI einrichten */
	        gtk_init (&argc, &argv);
	        create_window ();
	        gtk_widget_show_all(GTK_WIDGET(window));

            // TODO Erzeuge Mutex für Tafel
			pthread_mutex_init(&mutex_board, NULL);
			pthread_mutex_lock(&mutex_board);
			printf("Mutex TAFEL gesperrt\n");
			
            // TODO Starte Listener-Thread
			lt_data.socket = sock;
			lt_data.cdata = &cdata;
            if(pthread_create(&listener_tid, NULL, listener_handler, (void *) &lt_data) != 0) {
            	perror("pthread_create");
            	return(EXIT_FAILURE);
            }

            // TODO Starte Command-Thread
            cm_data.socket = sock;
            cm_data.cdata = &cdata;
            if(pthread_create(&command_tid, NULL, command_handler, (void *) &cm_data) != 0) {
            	perror("pthread_create");
            	return(EXIT_FAILURE);
            }
            
            trigger_command(m_login, NULL);

            // TODO Starte Live-Agent
            la_data.socket = sock;
            if(pthread_create(&liveagent_tid, NULL, liveagent_handler, (void *) &la_data) != 0) {
            	perror("pthread_create");
            	return(EXIT_FAILURE);
            }

            // Starte GUI
	        fprintf(stdout, "Start GUI\n");

			/* GUI starten */
	        gdk_threads_enter();
	        //updateGUIstate();

	        /* GTK Hauptprogramm ausführen */
	        gtk_main();
	        gdk_threads_leave();

            // TODO Starte Trigger für Tafeländerung
	
	        // TODO Aufräumen!
        
			pthread_mutex_destroy(&mutex_board);

            close(sock);
			break;
        } else {
			perror("FAILED");
		}
		close(sock);

		p = p->ai_next;
    }

    /* delete addressinfo */
    freeaddrinfo(addr_info);	

	/* Hauptprogramm verlassen */
	return 0;
}

char *get_blackboard() {
	return blackboard;
}

