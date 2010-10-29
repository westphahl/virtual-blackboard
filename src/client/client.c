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

#include <gtk/gtk.h>


/* User */
#include "../commons.h"
#include "gui.h"
#include "client.h"
#include "shared.h"
#include "listener_thread.h"
#include "utils.h"


/* globale Variable */
static char toggleRequestWrite = 1;



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
int updateGUIstate(void)
{
	static char tmp[256];
	GdkColor tmpColor;

/*	gdk_color_parse ("#385030", &tmpColor); */
	gdk_color_parse ("#386038", &tmpColor); 
	gtk_widget_modify_base(GTK_WIDGET(textview), GTK_STATE_NORMAL, &tmpColor); /* Tafelhintergrundfarbe setzen */

	/* Statusleiste */
	gtk_label_set_text(statusWrite, "Tafelrechte\n[lesen/schreiben]");

	sprintf(tmp, "Name: \t[%s]\nRolle: \t[%s]\nClientID:\t[%i]",
			"Test", 1?"Dozent":(0?"Tutor":"Student"), 1);
	gtk_label_set_text(statusRole, tmp);

	sprintf(tmp, "Zeit der letzten Änderung\n[%i]", 12345);
	gtk_label_set_text(statusVersion, tmp);

	sprintf(tmp, "Dozenten: \t[%i]\nTutoren:    \t[%i]\nStudenten: \t[%i]", 1, 2, 3);
	gtk_label_set_text(statusUsers, tmp);

	/* Buttons */
	sprintf(tmp,"Schreibrecht %s", toggleRequestWrite?"anfragen":"abgeben" );
	gtk_button_set_label(GTK_BUTTON(requestWrite),tmp);

	return 0;
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
    popupMessage("Schreibrecht angefragt");
	toggleRequestWrite = toggleRequestWrite ? 0 : 1;
	updateGUIstate();
}


 /**************************************************************************************\
 * Funktion:                    on_button_quit_all_clicked                              *
 \**************************************************************************************/
void on_button_quit_all_clicked (GtkButton * button, gpointer user_data)
{
	if(popupQuestionDialog("Sind Sie sicher?","Wollen Sie das Programm wirklich beenden?"))
		gtk_main_quit();
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
    popupMessage("Tafel löschen");
}


 /**************************************************************************************\
 * Funktion:          on_button_change_mode_clicked                                     *
 \**************************************************************************************/
void on_button_remove_permissions_clicked ( GtkButton * button, gpointer user_data )
{
    popupMessage("Schreibrecht entziehen");
}


 /**************************************************************************************\
 * Funktion:           on_text_buffer_changed                                           *
 \**************************************************************************************/
void on_text_buffer_changed (GtkTextBuffer *textbuffer, gpointer user_data)
{
	popupMessage("Text verändert");
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

    /* pthread */
    pthread_t listener_tid;

    /* parse command-line options */
    while ((opt = getopt(argc, argv, "hs:p:")) != -1) {
        switch (opt) {
            case 's':
                server_addr = optarg;
                fprintf(stdout, " %s", server_addr);
                fflush(stdout);
                break;
            case 'p':
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
		int sock;
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

            // TODO Erzeuge Semaphore für Tafel
        
            board_sem_id = create_semaphore(BLACKBOARD_SEM_KEY);

            board_sem.sem_num = 0;
            board_sem.sem_flg = SEM_UNDO;

            semaphore_up(board_sem_id, board_sem);
			
            // TODO Starte Listener-Thread
            if((pthread_create(&listener_tid, NULL, listener_handler, NULL)) != 0) {
                perror("pthread_create");
                return(EXIT_FAILURE);
            }

            semaphore_down(board_sem_id, board_sem);
            
            getchar();

            semaphore_up(board_sem_id, board_sem);

            // TODO Starte Command-Thread
    
            // TODO Starte Live-Agent

            // Starte GUI
	        fprintf(stdout, "Start GUI\n");

	        /* GTK threading aktivieren */
	        g_thread_init(NULL);
	        gdk_threads_init();

	        /* GUI einrichten */
	        gtk_init (&argc, &argv);
	        create_window ();
	        gtk_widget_show_all (GTK_WIDGET(window));

	        gdk_threads_enter();
	        updateGUIstate();

	        /* GTK Hauptprogramm ausführen */
	        gtk_main();
	        gdk_threads_leave();

            // TODO Starte Trigger für Tafeländerung
	
	        // TODO Aufräumen!
            delete_semaphore(board_sem_id);
            
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

