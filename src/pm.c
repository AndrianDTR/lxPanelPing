/**
 *
 * Copyright (c) 2014 Andrian Yablonskyy (andrian.yablonskyy@gmail.com).
 *
 * This program is free software; you can redistribute it and/or modify it
 * for personal or commercial use.
 * 
 * Any use of this Program or its part requires link to this Program author name.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib/gi18n.h>

#include <string.h>

#include <lxpanel/plugin.h>

#define CLR_GREY    0x777777

typedef struct {
    unsigned int    timer;
    Plugin*         plugin;
    GtkWidget*      main;
    GtkWidget*      widget;
    GtkTooltips*    tip;
    gchar*          text;
    gint            width;
    gint            speed;
    gint            wait;
    gboolean        start;
    gchar*          szFile;
    gchar*          args;
} tPingMonitor;

G_LOCK_DEFINE_STATIC (pingLock);

char* trim(char*s)
{
    char *end = s + strlen(s)-1;
    
    while(*s && isspace(*s))
        *s++ = 0;
    
    while(isspace(*end))
        *end-- = 0;
    
    return s;
}

void ping(tPingMonitor *egz)
{
	char cmd[255] = {0};
    char out[255] = {0};
    char buffer[255] = {0};
    char *line = "";
    FILE *pp = NULL;

    G_LOCK(pingLock);
            
    if(!egz->args)
        egz->args = g_strdup("");

    sprintf(cmd, "%s \"%s\" %s", egz->szFile, egz->text, egz->args);

    G_UNLOCK (pingLock);

	pp = popen(cmd, "r");
    if(pp != NULL)
    {
        while (1)
        {
            line = fgets(out, sizeof out, pp);
            
            if(!line) 
                break;

            sprintf(buffer, "<span>%s</span>", trim(line));

            G_LOCK(pingLock);
   
            gtk_label_set_markup (GTK_LABEL(egz->widget), buffer);
            gtk_label_set_width_chars(GTK_LABEL(egz->widget), egz->width);

            G_UNLOCK (pingLock);
        }
        pclose(pp);
    }
}

void *pingThread(void *args)
{
    tPingMonitor *data = (tPingMonitor*)args;
  
    gdk_threads_enter();

    ping(data);

    gdk_threads_leave ();

    return NULL;
}

void runPing(tPingMonitor* egz)
{
    pthread_t pingTid;

    pthread_create(&pingTid, NULL, pingThread, egz);
}

gboolean button_press_event(GtkWidget *widget, GdkEventButton* event, tPingMonitor* egz)
{
    ENTER2;

    /* left button */
    if(event->button == 1) 
    {
    	runPing(egz);
    }
    /* middle button */
    else if(event->button == 2)
    {
    }
    /* right button */
    else if(event->button == 3)  
    {
        /* lxpanel_menu, can be reimplemented */
        GtkMenu* popup = (GtkMenu*)lxpanel_get_panel_menu(egz->plugin->panel, egz->plugin, FALSE );
        gtk_menu_popup(popup, NULL, NULL, NULL, NULL, event->button, event->time);
        return TRUE;
    }

    RET2(TRUE);
}

gboolean scroll_event (GtkWidget *widget, GdkEventScroll *event, Plugin* plugin)
{
    ENTER2;
    RET2(TRUE);
}

static gint timer_event(tPingMonitor *egz)
{
    if(egz->start) 
        return TRUE;
    
    if(--egz->wait)
        return TRUE;
    
    egz->wait = egz->speed;

    runPing(egz);

    return TRUE;
}

static gint update_tooltip(tPingMonitor *egz)
{
    char *tooltip;
    ENTER;

    tooltip = g_strdup_printf(egz->text);
    gtk_tooltips_set_tip(egz->tip, egz->main, tooltip, NULL);
    g_free(tooltip);

    RET(TRUE);
}

