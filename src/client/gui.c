/* System */
#include <gtk/gtk.h>

/* User */
#include "../commons.h"
#include "gui.h"



/* moduluebergreifende globale Variablen */
GtkWindow *window = NULL;
GtkTextView *textview = NULL;
GtkButton *requestWrite = NULL;
GtkButton *logBlackboard = NULL;
GtkButton *removePermissions = NULL;
GtkButton *quitClient = NULL;
GtkButton *quitAll = NULL;
GtkLabel *statusMode = NULL;
GtkLabel *statusRole = NULL;
GtkLabel *statusWrite = NULL;
GtkLabel *statusVersion = NULL;
GtkLabel *statusUsers = NULL;



 /**************************************************************************************\
 * Funktion:          on_delete_range                                                   *
 *                                                                                      *
 * Kurzbeschreibung:  Callback (Eventhandler)                                           *
 *                    Wird vor dem Löschen eines markierten Bereichs aufgerufen         *
 *                                                                                      *
 * Ergebnis:          FALSE: Bereich wird gelöscht.                                     *
 *                                                                                      *
 * Import-Parameter:  -                                                                 *
 *                                                                                      *
 * Export-Parameter:  -                                                                 *
 *                                                                                      *
 \**************************************************************************************/
gint on_delete_range(GtkTextBuffer *textbuffer, GtkTextIter *start, GtkTextIter *end, gpointer user_data)
{
	return FALSE;
}

 /**************************************************************************************\
 * Funktion:          on_insert_text                                                    *
 *                                                                                      *
 * Kurzbeschreibung:  Callback (Eventhandler)                                           *
 *                    Wird vor dem Einfügen von Text aufgerufen.                        *
 *                    Es wird sichergestellt, dass eine Zeile nicht mehr Bytes          *
 *                    als BLACKBOARD_COLUMNS und die Tafel nicht mehr Zeilen            *
 *                    als BLACKBOARD_ROWS beinhalten darf.                              *
 *                    Zeichencodierung: UTF8                                            *
 *                                                                                      *
 * Ergebnis:          FALSE: TRUE:                                                      *
 *                                                                                      *
 * Import-Parameter:  -                                                                 *
 *                                                                                      *
 * Export-Parameter:  -                                                                 *
 *                                                                                      *
 \**************************************************************************************/
