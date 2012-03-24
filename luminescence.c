#include "luminescence.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>

Lumi lumi;

KeyCallback **key_callbacks;
KeyCallback *key_grabber = 0;
int key_callbacks_size = 0;
int key_callback_count = 0;

bool on_key_press(GtkWidget *widget, GdkEventKey *event){
    int i = 0, code;
    for(; i<key_callback_count; i++){
        code = (*(key_grabber ? key_grabber : key_callbacks[i]))(event);
        if(code & FOCUS_GRAB){
            if(!key_grabber) key_grabber = key_callbacks[i];
            return code & EVENT_STOP ? TRUE : FALSE;
        }
        else if(key_grabber){
            key_grabber = 0;
            i--;
        }
        if(code & EVENT_STOP)
            return TRUE;
    }
    return TRUE;
}

void load_plugin(const char *path){
    void *plugin = dlopen(path, RTLD_LAZY);
    if(!plugin) return;

    // Init
    void (*plugin_init)(Lumi*) = dlsym(plugin, "init");
    if(plugin_init) (*plugin_init)(&lumi);

    // Key callback
    KeyCallback* plugin_key_callback = dlsym(plugin, "key_callback");
    if(plugin_key_callback){
        if(key_callback_count >= key_callbacks_size){
            // Grow key_callbacks
            key_callbacks_size += 8;
            key_callbacks = (KeyCallback**) realloc(key_callbacks, key_callbacks_size*sizeof(KeyCallback*));
        }
        key_callbacks[key_callback_count++] = plugin_key_callback;
    }
}

int is_visible(const struct dirent *entry){
    return entry->d_name[0] != '.';
}

void load_plugins(){
    struct dirent **entries;
    int entry_count = scandir("plugins", &entries, is_visible, alphasort);
    if(entry_count < 0) return;
    char filename[256];
    int i = 0;
    for(; i<entry_count; i++){
        strcpy(filename, "plugins/");
        strncat(filename, entries[i]->d_name, 247);
        load_plugin(filename);
        free(entries[i]);
    }
    free(entries);
}

int main(int argc, char **argv){
    gtk_init(&argc, &argv);

    // Window
    lumi.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_add_events(lumi.window, GDK_KEY_PRESS_MASK);
    gtk_window_set_has_resize_grip(GTK_WINDOW(lumi.window), FALSE);
    g_signal_connect(lumi.window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(lumi.window, "key-press-event", G_CALLBACK(on_key_press), NULL);

    // Layout
    GtkWidget *layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(lumi.window), layout);
    gtk_widget_show(layout);

    // Web view
    lumi.web_view = webkit_web_view_new();
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), lumi.web_view);
    gtk_box_pack_start(GTK_BOX(layout), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show_all(scrolled_window);

    // Status bar
    lumi.status_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(layout), lumi.status_bar, FALSE, FALSE, 0);
    gtk_widget_show(lumi.status_bar);

    load_plugins();

    gtk_widget_show(lumi.window);
    gtk_main();
    return 0;
}
