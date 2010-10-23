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

#include <gtk/gtk.h>


/* User */
#include "../commons.h"
#include "gui.h"
#include "client.h"


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
	toggleRequestWrite = toggleRequestWrite?0:1;
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
}


 /**************************************************************************************\
 * Funktion:          on_button_change_mode_clicked                                     *
 \**************************************************************************************/
void on_button_remove_permissions_clicked ( GtkButton * button, gpointer user_data )
{
}


 /**************************************************************************************\
 * Funktion:           on_text_buffer_changed                                           *
 \**************************************************************************************/
void on_text_buffer_changed (GtkTextBuffer *textbuffer, gpointer user_data)
{
	/* popupMessage("HALLO!!!"); */
}





 /**************************************************************************************\
 * Funktion: main                                                                       *
 \**************************************************************************************/
int main (int argc, char **argv)
{
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
	
	/* cleanup here */
	

	/* Hauptprogramm verlassen */
	return 0;
}