gint on_insert_text( GtkTextBuffer *textbuffer, GtkTextIter *location, gchar *text, gint textBytes, gpointer user_data)
{
	static gboolean insertPreparationAlreadyDone = FALSE;

	GtkTextIter *start, *end;
	short newNlcount=0;
	int i,j;
	start = (GtkTextIter*)g_malloc(sizeof(GtkTextIter));
	end   = (GtkTextIter*)g_malloc(sizeof(GtkTextIter));

	if(!insertPreparationAlreadyDone)
	{
		/* test the bounds */
		int lineBytes 		= gtk_text_iter_get_bytes_in_line(location);
		int insertLine 		= gtk_text_iter_get_line(location);
		int insertColumn 	= gtk_text_iter_get_line_index(location);
		int lines 			= gtk_text_buffer_get_line_count(textbuffer);
		int insertCount = 0;
		if(gtk_text_buffer_get_selection_bounds(textbuffer, start, end))
		{
			gtk_text_buffer_delete(textbuffer, start, end);
			*location = *start;
			gtk_text_iter_free(start);
			gtk_text_iter_free(end);
		}



		/* Anzahl an neuen Zeilen ermitteln */
		for(i=0;i<textBytes;i++)
			if( text[i]=='\n' && lines + newNlcount  < BLACKBOARD_ROWS )
				newNlcount++;


		/* Keine neue Zeile */
		if(newNlcount==0)
		{
			j=lineBytes;

			if(insertLine+1 == lines) /* Momentane Zeile gleich letzte Zeile, ein Zeichen (Zeilenumbruch) reservieren */
				j++;

			for( ;  insertCount < textBytes ;j++)
			{

				if(j >= BLACKBOARD_COLUMNS)
				{
					/* UTF8 Korrekturen*/
					if( insertCount >= 3 && g_utf8_strlen(&text[insertCount-3],3)==0 )
					{
						insertCount-=3;
						break;
					}
					if( insertCount >= 2 && g_utf8_strlen(&text[insertCount-2],2)==0 )
					{
						insertCount-=2;
						break;
					}
					if( insertCount >= 1 && g_utf8_strlen(&text[insertCount-1],1)==0 )
					{
						insertCount-=1;
					}
					break;

				}

				if(text[insertCount] != '\n') /* im Falle newNlcount 0 aber ein '\n' im text */
					insertCount++;
				else
					break;
			}
		}
		else
		{
			for(i=0;i<=newNlcount;i++)
			{

				if(i==0)
					j=insertColumn;
				else if(i >= newNlcount)
					j=lineBytes-insertColumn-1;
				else
					j=0;

				while( 1 )
				{

					if(insertCount >= textBytes)
						goto end;

					if( j+1 >= BLACKBOARD_COLUMNS )/* >? */
					{
						if(text[insertCount]!='\n')
						{
							/* UTF8 Korrekturen*/
							if( insertCount >= 3 && g_utf8_strlen(&text[insertCount-3],3)==0 )
							{
								insertCount-=3;
								goto end;
							}
							if( insertCount >= 2 && g_utf8_strlen(&text[insertCount-2],2)==0 )
							{
								insertCount-=2;
								goto end;
							}
							if( insertCount >= 1 && g_utf8_strlen(&text[insertCount-1],1)==0 )
							{
								insertCount-=1;
							}
							goto end;
						}
						if(i+insertLine+1<=BLACKBOARD_ROWS)
							insertCount++;
						break;
					}

					/*Letzte einzufügende Zeile*/
					if(i>=newNlcount)
					{
						if(text[insertCount] != '\n')
						{
							insertCount++;
							j++;
						}
						else
						{
							goto end;
						}
					}
					else
					{
						insertCount++;
						j++;
						if(text[insertCount-1] == '\n')
						{
							break;
						}
					}
				}
			}
		}
end:
		if(insertCount > 0)
		{
			insertPreparationAlreadyDone = TRUE;
			gtk_text_buffer_insert(textbuffer,location,text,insertCount);
		}
		g_signal_stop_emission_by_name((gpointer)textbuffer, "insert-text");


	}
	else
		insertPreparationAlreadyDone = FALSE;

 	return FALSE;
}



 /**************************************************************************************\
 * Funktion:            create_window                                                   *
 *                                                                                      *
 * Kurzbeschreibung:    Initialisiert alle Widgets der GUI, ordnen die Widgets an und   *
 *                      registriert Events.                                             *
 *                                                                                      *
 * Ergebnis:            -                                                               *
 *                                                                                      *
 * Import-Parameter:    -                                                               *
 *                                                                                      *
 * Export-Parameter:    window                                                          *
 *                      textview                                                        *
 *                      buffer                                                          *
 *                      commitBlackboard, requestWrite, logBlackboard, changeMode,      *
 *                      quitClient, quitAll                                             *
 *                      statusMode, statusRole, statusWrite, statusVersion              *
 *                                                                                      *
 \**************************************************************************************/
