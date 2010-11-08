#define _POSIX_SOURCE 1

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
#include "client.h"
#include "gui.h"
#include "command_thread.h"
#include "listener_thread.h"
#include "liveagent_thread.h"
#include "utils.h"
#include "shared.h"
#include "../commons.h"
#include "../net_message.h"

// Global variables
static struct client_data cdata;
static int sock;
static char *blackboard;

/* 
 * Update gui labels with role, name, client-id, ...
 */
int updateGUIstate() {
	cdata_lock();
	
	// Temporary char buffer
	static char tmp[256];
	
	// Set blackboard background color
	GdkColor tmpColor;
	gdk_color_parse ("#386038", &tmpColor); 
	gtk_widget_modify_base(GTK_WIDGET(textview), GTK_STATE_NORMAL, &tmpColor);

	// Update user-data labels
	sprintf(tmp,"Tafelrechte\n%s", cdata.write ? "[schreiben]" : "[lesen]");
	gtk_label_set_text(statusWrite, tmp);

	sprintf(tmp, "Name: \t[%s]\nRolle: \t[%s]\nCID:\t[%i]",
			cdata.name, cdata.role==2 ? "Dozent" : (cdata.write ? "Tutor" : "Student"), cdata.cid);
	gtk_label_set_text(statusRole, tmp);

	sprintf(tmp, "Zeit der letzten Änderung\n[%li]", time(NULL));
	gtk_label_set_text(statusVersion, tmp);

	sprintf(tmp, "Dozenten: \t[%i]\nTutoren: \t[%i]\nStudenten: \t[%i]", cdata.dozenten, cdata.tutoren, cdata.studenten);
	gtk_label_set_text(statusUsers, tmp);

	// Update button text
	sprintf(tmp,"Schreibrecht %s", cdata.write ? "abgeben" : "anfragen");
	gtk_button_set_label(GTK_BUTTON(requestWrite),tmp);

	// Disable illegal buttons
	gtk_widget_set_sensitive(GTK_WIDGET(requestWrite), cdata.role==2 ? 0 : 1);
	gtk_widget_set_sensitive(GTK_WIDGET(removePermissions), cdata.role==2 ? 1 : 0);
	gtk_widget_set_sensitive(GTK_WIDGET(logBlackboard), cdata.write ? 1 : 0);
	gtk_widget_set_sensitive(GTK_WIDGET(quitAll), cdata.role==2 ? 1 : 0);
	
	// Enable/Disable textview
	gtk_text_view_set_editable(textview, cdata.write);
	
	cdata_unlock();

	return 0;
}

/*
 * Update board content.
 */
void updateBoard(char* text) {
	// Temporary char buffer
	static char tmp[256];
	
	// Update label text
	sprintf(tmp, "Zeit der letzten Änderung\n[%li]", time(NULL));
	gtk_label_set_text(statusVersion, tmp);

	// Copy buffer-text to blackboard
	GtkTextBuffer *gtkbuf = gtk_text_view_get_buffer(textview);
	gtk_text_buffer_set_text(gtkbuf, text, strlen(text));
}

/* 
 * Event: quit client.
 */
gint on_application_exit(GtkWidget * widget, GdkEvent event, gpointer daten) {
	// Call popup question
	if(popupQuestionDialog("Sind Sie sicher?","Wollen Sie das Programm wirklich beenden?")) {
		// Quit programm
		gtk_main_quit();
		// Trigger event 'destroy'
		return FALSE;
	}
	return TRUE;
}

/*
 * Event: request write permission.
 */
void on_button_request_write_clicked(GtkButton * button, gpointer user_data) {
	// Trigger request for write permisson
	trigger_command(m_request);
}

/*
 * Event: shutdown system.
 */
void on_button_quit_all_clicked(GtkButton * button, gpointer user_data) {
	// Call popup question
	if(popupQuestionDialog("Sind Sie sicher?","Wollen Sie das Programm und den Server wirklich beenden?")) {
		// Trigger shutdown
		trigger_command(m_shutdown);
		// Quit programm
		gtk_main_quit();
	}
}

/*
 * Event: quit client
 */
