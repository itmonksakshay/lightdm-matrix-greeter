#include<stdio.h>
#include<gtk/gtk.h>
#include<lightdm.h>
#include<config.h>
#include<string.h>
#include<unistd.h>
#include"src/lightdm-matrix-greeter-ui.h"

GtkBuilder *builder;
GtkWidget *main_window,*main_overlay,*command_dialog;
GtkEntry *user_entry;
GtkLabel *hostname_label,*user_entry_label,*status_response_label;
GtkEntry *user_entry_field;
GtkButton *shutdown_button,*restart_button,*hibernate_button,*sleep_button;
GtkImage *main_logo,*background_image;
GError *error = NULL;
GMainLoop *main_loop;
GAsyncResult *result;
gint response;

    GdkScreen       *screen;
    GdkDisplay      *display;
    GdkMonitor      *monitor;
    GdkRectangle     geometry;


static LightDMGreeter *greeter;

gchar *user_text = "Username",*pass_text = "Password" , *session = "i3";

//Command Button Options shutdown Restart Hibernate
static void shutdown_command_control(){
	
  	command_dialog = gtk_message_dialog_new(GTK_WINDOW (main_window),
				GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_OTHER,GTK_BUTTONS_NONE,
		"Are you sure you want to close all programs and shutdown the computer?");
           
	gtk_dialog_add_buttons (GTK_DIALOG (command_dialog),"Return To Login", FALSE,"Shutdown", TRUE, NULL);
	gtk_widget_show_all (command_dialog);
  	if(gtk_dialog_run(GTK_DIALOG(command_dialog))){
  		lightdm_shutdown(NULL);	
	}
  	gtk_widget_destroy(command_dialog);
}

static void restart_command_control(){
		command_dialog = gtk_message_dialog_new(GTK_WINDOW (main_window),
				GTK_DIALOG_MODAL| GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_OTHER,GTK_BUTTONS_NONE,
		"Are you sure you want to close all programs and Restart computer?");
           
	gtk_dialog_add_buttons (GTK_DIALOG (command_dialog),"Return To Login", FALSE,"Restart", TRUE, NULL);
	gtk_widget_show_all (command_dialog);
  	if(gtk_dialog_run(GTK_DIALOG(command_dialog))){
		  lightdm_restart(NULL);	
	}
  	gtk_widget_destroy(command_dialog);


}

static void command_button_visiblty(){
	if(!lightdm_get_can_suspend()){
		gtk_widget_hide(GTK_WIDGET(sleep_button));

	}else{
		g_signal_connect(sleep_button,"clicked",G_CALLBACK(lightdm_suspend),NULL);
	}if(!lightdm_get_can_hibernate()){
		gtk_widget_hide(GTK_WIDGET(hibernate_button));
	}else{
		g_signal_connect(hibernate_button,"clicked",G_CALLBACK(lightdm_hibernate),NULL);
	}if(!lightdm_get_can_restart()){
		gtk_widget_hide(GTK_WIDGET(restart_button));
	}else{
		g_signal_connect(restart_button,"clicked",G_CALLBACK(restart_command_control),NULL);
	}if(!lightdm_get_can_shutdown()){
		gtk_widget_hide(GTK_WIDGET(shutdown_button));
	}else{
		g_signal_connect(shutdown_button,"clicked",G_CALLBACK(shutdown_command_control),NULL);
	}
}

//Show Status Label

static void show_message_cb (LightDMGreeter *ldm, const gchar *text, LightDMPromptType type){

  	gtk_label_set_text (status_response_label, text);

}

// Show Login entry prompt

static void show_prompt_cb (LightDMGreeter *ldm, const gchar *text, LightDMPromptType type) {

    gtk_label_set_text(user_entry_label,type == LIGHTDM_PROMPT_TYPE_SECRET ? pass_text : user_text);
    gtk_entry_set_text(user_entry_field, "");
    gtk_entry_set_visibility(user_entry_field,type == LIGHTDM_PROMPT_TYPE_SECRET ? 0 : 1);

}