void create_window (void) {

	GtkHBox *statusBox;
	GtkHSeparator *hSep;
	GtkVSeparator *vSep[4];
	GtkTextBuffer *buffer;

	GtkVBox *vbox_main;
	GtkHBox *hbox_ButtonsBlackboard;
	GtkVButtonBox *vbox_buttons;
	GdkColor tmpColor;
	PangoFontDescription *textviewFont;
	GtkScrolledWindow *scrolledwindow;
	int i;

	/* Fenster mit folgenden Eigenschaften anlegen */
	window = g_object_new( GTK_TYPE_WINDOW,
		"title", "Elektronische Tafel v1.3",
		"default-width",  800,
		"default-height", 450,
		"resizable", FALSE,
		"window-position", GTK_WIN_POS_CENTER,
		"border-width", 0,
		"icon", NULL,
		NULL );
	gtk_window_resize(window,900,500);
	gtk_widget_set_size_request(GTK_WIDGET(window),900,500);

	/* eine vertikale Box erzeugen */
	vbox_main = g_object_new(GTK_TYPE_VBOX, "spacing", 0, NULL);
	hbox_ButtonsBlackboard = g_object_new( GTK_TYPE_HBOX, "spacing", 0, NULL);

	/* horizontale Trennlinien initialisieren */
	hSep = (GtkHSeparator*)gtk_hseparator_new();
	for(i=0;i<4;i++)
		vSep[i] = (GtkVSeparator*)gtk_vseparator_new();

	/* Statuslabels initialisieren */
	statusBox = g_object_new(GTK_TYPE_HBOX, NULL);

	statusMode    = g_object_new( GTK_TYPE_LABEL, "label", "Systemstatus", "use-markup", FALSE, NULL);
	statusRole    = g_object_new( GTK_TYPE_LABEL, "label", "", "use-markup", FALSE, NULL);
	statusWrite   = g_object_new( GTK_TYPE_LABEL, "label", "", "use-markup", FALSE, NULL);
	statusVersion = g_object_new( GTK_TYPE_LABEL, "label", "", "use-markup", FALSE, NULL);
	statusUsers   = g_object_new( GTK_TYPE_LABEL, "label", "", "use-markup", FALSE, NULL);

	/* Fensterhintergrundfarbe setzen */
	gdk_color_parse ("#FFFFFF", &tmpColor);
	gtk_widget_modify_bg(GTK_WIDGET(window), GTK_STATE_NORMAL, &tmpColor);

	/* Statuslabels dimensionieren */
	gtk_widget_set_size_request(GTK_WIDGET(statusMode), 170, 50);
	gtk_widget_set_size_request(GTK_WIDGET(statusRole), 170, 50);
	gtk_widget_set_size_request(GTK_WIDGET(statusWrite), 170, 50);
	gtk_widget_set_size_request(GTK_WIDGET(statusVersion), 170, 50);
	gtk_widget_set_size_request(GTK_WIDGET(statusUsers), 170, 50);

	vbox_buttons = g_object_new(GTK_TYPE_VBUTTON_BOX, NULL);

	/* Tasten aufschrift festlegen  */
	requestWrite = g_object_new(GTK_TYPE_BUTTON, "label", "Schreibrecht anfragen", NULL);
	gtk_widget_set_size_request(GTK_WIDGET(requestWrite), 150, 35);

	logBlackboard = g_object_new(GTK_TYPE_BUTTON, "label", "Tafel löschen", NULL);
	gtk_widget_set_size_request(GTK_WIDGET(logBlackboard), 150, 35);

	removePermissions = g_object_new(GTK_TYPE_BUTTON, "label", "Schreibrechte entziehen", NULL);
	gtk_widget_set_size_request(GTK_WIDGET(removePermissions), 160, 35);

	quitClient = g_object_new(GTK_TYPE_BUTTON, "label", "Klienten beenden", NULL);
	gtk_widget_set_size_request(GTK_WIDGET(quitClient), 150, 35);

	quitAll = g_object_new(GTK_TYPE_BUTTON, "label", "System beenden", NULL);
	gtk_widget_set_size_request(GTK_WIDGET(quitAll), 150, 35);

	/* Rollbalken erzeugen */
	scrolledwindow = g_object_new( GTK_TYPE_SCROLLED_WINDOW,
		"hscrollbar_policy", GTK_POLICY_AUTOMATIC,
		"vscrollbar_policy", GTK_POLICY_AUTOMATIC,
		"window_placement" , GTK_CORNER_TOP_LEFT,
		NULL);
	gtk_widget_set_size_request(GTK_WIDGET(scrolledwindow), 800, 440); /* Tafeldimensionen festlegen */

	/* Eine Textview erzeugen */
	textview = g_object_new( 	GTK_TYPE_TEXT_VIEW,
								"editable", TRUE,
								"left-margin", 5,
								"right-margin", 5,
								NULL);

	gdk_color_parse ("#386038", &tmpColor);
	gtk_widget_modify_base(GTK_WIDGET(textview), GTK_STATE_NORMAL, &tmpColor); /* Tafelhintergrundfarbe setzen */


	gdk_color_parse ("#CCCCCC", &tmpColor);
	textviewFont = pango_font_description_from_string ("URW Chancery L 16");
	gtk_widget_modify_font(GTK_WIDGET(textview), textviewFont); /* Schriftart setzen */
	pango_font_description_free (textviewFont);
	gtk_widget_modify_text(GTK_WIDGET(textview), GTK_STATE_NORMAL, &tmpColor); /* Schriftfarbe */

	/* buffer der Textview zuordnen */
	buffer = g_object_new( 	GTK_TYPE_TEXT_BUFFER, NULL);
	gtk_text_view_set_buffer(textview,buffer);


	/* Eventhandler einrichten */
	g_signal_connect ( window, "delete-event", G_CALLBACK(on_application_exit), NULL );
  	g_signal_connect ( window, "destroy",G_CALLBACK(gtk_main_quit), NULL );

	g_signal_connect((gpointer)buffer, "delete-range", G_CALLBACK(on_delete_range), NULL);
	g_signal_connect((gpointer)buffer, "insert-text", G_CALLBACK(on_insert_text), NULL);
	g_signal_connect((gpointer)buffer, "changed", G_CALLBACK(on_text_buffer_changed), NULL);
	
	g_signal_connect((gpointer)requestWrite, "clicked", G_CALLBACK(on_button_request_write_clicked), NULL);
	g_signal_connect((gpointer)quitAll, "clicked", G_CALLBACK(on_button_quit_all_clicked), NULL);
	g_signal_connect((gpointer)quitClient, "clicked", G_CALLBACK(on_button_quit_clicked), NULL);
	g_signal_connect((gpointer)logBlackboard, "clicked", G_CALLBACK(on_button_log_clicked), NULL);
	g_signal_connect((gpointer)removePermissions, "clicked", G_CALLBACK(on_button_remove_permissions_clicked), NULL);


	/* Alles packen (Layout) */
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET( vbox_main ));
		gtk_box_pack_start( GTK_BOX (vbox_main), GTK_WIDGET( hbox_ButtonsBlackboard ), TRUE, FALSE, 0);
			gtk_box_pack_start( GTK_BOX (hbox_ButtonsBlackboard), GTK_WIDGET(vbox_buttons), FALSE, FALSE, 5);
				gtk_box_pack_start_defaults(GTK_BOX (vbox_buttons), GTK_WIDGET(requestWrite));
				gtk_box_pack_start_defaults(GTK_BOX (vbox_buttons), GTK_WIDGET(removePermissions));
				gtk_box_pack_start_defaults(GTK_BOX (vbox_buttons), GTK_WIDGET(logBlackboard));
				gtk_box_pack_start_defaults(GTK_BOX (vbox_buttons), GTK_WIDGET(quitClient));
				gtk_box_pack_start_defaults(GTK_BOX (vbox_buttons), GTK_WIDGET(quitAll));
			gtk_box_pack_start( GTK_BOX (hbox_ButtonsBlackboard), GTK_WIDGET(scrolledwindow), FALSE, FALSE, 0);
				gtk_container_add ( GTK_CONTAINER (scrolledwindow), GTK_WIDGET(textview) );
		gtk_box_pack_start( GTK_BOX (vbox_main), GTK_WIDGET(hSep), FALSE, FALSE, 0);
		gtk_box_pack_start( GTK_BOX (vbox_main), GTK_WIDGET(statusBox), FALSE, FALSE, 0);
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(statusMode));
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(vSep[0]));
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(statusRole));
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(vSep[1]));
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(statusWrite));
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(vSep[2]));
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(statusVersion));
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(vSep[3]));
			gtk_container_add(GTK_CONTAINER(statusBox), GTK_WIDGET(statusUsers));

	return;
}



 /**************************************************************************************\
 * Funktion:           popupQuestionDialog                                              *
 *                                                                                      *
 * Kurzbeschreibung:   helper                                                           *
 *                     Eine Popupnachricht wird dem Benutzer modal angezeigt.           *
 *                     Er hat die Wahl zwischen zwei Antworten (Ja Nein).               *
 *                     Benutzerbefragung                                                *
 *                                                                                      *
 * Ergebnis:           Benutzerantwort: JA==1; Nein==0                                  *
 *                                                                                      *
 * Import-Parameter:   title:    Nachrichtentitel                                       *
 *                     question: Nachrichtentext(Frage)                                 *
 *                                                                                      *
 * Export-Parameter:   -                                                                *
 *                                                                                      *
 \**************************************************************************************/
