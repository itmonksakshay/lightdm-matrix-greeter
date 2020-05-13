#include<stdio.h>
#include<gtk/gtk.h>
#include<lightdm.h>
#include<config.h>
#include<string.h>
#include<unistd.h>
#include"src/lightdm-matrix-greeter-ui.h"
#include"src/lightdm-matrix-greeter-css.h"

GtkBuilder *builder;
GtkCssProvider *css_provider;
GtkWidget *main_window,*main_overlay,*command_dialog;
GtkEntry *user_entry;
GtkLabel *hostname_label,*user_entry_label,*status_response_label,*dialog_info_label;
GtkEntry *user_entry_field;
GtkButton *shutdown_button,*restart_button,*hibernate_button,*sleep_button,*dialog_command_button,*dialog_cancel_button;
GtkImage *main_logo,*background_image;
GtkBox *matrix_login_box,*matrix_command_box;
GError *error = NULL;
GMainLoop *main_loop;
GAsyncResult *result;
gint response;
GdkScreen *screen;
GdkDisplay *display;
GdkMonitor *monitor;
GdkRectangle geometry;

typedef struct dialog_data_struct {

	gchar *button_name;
	gboolean (*function_name)();

}command_name;


static LightDMGreeter *greeter;

gchar *user_text = "Username",*pass_text = "Password" , *session = "i3";


void center_window (GtkWindow *window){
    GtkAllocation allocation;
    GdkRectangle monitor_geometry;

      	gdk_monitor_get_geometry(gdk_display_get_primary_monitor(gdk_display_get_default()), 
			&monitor_geometry);    
	gtk_widget_get_allocation (GTK_WIDGET (window), &allocation);
    	gtk_window_move (window,
                     monitor_geometry.x + (monitor_geometry.width - allocation.width) / 2,
                     monitor_geometry.y + (monitor_geometry.height - allocation.height) / 2);
}


//Command Button Options shutdown Restart Hibernate
static void dialog_window(command_name *data){
	
	
	gtk_widget_hide(GTK_WIDGET(matrix_login_box));
	gtk_widget_hide(GTK_WIDGET(matrix_command_box));
	
	command_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "matrix_dialog_command"));
	
	dialog_command_button = GTK_BUTTON(gtk_builder_get_object(builder, 
				"matrix_dialog_command_action_button"));
	dialog_cancel_button = GTK_BUTTON(gtk_builder_get_object(builder, 
				"matrix_dialog_command_button_cancel"));

	gtk_button_set_label(dialog_cancel_button,"Return To Login");	
  
	dialog_info_label=GTK_LABEL(gtk_builder_get_object(builder,"matrix_dialog_info_label"));
	gtk_label_set_text(dialog_info_label,
			"Are you sure you want to close all programs and shutdown the computer?");
	
	gtk_button_set_label(dialog_command_button,data->button_name);
	gtk_window_set_transient_for(GTK_WINDOW(command_dialog),GTK_WINDOW(main_window));
	gtk_widget_show_all(command_dialog);
  	center_window(GTK_WINDOW(command_dialog));
	if(!gtk_dialog_run(GTK_DIALOG(command_dialog))){
		data->function_name();	
	}

	
	gtk_widget_hide_on_delete(command_dialog);		
	gtk_widget_show(GTK_WIDGET(matrix_login_box));
	gtk_widget_show(GTK_WIDGET(matrix_command_box));
}

void construct_dialog(GtkWidget *button,gpointer user_data){

 command_name *data = (command_name*)malloc(1*sizeof(command_name));
	if(!strcmp(user_data,"restart")){	
		data->function_name = lightdm_restart; 
		data->button_name = "Restart";
		dialog_window(data);
	}else{
		data->function_name = lightdm_shutdown; 
		data->button_name = "Shutdown";
		dialog_window(data);
	}
	g_free(data);
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
		g_signal_connect(restart_button,"clicked",
				G_CALLBACK(construct_dialog),"restart");
	}if(!lightdm_get_can_shutdown()){
		gtk_widget_hide(GTK_WIDGET(shutdown_button));
	}else{
		g_signal_connect(shutdown_button,"clicked",
				G_CALLBACK(construct_dialog),"shutdown");
	}
}


// Show Login entry prompt

static void show_prompt_cb (LightDMGreeter *ldm, const gchar *text, LightDMPromptType type) {

    gtk_label_set_text(user_entry_label,type == LIGHTDM_PROMPT_TYPE_SECRET ? pass_text : user_text);
    gtk_entry_set_text(user_entry_field, "");
    gtk_entry_set_visibility(user_entry_field,type == LIGHTDM_PROMPT_TYPE_SECRET ? 0 : 1);

}

gboolean is_user(const gchar *username){
	if(lightdm_user_list_get_user_by_name(lightdm_user_list_get_instance(),username) != NULL
			){
		return TRUE;
	}
	return FALSE;
}

static void login_cb() {
	gtk_label_set_text(status_response_label,"");
 	if (lightdm_greeter_get_is_authenticated(greeter)) {
		// authentication completed - start session
        	lightdm_greeter_start_session_sync(greeter,session,NULL);
    	} else if (lightdm_greeter_get_in_authentication(greeter)) {
      	    	// authentication in progress - send password
        	lightdm_greeter_respond(greeter,gtk_entry_get_text(user_entry_field),NULL);
    	} else if(!is_user(gtk_entry_get_text(user_entry_field))){
        	gtk_label_set_text(status_response_label, "Invalid User");
		gtk_entry_set_text(user_entry_field, "");

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

	css_provider = gtk_css_provider_new ();
    	gtk_css_provider_load_from_data (css_provider,matrix_lightdm_css,
			matrix_lightdm_css_length, NULL);
    	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), 
			GTK_STYLE_PROVIDER(css_provider),
                        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);


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
	
	matrix_login_box =  GTK_BOX(gtk_builder_get_object(builder,"matrix_login_box"));
	matrix_command_box = GTK_BOX(gtk_builder_get_object(builder,"matrix_command_box"));

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
    	gdk_monitor_get_geometry(gdk_display_get_primary_monitor(gdk_display_get_default()), 
			&geometry);
    	gtk_window_set_default_size(GTK_WINDOW(main_window),geometry.width,geometry.height);
	
	gtk_widget_show(GTK_WIDGET(main_window));

	g_signal_connect(user_entry_field,"activate",G_CALLBACK(login_cb), NULL);

	gtk_label_set_text(hostname_label,lightdm_get_hostname());

	//Hostname
		
	greeter = lightdm_greeter_new();
	g_signal_connect (greeter, "show-prompt", G_CALLBACK (show_prompt_cb), NULL);
    	g_signal_connect (greeter, "authentication-complete",
		       	G_CALLBACK (authentication_complete_cb), NULL);
	

	if(!lightdm_greeter_connect_to_daemon_sync(greeter,NULL)){
	 	return EXIT_FAILURE;
	}

	main_loop = g_main_loop_new (NULL, 0);

    	// start main loop
    	g_main_loop_run (main_loop);
	return 0;
}