void on_button_quit_clicked(GtkButton * button, gpointer user_data) {
	// Call popup question
	if(popupQuestionDialog("Sind Sie sicher?","Wollen Sie das Programm wirklich beenden?"))
		// Quit programm
		gtk_main_quit();
}

/*
 * Event: clear board.
 */
void on_button_log_clicked(GtkButton * button, gpointer user_data) {
	// Trigger clean board
	trigger_command(m_clear);
}

/*
 * Event: remove permission.
 */
void on_button_remove_permissions_clicked(GtkButton * button, gpointer user_data) {
	// Trigger request
	trigger_command(m_request);
}

/*
 * Event: text buffer changed.
 */
void on_text_buffer_changed(GtkTextBuffer *textbuffer, gpointer user_data) {
	// Check if user has write permissions.
	if(cdata.write == 1) {
		char *buffer;
		
		// Get gtk textbuffer range
		GtkTextIter startIter, endIter;
		gtk_text_buffer_get_start_iter(textbuffer, &startIter);
		gtk_text_buffer_get_end_iter(textbuffer, &endIter);

		// Copy gtk textbuffer in temporary buffer
		buffer = gtk_text_buffer_get_text(textbuffer, &startIter, &endIter, FALSE);
		
		// Assign buffer to blackboard
		board_lock();
		blackboard = buffer;
		board_unlock();

		// Update timestamp
		static char tmp[256];
		sprintf(tmp, "Zeit der letzten Änderung\n[%li]", time(NULL));
		gtk_label_set_text(statusVersion, tmp);
		
		// Trigger live-agent
		static int counter = 0;
		static struct itimerval itimer;

		// Setup timer
		itimer.it_interval.tv_sec= 250/1000;
		itimer.it_interval.tv_usec= 0;
		itimer.it_value.tv_sec= 250/1000;
		itimer.it_value.tv_usec= 250*1000;
		
		// Check timer state
		if(counter < 6) {
			// Start/restart timeruninitialized
			setitimer(ITIMER_REAL, &itimer, NULL);
			counter++; // Raise counter
		} else {
			// Start alternative trigger
			trigger_liveagent();
			counter = 0;	
		}
	}
}

/*
 * Return the blackboard.
 */
char *get_blackboard() {
	return blackboard;
}

/*
 * Client for virtual blackboard.
 *
 * Takes four arguments:
 *      -s <ADRESS> Set server-ip or server-name
 *      -p <PORT>   Set user-defined port
 *      -u <NAME>   Set username
 *      -r <ROLE>   Set user-role
 */
