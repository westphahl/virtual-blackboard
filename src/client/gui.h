#ifndef GUI_H_
#define GUI_H_

 /****** Dipl.-Inform.(FH) Elias Drotleff / HS Ravensburg-Weingarten *******\
 *                                                                          *
 * Projekt:                             Blackboard                          *
 *                                                                          *
 * Modulname:                   gui                                         *
 *                                                                          *
 * Modultyp:                    Funktionsbibliotek / Globale Daten          *
 *                                                                          *
 * Sprache:                     C                                           *
 *                                                                          *
 * Kurzbeschreibung:    GTK+ Gui-Funktionen, exportiert Globale Variablen   *
 *                                                                          *
 * Ersteller/Fach:      Elias Drotleff / Projektarbeit Sysop                *
 * Erstellungsdatum:    15.05.2007                                          *
 * letzte Aenderung:    11.03.2010                                          *
 *                                                                          *
 \**************************************************************************/



/* globale Variablen */
extern GtkWindow *window;
extern GtkTextView *textview;
extern GtkButton *requestWrite;
extern GtkButton *logBlackboard;
extern GtkButton *removePermissions;
extern GtkButton *quitClient;
extern GtkButton *quitAll;
extern GtkLabel *statusMode;
extern GtkLabel *statusRole;
extern GtkLabel *statusWrite;
extern GtkLabel *statusVersion;
extern GtkLabel *statusUsers;



/* Funktionsprototypen */
void create_window (void);
int popupQuestionDialog (const char *title, const char *question);
void popupMessage (const char *message);

gint on_application_exit (GtkWidget * widget, GdkEvent event, gpointer daten);
void on_button_request_write_clicked ( GtkButton * button, gpointer user_data );
void on_button_quit_all_clicked ( GtkButton * button, gpointer user_data );
void on_button_quit_clicked ( GtkButton * button, gpointer user_data );

void on_button_log_clicked ( GtkButton * button, gpointer user_data );
void on_button_remove_permissions_clicked ( GtkButton * button, gpointer user_data );

void on_text_buffer_changed (GtkTextBuffer *textbuffer, gpointer user_data);


#endif /*GUI_H_*/