int popupQuestionDialog(const char *title, const char *question)
{
	GtkWidget *dialog, *label;
	int result;

	dialog = gtk_dialog_new_with_buttons (
                                title,
                                window,
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                "Ja", GTK_RESPONSE_ACCEPT,
                                "Nein", GTK_RESPONSE_REJECT,
                                NULL);
  	label = gtk_label_new (question);

  	gtk_container_add(GTK_CONTAINER(GTK_DIALOG( dialog )->vbox), label);

  	gtk_widget_show_all (dialog);

  	switch ( gtk_dialog_run(GTK_DIALOG (dialog)) )
  	{
  		case GTK_RESPONSE_ACCEPT:
			result = 1;
  			break;
  		case GTK_RESPONSE_REJECT:
  		case GTK_RESPONSE_DELETE_EVENT:
  		default:
			result = 0;
			break;
  	}

  	gtk_widget_destroy (dialog);
  	return result;
}

 /**************************************************************************************\
 * Funktion:     popupMessage                                                           *
 *                                                                                      *
 * Kurzbeschreibung:  helper                                                            *
 *                    Eine Popupnachricht wird dem Benutzer modal angezeigt.            *
 *                    Um weitere GUI-eingaben machen zu können muss er die              *
 *                    Popupnachricht quittieren.                                        *
 *                    Benutzerbenachrichtigung                                          *
 *                                                                                      *
 * Ergebnis:          -                                                                 *
 *                                                                                      *
 * Import-Parameter:  message:  Nachrichtentext(Information)                            *
 *                                                                                      *
 * Export-Parameter:  -                                                                 *
 *                                                                                      *
 \**************************************************************************************/
void popupMessage(const char *message)
{
	GtkWidget *dialog;
	dialog = gtk_message_dialog_new (window,
                                  	GTK_DIALOG_DESTROY_WITH_PARENT,
                                  	GTK_MESSAGE_INFO,
                                  	GTK_BUTTONS_OK,
                                  	"%s", message);
 	gtk_dialog_run (GTK_DIALOG (dialog));
 	gtk_widget_destroy (dialog);
}