int main(int argc, char **argv) {
	if(argc < 1) {
		printf("Must at least specify a server.\n\n");
		printf("Usage: client [OPTIONS] ...\n");
		printf("   -s  --server    Server ip or name (default localhost)\n");
		printf("   -p  --port      Specify a port (default 50000)\n");
		printf("   -n  --name      Name (default Guest)\n");
		printf("   -1  --student   Login as student\n");
		printf("   -2  --docent    Login as docent\n\n");
		printf("   -h  --help      Show this help message\n");
		return 0; // Close program
	}
    // Socket variables
	char *listen_port = DEFAULT_PORT;
    char *server_addr = NULL;
    struct addrinfo *addr_info;
    struct addrinfo *p; 
    struct addrinfo hints;
    int ret;
    
    // Parser options
    int opt;
    const char* short_options = "s:p:n:12h";
	struct option long_options[] = {
		{"server", 1, NULL, 's'},
		{"port", 1, NULL, 'p'},
		{"name", 1, NULL, 'n'},
		{"student", 0, NULL, '1'},
		{"docent", 0, NULL, '2'},
		{"help", 0, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	int long_index = 0;

    // Variables for listener-thread
    pthread_t listener_tid;
	struct listenert_data lt_data;
	
	// Variables for command-thread
	pthread_t command_tid;
	struct commandt_data cm_data;
	
	// Variables for live-agent
	pthread_t liveagent_tid;
	struct liveagentt_data la_data;

	// Initialize cdata with 0
	cdata.cid = 0;
	cdata.role = 0;
	cdata.write = 0;
	cdata.dozenten = 0;
	cdata.tutoren = 0;
	cdata.studenten = 0;
	cdata.name = "Guest";

    // Parser for command-line options
    while ((opt = getopt_long(argc, argv, short_options, long_options, &long_index)) != -1) {
        switch (opt) {
        	case 's':
        		// Set server-address
        		printf("  --server %s\n", optarg);
	            server_addr = optarg;
	            break;
        	case 'p': 
        		// Set server-port
        		printf("  --port %s\n", optarg);
            	listen_port = optarg;
            	if((strtol(listen_port, NULL, 10) < PORT_RANGE_MIN) || 
                	    (strtol(listen_port, NULL, 10) > PORT_RANGE_MAX)) {
               		printf("Invalid port range: must be between %i and %i.\n" \
                	    "Falling back to default (%s)\n", 
                	    PORT_RANGE_MIN, PORT_RANGE_MAX, DEFAULT_PORT);
                	listen_port = DEFAULT_PORT;
            	}
            	break;
			case 'n':
				// Set username
				printf("  --name %s\n", optarg);
				cdata.name = optarg;
				break;
			case '1':
				// Set user-role student
				printf("  --role 1\n");
				cdata.role = 1;
				break;
			case '2':
				// Set user-role docent
				printf("  --role 2\n");
				cdata.role = 2;
				break;
			case 'h':
				// Show help message
				printf("Usage: client [OPTIONS] ...\n");
				printf("   -s  --server    Server ip or name (default localhost)\n");
				printf("   -p  --port      Specify a port (default 50000)\n");
				printf("   -n  --name      Name (default Guest)\n");
				printf("   -1  --student   Login as student\n");
				printf("   -2  --docent    Login as docent\n\n");
				printf("   -h  --help      Show this help message\n");
				return 0;
				break;
        }
    }

    // clear addr-struct
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    // solving server address
    ret = getaddrinfo(server_addr, listen_port, &hints, &addr_info);
    if(ret) {
		printf("getaddrinfo: %s\n", gai_strerror(ret));
		exit(EXIT_FAILURE);
	}
	
	// Save addresses in p
    p = addr_info;
    
    while (p) {
		char dst[INET6_ADDRSTRLEN]; 

		// Create socket for found family
        sock = socket(p->ai_family, p->ai_socktype, 0);

		// getnameinfo
		getnameinfo(p->ai_addr,
			p->ai_addrlen,
			dst,
			sizeof(dst),
			NULL,
			0,
			NI_NUMERICHOST);

		printf("Trying %s:%s ... ", dst, listen_port);
		fflush(stdout);

		// Try to connect
        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0) {
			printf("Connected\n");
			fflush(stdout);

			// Activate gtk threading
	        g_thread_init(NULL);
	        gdk_threads_init();            

	        // Setup gui 
	        gtk_init (&argc, &argv);
	        create_window ();
	        gtk_widget_show_all(GTK_WIDGET(window));
			
            // Start listener-thread
			lt_data.socket = sock;
			lt_data.cdata = &cdata;
            if(pthread_create(&listener_tid, NULL, listener_handler, (void *) &lt_data) != 0) {
            	perror("pthread_create");
            	return(EXIT_FAILURE);
            }

            // Start command-thread
            cm_data.socket = sock;
            cm_data.cdata = &cdata;
            if(pthread_create(&command_tid, NULL, command_handler, (void *) &cm_data) != 0) {
            	perror("pthread_create");
            	return(EXIT_FAILURE);
            }

            // start live-agent
            la_data.socket = sock;
            if(pthread_create(&liveagent_tid, NULL, liveagent_handler, (void *) &la_data) != 0) {
            	perror("pthread_create");
            	return(EXIT_FAILURE);
            }
            
            sleep(1);
            
            // Login to server
			//send_login(sock, cdata.role, cdata.name);
			trigger_command(m_login);
            
			// Start gui
	        gdk_threads_enter();
	        gtk_main();
	        gdk_threads_leave();

	        // TODO Clean up
			board_destroy(); // Destroy board mutex
			cdata_destroy(); // Destroy cdata mutex

            close(sock); // Close connection
			break;
        } else {
			perror("FAILED");
		}
		close(sock);

		// Get next addressinfo
		p = p->ai_next;
    }

	// Wait for threads ???

    // Delete addressinfo
    freeaddrinfo(addr_info);

	// Close main programm
	return 0;
}