static void login_cb() {
	gtk_label_set_text(status_response_label,"");
 	if (lightdm_greeter_get_is_authenticated(greeter)) {
		// authentication completed - start session
        	lightdm_greeter_start_session_sync(greeter,session,NULL);
    	} else if (lightdm_greeter_get_in_authentication(greeter)) {
      	    	// authentication in progress - send password
        	lightdm_greeter_respond(greeter,gtk_entry_get_text(user_entry_field),NULL);
    	} else {
	          // authentication request - send username
        	lightdm_greeter_authenticate(greeter,gtk_entry_get_text(user_entry_field),NULL);
    	}
}

// Authentication Response

static void authentication_complete_cb (LightDMGreeter *ldm) {

    if (!lightdm_greeter_get_is_authenticated (ldm)) {
        gtk_label_set_text(status_response_label, "Authentication Failure.");

    } else if (!lightdm_greeter_start_session_sync (ldm,session, NULL)) {
        gtk_label_set_text(status_response_label, "Failed to start session.");

    }

    lightdm_greeter_authenticate (ldm, NULL, NULL);

}

int main(int argc,char **argv){
		
	gtk_init(&argc,&argv);

	builder = gtk_builder_new ();
	if(!gtk_builder_add_from_string(builder,lightdm_gtk_greeter_ui,
				lightdm_gtk_greeter_ui_length,&error)){
		g_warning("Error Loading UI :- %s\n",error->message);
		return EXIT_FAILURE;
	}
	g_clear_error(&error);

	main_window = GTK_WIDGET(gtk_builder_get_object(builder, "matrix_greeter_window"));
	main_overlay = GTK_WIDGET(gtk_builder_get_object(builder,"matrix_greeter_overlay"));
	main_logo = GTK_IMAGE(gtk_builder_get_object(builder,"matrix_header_logo"));
	
	hostname_label = GTK_LABEL(gtk_builder_get_object(builder,"matrix_hostname_label"));
	
	user_entry_label = GTK_LABEL(gtk_builder_get_object(builder,"matrix_input_label"));
	gtk_label_set_text(user_entry_label,user_text);

	user_entry_field = GTK_ENTRY(gtk_builder_get_object(builder,"matrix_user_entry"));
	status_response_label = GTK_LABEL(gtk_builder_get_object(builder,"matrix_response_label"));
	gtk_label_set_text(status_response_label,"");

	shutdown_button = GTK_BUTTON(gtk_builder_get_object(builder,"matrix_shutdown_button"));
	restart_button = GTK_BUTTON(gtk_builder_get_object(builder,"matrix_restart_button"));
	hibernate_button = GTK_BUTTON(gtk_builder_get_object(builder,"matrix_hibernate_button"));
	sleep_button = GTK_BUTTON(gtk_builder_get_object(builder,"matrix_sleep_button"));


	command_button_visiblty();
    	gdk_monitor_get_geometry(gdk_display_get_primary_monitor(gdk_display_get_default()), &geometry);
    	gtk_window_set_default_size(GTK_WINDOW(main_window),geometry.width,geometry.height);
	
	gtk_widget_show(GTK_WIDGET(main_window));


	g_signal_connect(user_entry_field,"activate",G_CALLBACK(login_cb), NULL);


	gtk_label_set_text(hostname_label,lightdm_get_hostname());

	//Hostname
		
	greeter = lightdm_greeter_new();
	g_signal_connect (greeter, "show-prompt", G_CALLBACK (show_prompt_cb), NULL);
	g_signal_connect (greeter, "show-message", G_CALLBACK (show_message_cb), NULL);
    	g_signal_connect (greeter, "authentication-complete",
		       	G_CALLBACK (authentication_complete_cb), NULL);
	

	 if(!lightdm_greeter_connect_to_daemon_sync(greeter,&error)){
	   return EXIT_FAILURE;		
	}
		
	lightdm_greeter_authenticate(greeter, NULL, NULL);

	main_loop = g_main_loop_new (NULL, 0);

    	// start main loop
    	g_main_loop_run (main_loop);
	return 0;
}
