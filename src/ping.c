/**
 *
 * Copyright (c) 2009 LxDE Developers, see the file AUTHORS for details.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib/gi18n.h>

#include <string.h>

#include <lxpanel/plugin.h>

typedef struct {
    unsigned int timer;
    Plugin * plugin;
    GtkWidget *main;
    GtkWidget *widget;
    GtkTooltips *tip;
    gchar* text;
    gint   speed;
    gint   wait;
    gint   cnt;
    gint   color;
    gboolean bool;
    gchar* szFile;
} Ping;

#define CLR_RED             0
#define CLR_GREEN           1
#define CLR_YELLOW          2
#define CLR_GREY            3

gint colors_a[] =
{
    0xFF0000,
    0x00FF00,
    0xFFFF00,
    0x777777,
    0
};

char* trim(char*s)
{
    char *end = s + strlen(s)-1;
    while(*s && isspace(*s))
        *s++ = 0;
    while(isspace(*end))
        *end-- = 0;
    return s;
}

void ping(Ping *egz)
{
	char buffer[60] = {0};
    char *line = "777777";
    FILE *pp = NULL;

	pp = popen(egz->szFile, "r");
    if (pp != NULL)
    {
        while (1)
        {
            char buf[10];
            line = fgets(buf, sizeof buf, pp);
            if(!line) 
                break;
            sprintf(buffer, "<span color=\"#%s\"><b>%s</b></span>", trim(line), egz->text);
            gtk_label_set_markup (GTK_LABEL(egz->widget), buffer);
        }
        pclose(pp);
    }
}

gboolean button_press_event(GtkWidget *widget, GdkEventButton* event, Ping* plugin)
{
    ENTER2;

    if( event->button == 1 ) /* left button */
    {
    	ping((Ping*)plugin);
    }
    else if( event->button == 2 ) /* middle button */
    {
        ping((Ping*)plugin);
    }
    else if( event->button == 3 )  /* right button */
    {
        GtkMenu* popup = (GtkMenu*)lxpanel_get_panel_menu( plugin->plugin->panel, plugin->plugin, FALSE ); /* lxpanel_menu, can be reimplemented */
        gtk_menu_popup( popup, NULL, NULL, NULL, NULL, event->button, event->time );
        return TRUE;
    }

    RET2(TRUE);
}

gboolean scroll_event (GtkWidget *widget, GdkEventScroll *event, Plugin* plugin)
{
    ENTER2;
    RET2(TRUE);
}

static gint timer_event(Ping *egz)
{
    if(egz->bool) return TRUE;
    if(--egz->wait) return TRUE;
    egz->wait = egz->speed;

    ping(egz);

    return TRUE;
}

static gint update_tooltip(Ping *egz)
{
    char *tooltip;
    ENTER;

    tooltip = g_strdup_printf(egz->text);
    gtk_tooltips_set_tip(egz->tip, egz->main, tooltip, NULL);
    g_free(tooltip);

    RET(TRUE);
}