static int pm_constructor(Plugin *p, char** fp)
{
    tPingMonitor *egz;
    char buffer [60];

    ENTER;
    
    /* initialization */
    egz = g_new0(tPingMonitor, 1);
    egz->plugin = p;
    egz->text   = g_strdup("PING");
    egz->szFile = g_strdup("");
    egz->args   = g_strdup("");
    egz->width  = 20;
    egz->wait   = egz->speed  = 1;
    p->priv     = egz;

    p->pwid = gtk_event_box_new();
    GTK_WIDGET_SET_FLAGS(p->pwid, GTK_NO_WINDOW);
    gtk_container_set_border_width(GTK_CONTAINER(p->pwid), 2);

    egz->widget = gtk_label_new(" ");
    sprintf(buffer, "<span color=\"#%06x\"><b>%s</b></span>", CLR_GREY, egz->text);
    gtk_label_set_markup (GTK_LABEL(egz->widget), buffer);
    gtk_label_set_width_chars(GTK_LABEL(egz->widget), egz->width);

    gtk_container_add(GTK_CONTAINER(p->pwid), egz->widget);

    egz->main = p->pwid;
    egz->tip  = gtk_tooltips_new();
    update_tooltip(egz);

    g_signal_connect(G_OBJECT(p->pwid), "button_press_event", G_CALLBACK (button_press_event), (gpointer) egz);
    g_signal_connect(G_OBJECT(p->pwid), "scroll_event", G_CALLBACK (scroll_event), (gpointer) p);

    line s;
    s.len = 256;

    if(fp)
    {
        while(lxpanel_get_line(fp, &s) != LINE_BLOCK_END)
        {
            if(s.type == LINE_NONE)
            {
                ERR( "Ping Monitor: illegal token %s\n", s.str);
                goto error;
            }
            
            if(s.type == LINE_VAR)
            {
                if(!g_ascii_strcasecmp(s.t[0], "text"))
                {
                    g_free(egz->text);
                    egz->text = g_strdup(s.t[1]);
                }
                else if(!g_ascii_strcasecmp(s.t[0], "width"))
                {
                    egz->width = atoi(s.t[1]);
                }
                else if(!g_ascii_strcasecmp(s.t[0], "speed"))
                {
                    egz->speed = atoi(s.t[1]);
                }
                else if(!g_ascii_strcasecmp(s.t[0], "start"))
                {
                    egz->start = atoi(s.t[1]); /* 0=false, 1=true */
                }
                else if(!g_ascii_strcasecmp(s.t[0], "file"))
                {
                    egz->szFile = g_strdup(s.t[1]);
                }
                else if(!g_ascii_strcasecmp(s.t[0], "args"))
                {
                    g_free(egz->args);
                    egz->args = g_strdup(s.t[1]);
                }
                else
                {
                    ERR("Ping Monitor: unknown var %s\n", s.t[0]);
                    continue;
                }
            }
            else
            {
                ERR("Ping Monitor: illegal in this context %s\n", s.str);
                goto error;
            }
        }
    }

    /* set timer */
    egz->timer = g_timeout_add(1000, (GSourceFunc) timer_event, (gpointer)egz);

    /* show plugin on panel */
    gtk_widget_show(egz->widget);
    RET(TRUE);

error:
    destructor(p);
    RET(FALSE);
}

static void applyConfig(Plugin* p)
{
    ENTER;
    tPingMonitor *egz = (tPingMonitor *)p->priv;
    gchar buffer[60];

    if(egz->speed == 0)
    	egz->speed = 1;
    
    egz->wait  = egz->speed;

    runPing(egz);
    update_tooltip(egz);

    RET();
}

static void config(Plugin *p, GtkWindow* parent)
{
    ENTER;

    GtkWidget *dialog;
    tPingMonitor *egz = (tPingMonitor *) p->priv;
    dialog = create_generic_config_dlg(_(p->class->name),
            GTK_WIDGET(parent),
            (GSourceFunc) applyConfig, (gpointer) p,
            _("Text"), &egz->text, CONF_TYPE_STR,
            _("Label width on panel"), &egz->width, CONF_TYPE_INT,
            _("Run script every (seconds)"), &egz->speed, CONF_TYPE_INT,
            _("Stop timer"), &egz->start, CONF_TYPE_BOOL,
            _("Script path"), &egz->szFile, CONF_TYPE_FILE_ENTRY,
            _("Script args"), &egz->args, CONF_TYPE_STR,
            NULL);
    gtk_window_present(GTK_WINDOW(dialog));

    RET();
}

static void pm_destructor(Plugin *p)
{
    ENTER;
    tPingMonitor *egz = (tPingMonitor *)p->priv;
    g_source_remove(egz->timer);
    g_free(egz->text);
    g_free(egz);
RET();
}

static void save_config(Plugin* p, FILE* fp)
{
    ENTER;
    tPingMonitor *egz = (tPingMonitor *)p->priv;

    lxpanel_put_str(fp, "text", egz->text);
    lxpanel_put_int(fp, "width", egz->width);
    lxpanel_put_int(fp, "speed", egz->speed);
    lxpanel_put_bool(fp, "start", egz->start);
    lxpanel_put_str(fp, "file", egz->szFile);
    lxpanel_put_str(fp, "args", egz->args);
    RET();
}

PluginClass pm_plugin_class = {

    PLUGINCLASS_VERSIONING,

    type : "pm",
    name : N_("Ping Monitor"),
    version: "0.1",
    description : N_("Ping Monitor - status monitor for any process, event or action."),

    constructor : pm_constructor,
    destructor  : pm_destructor,
    config      : config,
    save        : save_config,
    panel_configuration_changed : NULL
};
