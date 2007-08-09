/*
 * This file is part of Other Maemo Weather(omweather)
 *
 * Copyright (C) 2006 Vlad Vasiliev
 * Copyright (C) 2006 Pavel Fialko
 * 	for the code
 *        
 * Copyright (C) Superkaramba's Liquid Weather ++ team
 *	for ALL the artwork (icons)
 *        The maintainer (also the main author I believe)
 *        is Matthew <dmbkiwi@yahoo.com>
 *  http://liquidweather.net/icons.html#iconsets
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
*/
/*******************************************************************************/
#include "weather-home_download.h"
/*******************************************************************************/
static GString *url = NULL;
static GString *full_filename_new_xml = NULL;
CURL *curl_handle = NULL;
CURL *curl_multi = NULL;
GtkWidget *update_window = NULL;     
#ifdef USE_CONIC
#include <conic/conic.h>
#define USER_DATA_MAGIC 0xaadcaadc
static ConIcConnection *connection;
#endif

/*******************************************************************************/
/* Create standard Hildon animation small window */
void create_window_update(){
    update_window = hildon_banner_show_animation(app->main_window,
						    NULL,
						    _("Update weather"));
}
/*******************************************************************************/
static DBusHandlerResult
get_connection_status_signal_cb(DBusConnection *connection,
        DBusMessage *message, void *user_data){

    gchar *iap_name = NULL, *iap_nw_type = NULL, *iap_state = NULL;
    
//#ifndef RELEASE
    fprintf(stderr,"%s()\n", __PRETTY_FUNCTION__);
//#endif
    /* check signal */
    if(!dbus_message_is_signal(message,
                ICD_DBUS_INTERFACE,
                ICD_STATUS_CHANGED_SIG)){
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if(!dbus_message_get_args(message, NULL,
                DBUS_TYPE_STRING, &iap_name,
                DBUS_TYPE_STRING, &iap_nw_type,
                DBUS_TYPE_STRING, &iap_state,
                DBUS_TYPE_INVALID)){
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    fprintf(stderr,"  > iap_state = %s\n", iap_state);
    if(!strcmp(iap_state, "CONNECTED")){
        if(!app->iap_connected){
            app->iap_connected = TRUE;
        }
    }
    else if(app->iap_connected){
        app->iap_connected = FALSE; /* !!!!!!!!! Need Remove download */
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
/*******************************************************************************/
/* Callback function for request  connection to Internet */
void iap_callback(struct iap_event_t *event, void *arg){
    switch(event->type){
	case OSSO_IAP_CONNECTED:
#ifndef RELEASE
	    second_attempt = TRUE;
	    update_weather();
#endif
    	    app->iap_connected = TRUE;
	break;
	case OSSO_IAP_DISCONNECTED:
	    app->iap_connected = FALSE;
	break;
	case OSSO_IAP_ERROR:
    	    app->iap_connected = FALSE;
	    hildon_banner_show_information(app->main_window,
					    NULL,
					    _("Not connected to Internet"));
	break;
    }
}
/*******************************************************************************/
/* Check connect to Internet and connection if it not */
gboolean get_connected(void){
    fprintf(stderr,"%s()\n", __PRETTY_FUNCTION__);
    /* Register a callback function for IAP related events. */
    if(osso_iap_cb(iap_callback) != OSSO_OK)
	return FALSE;
    if(osso_iap_connect(OSSO_IAP_ANY, OSSO_IAP_REQUESTED_CONNECT, NULL) != OSSO_OK)
	return FALSE;
    return TRUE;
}
/*******************************************************************************/
/* Check connect to Internet */
gboolean check_connected(void){
    fprintf(stderr,"%s()\n", __PRETTY_FUNCTION__);
    if(osso_iap_connect(OSSO_IAP_ANY, OSSO_IAP_REQUESTED_CONNECT, NULL) != OSSO_OK)
	return FALSE;
    return TRUE;
}
/*******************************************************************************/
#ifdef USE_CONIC
#define OSSO_CON_IC_CONNECTING             0x05

static void connection_cb(ConIcConnection *connection,
                          ConIcConnectionEvent *event,
                          gpointer user_data)
{
    const gchar *iap_id, *bearer;
    ConIcConnectionStatus status;
    ConIcConnectionError error;
/*
    g_assert(GPOINTER_TO_INT(user_data) == USER_DATA_MAGIC);
    g_assert(CON_IC_IS_CONNECTION_EVENT(event));

    status = con_ic_connection_event_get_status(event);
    error = con_ic_connection_event_get_error(event);
    iap_id = con_ic_event_get_iap_id(CON_IC_EVENT(event));
    bearer = con_ic_event_get_bearer_type(CON_IC_EVENT(event));

    switch (status) {

        case CON_IC_STATUS_CONNECTED:
            connected = TRUE;
            is_connecting = FALSE;
            g_signal_emit_by_name(G_OBJECT(webglobal), "connectivity-status", status);
            return ;

        case CON_IC_STATUS_DISCONNECTED:
#ifdef USE_DBUS
        // return;
#else
        break;
#endif
        case CON_IC_STATUS_DISCONNECTING:
            break;

        default:
            break;
    }

    connected = FALSE;
    g_signal_emit_by_name(G_OBJECT(webglobal), "connectivity-status", status);
    if (gsloop && g_main_loop_is_running(gsloop))
        g_main_loop_quit (gsloop);
    gsloop = NULL;
    is_connecting = FALSE;
*/    
}
#endif


/*******************************************************************************/
void weather_initialize_dbus(void){

    gchar *filter_string;
    
    fprintf(stderr,"%s()\n", __PRETTY_FUNCTION__);
    if(!app->dbus_is_initialize){   
	/* Add D-BUS signal handler for 'status_changed' */
        DBusConnection *dbus_conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
        filter_string = g_strdup_printf("interface=%s", ICD_DBUS_INTERFACE);
        /* add match */
        dbus_bus_add_match(dbus_conn, filter_string, NULL);
        g_free(filter_string);
        /* add the callback */
        dbus_connection_add_filter(dbus_conn,
                		    get_connection_status_signal_cb,
                		    NULL, NULL);
    	osso_iap_cb(iap_callback);
    /* For Debug on i386 */
    #ifndef RELEASE
	app->iap_connected = TRUE; 
    #endif
	app->dbus_is_initialize = TRUE;
#ifdef USE_CONIC
    connection = con_ic_connection_new();
    g_assert(connection != NULL);

    g_signal_connect(G_OBJECT(connection), "connection-event",
                     G_CALLBACK(connection_cb),
                     GINT_TO_POINTER(USER_DATA_MAGIC));
#endif

    }
    
}
/*******************************************************************************/
/* Init easy curl */
CURL* weather_curl_init(CURL *curl_handle){
    curl_handle = curl_easy_init(); 
    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1); 
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1); 
    curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1); 
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, 
            "Mozilla/5.0 (X11; U; Linux i686; en-US; " 
            "rv:1.8.1.1) Gecko/20061205 Iceweasel/2.0.0.1"); 
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30); 
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 10); 
    config_update_proxy();
    /* Set Proxy option */
    if(app->config->iap_http_proxy_host){ 
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, app->config->iap_http_proxy_host); 
        if(app->config->iap_http_proxy_port) 
            curl_easy_setopt(curl_handle, CURLOPT_PROXYPORT, app->config->iap_http_proxy_port); 
    } 
    return curl_handle;    
}
/*******************************************************************************/
int data_read(void *buffer, size_t size, size_t nmemb, void *stream){
    int result;
    struct HtmlFile *out = (struct HtmlFile *)stream;

    if(out && !out->stream){
    /* open file for writing */
	out->stream = fopen(out->filename, "wb");
	if(!out->stream)
	    return -1; /* failure, can't open file to write */      
    }
    result = fwrite(buffer, size, nmemb, out->stream);
    return result;
}			  
/*******************************************************************************/
/* Form URL and filename for  write xml file. 
   Returns TRUE if the station is taken from the list
   Else return FLASE. This the end list
*/
gboolean form_url_and_filename(){
    if(tmplist != NULL){
        ws = tmplist->data;
        if(ws->id_station != NULL){
	    if(url){
		g_string_free(url, TRUE);    
		url = NULL;
	    } 
	    if(full_filename_new_xml){
	        g_string_free(full_filename_new_xml, TRUE);    
		full_filename_new_xml = NULL;
	    } 
	    url = g_string_new(NULL);        
    	    g_string_append_printf(url,"http://xoap.weather.com/weather/local/%s?cc=*&prod=xoap&par=1004517364&key=a29796f587f206b2&unit=m&dayf=%d",
				    ws->id_station, Max_count_weather_day);
	    full_filename_new_xml = g_string_new(NULL);        
	    g_string_append_printf(full_filename_new_xml,"%s/%s.xml.new",
				    app->config->cache_dir_name, ws->id_station);
	    tmplist = g_slist_next(tmplist);
	    /* Forming structure for download data of weather */
	    html_file.filename = full_filename_new_xml->str;
    	    html_file.stream = NULL;
	    return TRUE;
	}
	else
	    return FALSE;	
    }
    else
	return FALSE;
}
/*******************************************************************************/
/* Download html/xml file. Call every 100 ms after begin download */
gboolean download_html(gpointer data){
    CURLMsg	*msg;
    CURLMcode	mret;
    fd_set	rs, ws, es;
    int		max;

    fprintf(stderr,"%s()\n", __PRETTY_FUNCTION__);
    if(app->popup_window && app->show_update_window){
	gtk_widget_destroy(app->popup_window);   
        app->popup_window=NULL;
    }
    /* If not connected and it autoupdate do go away */
    if(!app->show_update_window && !app->iap_connected){
    	app->flag_updating = 0;
	return FALSE;
    }
    if(app->iap_connected) 
	second_attempt = TRUE;
    if( app->show_update_window && (!second_attempt) ){
    
        fprintf(stderr,"osso_iap_connect(OSSO_IAP_ANY, OSSO_IAP_REQUESTED_CONNECT, NULL) != OSSO_OK){\n");
	/* Check this code */
/*        if(osso_iap_connect(OSSO_IAP_ANY, OSSO_IAP_REQUESTED_CONNECT, NULL) != OSSO_OK){
    	    fprintf(stderr,"after 1 osso_iap_connect(OSSO_IAP_ANY, OSSO_IAP_REQUESTED_CONNECT, NULL) != OSSO_OK){\n");	
	    app->flag_updating = 0;
    	    return FALSE;
	}   
*/
#ifdef USE_CONIC
        con_ic_connection_connect(connection, CON_IC_CONNECT_FLAG_NONE);
#endif
	fprintf(stderr,"after 2 osso_iap_connect(OSSO_IAP_ANY, OSSO_IAP_REQUESTED_CONNECT, NULL) != OSSO_OK){\n");	
	app->flag_updating = 0; 
        return FALSE;
    }
    
    second_attempt = FALSE;
    /* The second stage */
    /* call curl_multi_perform for read weather data from Inet */
    if(curl_multi && CURLM_CALL_MULTI_PERFORM ==
            curl_multi_perform(curl_multi, &num_transfers))
        return TRUE; /* return to UI */
    /* The first stage */
    if(!curl_handle){
	if(app->show_update_window)
    	    create_window_update(); /* Window with update information */
        /* Initialize list */
        tmplist = stations_view_list;
	if(!form_url_and_filename()){
	    if(url){
		g_string_free(url, TRUE);    
		url = NULL;
	    }	 
	    if(full_filename_new_xml){
	        g_string_free(full_filename_new_xml, TRUE);    
	    	full_filename_new_xml = NULL;
	    }
	    app->flag_updating = 0;	 
	    return FALSE; /* The strange error */		
	}    
	/* Init easy_curl */
	curl_handle = weather_curl_init(curl_handle);
	curl_easy_setopt(curl_handle, CURLOPT_URL, url->str);
	/* Init curl_mult */
	if(!curl_multi)
    	    curl_multi = curl_multi_init();
	max = 0;
	FD_ZERO(&rs);
	FD_ZERO(&ws);
	FD_ZERO(&es);
	mret = curl_multi_fdset(curl_multi,&rs,&ws,&es,&max);
	if(mret != CURLM_OK){
	    fprintf (stderr,"Error CURL\n");
	}
	/* set options for the curl easy handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &html_file);		
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, data_read);	
        /* for debug */
        /*    curl_easy_setopt(curl_handle, CURLOPT_URL, "http://127.0.0.1"); */
        /* add the easy handle to a multi session */
        curl_multi_add_handle(curl_multi, curl_handle);	
        return TRUE; /* return to UI */
    }
    else{
        /* The third stage */
	num_msgs = 0;
	while(curl_multi && (msg = curl_multi_info_read(curl_multi, &num_msgs))){
	    if(msg->msg == CURLMSG_DONE){	  
		if(msg->data.result != CURLE_OK){ /* Not success of the download */
		    if(app->show_update_window)
			hildon_banner_show_information(app->main_window, 
							NULL,
							_("Did not download weather"));
		}
		else{ /* Clean */
		    mret = curl_multi_remove_handle(curl_multi,curl_handle); /* Delete curl_handle from curl_multi */
		    if (mret != CURLM_OK)
			fprintf(stderr," Error remove handle %p\n",curl_handle);
			
		    curl_easy_cleanup(curl_handle); 
		    curl_handle = NULL;

		    if(html_file.stream)
        		fclose(html_file.stream);
		    				
		    if(!form_url_and_filename()){ /* Success - all is downloaded */
			if(app->show_update_window)
			    hildon_banner_show_information(app->main_window,
							    NULL,
							    _("Weather updated"));
        		weather_frame_update(FALSE);	
		    }
		    else{
			/* set options for the curl easy handle */
			curl_handle = weather_curl_init(curl_handle);
			curl_easy_setopt(curl_handle, CURLOPT_URL, url->str);
        		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &html_file);		
        		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, data_read);	
    			/* add the easy handle to a multi session */
    			curl_multi_add_handle(curl_multi, curl_handle);	
			return TRUE;/* Download next station */
		    }		    
		}

		if(update_window){
		    gtk_widget_destroy(update_window);
		    update_window = NULL;
		}
	  /* Clean all */    
	  /*
		if (msg->easy_handle) {
    		    curl_multi_remove_handle(curl_multi,msg->easy_handle);
		    curl_easy_cleanup(msg->easy_handle);
		    msg->easy_handle = NULL;
		}
		*/
		if(curl_handle){
		    curl_easy_cleanup(curl_handle); 
		    curl_handle = NULL;
		}
		curl_multi_cleanup(curl_multi);
		curl_multi = NULL;
		curl_handle = NULL;
		if(url){
		    g_string_free(url, TRUE);    
		    url = NULL;
		}	 
		if(full_filename_new_xml){
		    g_string_free(full_filename_new_xml, TRUE);    	  
		    full_filename_new_xml = NULL;
		}
		app->flag_updating = 0;     
		return FALSE; /* This is the end */
	    }
	}
	return TRUE;
    }
    app->flag_updating = 0;
    return FALSE;
}
/*******************************************************************************/
void clean_download(void){
    if(curl_multi)
	curl_multi_cleanup(curl_multi);
    curl_multi = NULL;
    curl_handle = NULL;
    if(update_window){
        gtk_widget_destroy(update_window);
        update_window = NULL;
    }
}
/*******************************************************************************/