static int BuildMonitor_constructor(Plugin *p, char** fp)
{
    Ping *egz;
    char buffer [60];

    ENTER;
    /* initialization */
    egz = g_new0(Ping, 1);
    egz->plugin = p;
    egz->text   = g_strdup("BUILD");
    egz->wait   = egz->speed  = 1;
    p->priv = egz;

    p->pwid = gtk_event_box_new();
    GTK_WIDGET_SET_FLAGS( p->pwid, GTK_NO_WINDOW );
    gtk_container_set_border_width( GTK_CONTAINER(p->pwid), 2 );

    egz->widget = gtk_label_new(" ");
    sprintf(buffer, "<span color=\"#%06x\"><b>%s</b></span>", colors_a[CLR_GREY], egz->text);
    gtk_label_set_markup (GTK_LABEL(egz->widget), buffer);

    gtk_container_add(GTK_CONTAINER(p->pwid), egz->widget);

    egz->main = p->pwid;
    egz->tip  = gtk_tooltips_new();
    update_tooltip(egz);

    g_signal_connect (G_OBJECT (p->pwid), "button_press_event", G_CALLBACK (button_press_event), (gpointer) egz);
    g_signal_connect (G_OBJECT (p->pwid), "scroll_event", G_CALLBACK (scroll_event), (gpointer) p);

    line s;
    s.len = 256;

    if (fp)
    {
        while (lxpanel_get_line(fp, &s) != LINE_BLOCK_END)
        {
            if (s.type == LINE_NONE) {
                ERR( "BuildMonitor: illegal token %s\n", s.str);
                goto error;
            }
            if (s.type == LINE_VAR) {
                if (!g_ascii_strcasecmp(s.t[0], "text")){
                    g_free(egz->text);
                    egz->text = g_strdup(s.t[1]);
                }else if (!g_ascii_strcasecmp(s.t[0], "speed")){
                    egz->speed = atoi(s.t[1]);
                }else if (!g_ascii_strcasecmp(s.t[0], "bool")){
                    egz->bool = atoi(s.t[1]); /* 0=false, 1=true */
                }else if (!g_ascii_strcasecmp(s.t[0], "file")){
                    egz->szFile = g_strdup(s.t[1]);
                }else {
                    ERR( "BuildMonitor: unknown var %s\n", s.t[0]);
                    continue;
                }
            }
            else {
                ERR( "BuildMonitor: illegal in this context %s\n", s.str);
                goto error;
            }
        }
    }

    egz->timer = g_timeout_add(1000, (GSourceFunc) timer_event, (gpointer)egz); /* set timer */

    gtk_widget_show(egz->widget); /* show plugin on panel */
    RET(TRUE);
error:
    destructor( p );
    RET(FALSE);
}

static void applyConfig(Plugin* p)
{
    ENTER;
    Ping *egz = (Ping *) p->priv;
    gchar buffer [60];

    egz->color = 0;
    if(egz->speed == 0) egz->speed = 1;
    egz->wait  = egz->speed;
    sprintf(buffer, "<span color=\"#%06x\"><b>%s</b></span>", colors_a[CLR_GREY], egz->text);
    update_tooltip(egz);
    RET();
}

static void config(Plugin *p, GtkWindow* parent) {
    ENTER;

    GtkWidget *dialog;
    Ping *egz = (Ping *) p->priv;
    dialog = create_generic_config_dlg(_(p->class->name),
            GTK_WIDGET(parent),
            (GSourceFunc) applyConfig, (gpointer) p,
            _("Text"), &egz->text, CONF_TYPE_STR,
            _("Speed"), &egz->speed, CONF_TYPE_INT,
            _("Stop"), &egz->bool, CONF_TYPE_BOOL,
            _("File"), &egz->szFile, CONF_TYPE_FILE_ENTRY,
            NULL);
    gtk_window_present(GTK_WINDOW(dialog));

    RET();
}

static void BuildMonitor_destructor(Plugin *p)
{
  ENTER;
  Ping *egz = (Ping *)p->priv;
  g_source_remove(egz->timer);
  g_free(egz->text);
  g_free(egz);
  RET();
}

static void save_config( Plugin* p, FILE* fp )
{
    ENTER;
    Ping *egz = (Ping *)p->priv;

    lxpanel_put_str( fp, "text", egz->text );
    lxpanel_put_int( fp, "speed", egz->speed );
    lxpanel_put_bool( fp, "bool", egz->bool );
    lxpanel_put_str( fp, "file", egz->szFile );
    RET();
}

PluginClass BuildMonitor_plugin_class = {

    PLUGINCLASS_VERSIONING,

    type : "BuildMonitor",
    name : N_("BuildMonitor"),
    version: "0.1",
    description : N_("BuildMonitor...not doing anything"),

    constructor : BuildMonitor_constructor,
    destructor  : BuildMonitor_destructor,
    config      : config,
    save        : save_config,
    panel_configuration_changed : NULL
};
