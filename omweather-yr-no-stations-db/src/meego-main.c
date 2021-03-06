/* vim: set sw=4 ts=4 et: */
/*
 * This file is part of omweather-weather-com-stations-db
 *
 * Copyright (C) 2006-2009 Vlad Vasiliev
 * 	for the code
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU  General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU  General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
*/
/*******************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "meego-main.h"
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <locale.h>
/*******************************************************************************/
static GHashTable *data = NULL;
/*******************************************************************************/
gboolean
source_init(void){
    data = g_hash_table_new(g_str_hash, g_str_equal);
    if(!data)
        return FALSE;
    return TRUE;
}
/*******************************************************************************/
void
source_destroy(void){
    GHashTable  *hashtable = NULL;
    GSList  *tmp = NULL;
    /* free station location data */
    hashtable = g_hash_table_lookup(data, "location");
    if(hashtable){
        g_hash_table_foreach(hashtable, free_fields, NULL);
        g_hash_table_remove_all(hashtable);
        g_hash_table_unref(hashtable);
    }
    /* free station current data */
    hashtable = g_hash_table_lookup(data, "current");
    if(hashtable){
        g_hash_table_foreach(hashtable, free_fields, NULL);
        g_hash_table_remove_all(hashtable);
        g_hash_table_unref(hashtable);
    }
    /* free station days data */
    tmp = g_hash_table_lookup(data, "forecast");
    while(tmp){
        hashtable = (GHashTable*)tmp->data;
        g_hash_table_foreach(hashtable, free_fields, NULL);
        g_hash_table_remove_all(hashtable);
        g_hash_table_unref(hashtable);
        tmp = g_slist_next(tmp);
    }
    tmp = g_hash_table_lookup(data, "forecast");
    if(tmp)
        g_slist_free(tmp);
    if(data){
        g_hash_table_remove_all(data);
        g_hash_table_destroy(data);
    }
}
/*******************************************************************************/
void
free_fields(gpointer key, gpointer val, gpointer user_data){
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    if(val){
        g_free(val);
        val = NULL;
    }
}
/*******************************************************************************/
convert_station_yrno_data(const gchar *station_id_with_path, const gchar *result_file,  gboolean get_detail_data){
 
    xmlDoc  *doc = NULL;
    xmlNode *root_node = NULL;
    gint    days_number = -1;
    gchar   buffer[1024],
            *delimiter = NULL;
    
    if(!station_id_with_path)
        return -1;
/* check for new file, if it exist, than rename it */
    *buffer = 0;
    snprintf(buffer, sizeof(buffer) - 1, "%s.new", station_id_with_path);
    if(!access(buffer, R_OK))
        rename(buffer, station_id_with_path);
    /* check file accessability */
    if(!access(station_id_with_path, R_OK)){
        /* check that the file containe valid data */
        doc = xmlReadFile(station_id_with_path, NULL, 0);
        if(!doc)
            return -1;
        root_node = xmlDocGetRootElement(doc);
        if(root_node->type == XML_ELEMENT_NODE &&
                strstr((char*)root_node->name, "err")){
            xmlFreeDoc(doc);
            xmlCleanupParser();
            return -2;
        }
        else{
            /* prepare station id */
            *buffer = 0;
            delimiter = strrchr(station_id_with_path, '/');
            if(delimiter){
                delimiter++; /* delete '/' */
                snprintf(buffer, sizeof(buffer) - 1, "%s", delimiter);
                delimiter = strrchr(buffer, '.');
                if(!delimiter){
                    xmlFreeDoc(doc);
                    xmlCleanupParser();
                    return -1;
                }
                *delimiter = 0;
             //   if(get_detail_data)
             //       days_number = parse_xml_detail_data(buffer, root_node, data);
             //   else
                days_number = parse_and_write_xml_data(buffer, root_node, result_file);
            }
            xmlFreeDoc(doc);
            xmlCleanupParser();
        }
    }
    else
        return -1;/* file isn't accessability */
    return days_number;
}
/*******************************************************************************/
gint
parse_and_write_xml_data(const gchar *station_id, xmlNode *root_node,  const gchar *result_file){
    xmlNode     *cur_node = NULL,
                *child_node = NULL,
                *child_node2 = NULL,
                *child_node3 = NULL,
                *child_node4 = NULL,
                *child_node1 = NULL;
    xmlChar     *temp_xml_string = NULL;
    xmlChar     *part_of_day = NULL;
    gint        count_day = 0;
    gchar       id_station[10],
                buff[256];
    struct tm   tmp_tm = {0};
    GSList      *forecast = NULL;
    GHashTable  *location = NULL,
                *current = NULL,
                *day = NULL;
    int         temp_hi = INT_MAX,temp_low = INT_MAX,temp_temp = INT_MAX;
    gboolean    first_day = TRUE;
    int         period;
    int         timezone = 0;
    GHashTable  *hash_for_translate;
    GHashTable  *hash_for_icons;
    int speed;
    time_t      utc_time;
    time_t      end_of_first_day;
    FILE        *file_out;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    hash_for_translate = hash_description_yrno_table_create();
    hash_for_icons = hash_icons_yrno_table_create();

    file_out = fopen(result_file, "w");
    if (!file_out)
        return -1;
    fprintf(file_out,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<station name=\"Station name\" id=\"%s\" xmlns=\"http://omweather.garage.maemo.org/schemas\">\n", station_id);
    fprintf(file_out," <units>\n  <t>C</t>\n  <ws>m/s</ws>\n  <wg>m/s</wg>\n  <d>km</d>\n");
    fprintf(file_out,"  <h>%%</h>  \n  <p>mmHg</p>\n </units>\n");


    for(cur_node = root_node->children; cur_node; cur_node = cur_node->next){
        if( cur_node->type == XML_ELEMENT_NODE ){
            /* get weather station data */
            if(!xmlStrcmp(cur_node->name, (const xmlChar *) "location" ) ){
                temp_xml_string = xmlGetProp(cur_node, (const xmlChar*)"id");
                snprintf(id_station, sizeof(id_station) - 1,
                            "%s", temp_xml_string);
                xmlFree(temp_xml_string);
                /* If station in xml not station in config file exit */
                //if( strcmp(id_station, station_id) )
                //    return -1;
                location = g_hash_table_new(g_str_hash, g_str_equal);
                for(child_node = cur_node->children; child_node; child_node = child_node->next){
                    if( child_node->type == XML_ELEMENT_NODE ){
                        if(!xmlStrcmp(child_node->name, (const xmlChar *) "location" ) ){
                            /* station lattitude */
                                temp_xml_string = xmlGetProp(child_node, (const xmlChar*)"latitude");
                                //g_hash_table_insert(location, "station_latitude",
                                //                    g_strdup((char*)temp_xml_string));
                                xmlFree(temp_xml_string);
                            /* station longitude */
                                temp_xml_string =  xmlGetProp(child_node, (const xmlChar*)"longitude");
                                //g_hash_table_insert(location, "station_longitude",
                                //                    g_strdup((char*)temp_xml_string));
                                xmlFree(temp_xml_string);
                                continue;
                        }
                        /* station time zone */
                        if( !xmlStrcmp(child_node->name, (const xmlChar *)"timezone") ){
                            temp_xml_string = xmlGetProp(child_node, (const xmlChar*)"utcoffsetMinutes");
                            memset(buff, 0, sizeof(buff));
                            if (temp_xml_string){
                                timezone = atoi(temp_xml_string)/60;
                                fprintf(file_out,"  <timezone>%i</timezone>\n", timezone);
                            }
                            xmlFree(temp_xml_string);
                                       continue;
                        }
                    }
                }
                //g_hash_table_insert(data, "location", (gpointer)location);
            }
            /* fill sun set sun rise */
            if(!xmlStrcmp(cur_node->name, (const xmlChar *) "sun" ) ){
                temp_xml_string = xmlGetProp(child_node1, (const xmlChar*)"rise");
                if (temp_xml_string){
                    setlocale(LC_TIME, "POSIX");
                    strptime((const char*)temp_xml_string, "%Y-%m-%dT", &tmp_tm);
                    setlocale(LC_TIME, "");
                    memset(buff, 0, sizeof(buff));
                    strftime(buff, sizeof(buff) - 1, "%a", &tmp_tm);
                    utc_time = mktime(&tmp_tm) - timezone * 3600;
                    fprintf(file_out,"    <period start=\"%li\" end=\"%l\"", utc_time, utc_time + 24*3600);
                    setlocale(LC_TIME, "POSIX");
                    strptime((const char*)temp_xml_string, "%Y-%m-%dT%H:%M:%S", &tmp_tm);
                    setlocale(LC_TIME, "");
                    memset(buff, 0, sizeof(buff));
                    strftime(buff, sizeof(buff) - 1, "%a", &tmp_tm);
                    utc_time = mktime(&tmp_tm) - timezone * 3600;
                    end_of_first_day = mktime(&tmp_tm) - timezone * 3600 + 24*3600+1;
                    fprintf(file_out,"    <sunrise> %li <sunirise>", utc_time);
                    xmlFree(temp_xml_string);
                    if (temp_xml_string){
                        temp_xml_string = xmlGetProp(child_node1, (const xmlChar*)"set");
                        setlocale(LC_TIME, "POSIX");
                        strptime((const char*)temp_xml_string, "%Y-%m-%dT%H:%M:%S", &tmp_tm);
                        setlocale(LC_TIME, "");
                        memset(buff, 0, sizeof(buff));
                        strftime(buff, sizeof(buff) - 1, "%a", &tmp_tm);
                        utc_time = mktime(&tmp_tm) - timezone * 3600;
                        fprintf(file_out,"    <sunset> %li <sunset>", utc_time);
                        xmlFree(temp_xml_string);
                    }
                    fprintf(file_out,"    </period>\n");
                }

            }

            /* Fill other days */
            if(!xmlStrcmp(cur_node->name, (const xmlChar *) "forecast" ) ){
                for(child_node = cur_node->children; child_node; child_node = child_node->next){
                   if (!xmlStrcmp(child_node->name, (const xmlChar *) "tabular" )){
                    for(child_node1 = child_node->children; child_node1; child_node1 = child_node1->next){
                        if(child_node1->type == XML_ELEMENT_NODE  &&
                                ( !xmlStrcmp(child_node1->name, (const xmlChar *)"time") ) ){
                            temp_xml_string = xmlGetProp(child_node1, (const xmlChar*)"period");
                            if (temp_xml_string){
                                if (!xmlStrcmp(temp_xml_string, (const xmlChar *)"0")){
                                    period = 0;
                                    count_day++;
                                }else if (!xmlStrcmp(temp_xml_string, (const xmlChar *)"1"))
                                         period = 1;
                                     else if (!xmlStrcmp(temp_xml_string, (const xmlChar *)"2"))
                                              period = 2;
                                          else if (!xmlStrcmp(temp_xml_string, (const xmlChar *)"3"))
                                                   period = 3;
                                               else
                                                   period = INT_MAX;
                        }

                        /* add day */
                        temp_xml_string = xmlGetProp(child_node1, (const xmlChar*)"from");
                        if (temp_xml_string){
                            setlocale(LC_TIME, "POSIX");
                            strptime((const char*)temp_xml_string, "%Y-%m-%dT%H:%M:%S", &tmp_tm);
                            setlocale(LC_TIME, "");
                            memset(buff, 0, sizeof(buff));
                            strftime(buff, sizeof(buff) - 1, "%a", &tmp_tm);
                            utc_time = mktime(&tmp_tm) - timezone * 3600;
                            /* increase past time for first forecast data */ 
                            if (first_day){
                                first_day = FALSE;
                                utc_time = utc_time - 12*3600;
                            }
                            fprintf(file_out,"    <period start=\"%li\"", utc_time);
                            xmlFree(temp_xml_string);
                            temp_xml_string = xmlGetProp(child_node1, (const xmlChar*)"to");
                            if (temp_xml_string){
                                setlocale(LC_TIME, "POSIX");
                                strptime((const char*)temp_xml_string, "%Y-%m-%dT%H:%M:%S", &tmp_tm);
                                setlocale(LC_TIME, "");
                                memset(buff, 0, sizeof(buff));
                                strftime(buff, sizeof(buff) - 1, "%a", &tmp_tm);
                                utc_time = mktime(&tmp_tm) - timezone * 3600;
                                fprintf(file_out," end=\"%li\">\n", utc_time); 
                                xmlFree(temp_xml_string);
                            }
                        }
                            for(child_node2 = child_node1->children; child_node2; child_node2 = child_node2->next){
                                if( child_node2->type == XML_ELEMENT_NODE){
                                    /* 24h hi temperature */
                                    if(!xmlStrcmp(child_node2->name, (const xmlChar *)"temperature")){
                                        temp_xml_string = xmlGetProp(child_node2, (const xmlChar*)"value");
                                        if (temp_xml_string)
			                                fprintf(file_out,"     <temperature>%s</temperature>\n", (char*)temp_xml_string); 
                                        xmlFree(temp_xml_string);
                                        continue;
                                    }  /* 24h icon */
                                    if(!xmlStrcmp(child_node2->name, (const xmlChar *)"symbol")){
                                        temp_xml_string = xmlGetProp(child_node2, (const xmlChar*)"number");
                                        memset(buff, 0, sizeof(buff));
                                        if (period == 0){
                                            snprintf(buff, sizeof(buff)-1, "night%s",(char*)temp_xml_string);
			                                fprintf(file_out,"     <icon>%s</icon>\n", hash_yrno_table_find(hash_for_icons, buff , FALSE)); 
                                        }
                                        if (period == 1){
                                            snprintf(buff, sizeof(buff)-1, "night%s",(char*)temp_xml_string);
			                                fprintf(file_out,"     <icon>%s</icon>\n", hash_yrno_table_find(hash_for_icons, buff , FALSE)); 
                                        }
                                        if (period == 2){
                                            snprintf(buff, sizeof(buff)-1, "day%s",(char*)temp_xml_string);
			                                fprintf(file_out,"     <icon>%s</icon>\n", hash_yrno_table_find(hash_for_icons, buff , FALSE)); 
                                        }
                                        if (period == 3){
                                            snprintf(buff, sizeof(buff)-1, "day%s",(char*)temp_xml_string);
			                                fprintf(file_out,"     <icon>%s</icon>\n", hash_yrno_table_find(hash_for_icons, buff , FALSE)); 
                                        }
                                        xmlFree(temp_xml_string);
                                        temp_xml_string = xmlGetProp(child_node2, (const xmlChar*)"name");
			                            fprintf(file_out,"     <description>%s</description>\n",
                                                         hash_yrno_table_find(hash_for_translate, 
                                                         (char*)temp_xml_string, FALSE));
                                        xmlFree(temp_xml_string);
                                        continue;
                                    }
                                    if(!xmlStrcmp(child_node2->name, (const xmlChar *)"pressure") ){
                                        temp_xml_string = xmlGetProp(child_node2, (const xmlChar*)"value");
			                            fprintf(file_out,"     <pressure>%s</pressure>\n",
                                                                   (char*)temp_xml_string);
                                        xmlFree(temp_xml_string);
                                        continue;
                                    }
                                    if(!xmlStrcmp(child_node2->name, (const xmlChar *)"precipitation") ){
                                        temp_xml_string = xmlGetProp(child_node2, (const xmlChar*)"value");
                                        fprintf(file_out,"     <precipitation>%s</precipitation>\n",
                                                                   (char*)temp_xml_string);
                                        xmlFree(temp_xml_string);
                                        continue;
                                    }
                                    if(!xmlStrcmp(child_node2->name, (const xmlChar *)"windSpeed") ){
                                        temp_xml_string = xmlGetProp(child_node2, (const xmlChar*)"mps");
                                        /* Normalize speed to km/h from m/s */
                                        /* fprintf(stderr, "Wind  speed    %s\n", temp_buffer); */
                                        speed = atoi((char*)temp_xml_string);
                                        /* speed = speed * 3600/1000; why??? */
                                        memset(buff, 0, sizeof(buff));
                                        snprintf(buff, sizeof(buff)-1, "%i", speed);
			                            fprintf(file_out,"     <wind_speed>%s</wind_speed>\n",  buff);
                                        xmlFree(temp_xml_string);
                                        continue;
                                    }
                                    if(!xmlStrcmp(child_node2->name, (const xmlChar *)"windDirection") ){
                                        temp_xml_string = xmlGetProp(child_node2, (const xmlChar*)"code");
			                            fprintf(file_out,"     <wind_direction>%s</wind_direction>\n",
                                                                                 (char*)temp_xml_string);
                                        xmlFree(temp_xml_string);
                                        continue;
                                    }



                                    if(!xmlStrcmp(child_node2->name, (const xmlChar *)"sunr")){
                                        temp_xml_string = xmlNodeGetContent(child_node2);
                                        g_hash_table_insert(day, "day_sunrise", g_strdup((char*)temp_xml_string));
                                        xmlFree(temp_xml_string);
                                        continue;
                                    }
                                    /* 24h sunset */
                                    if(!xmlStrcmp(child_node2->name, (const xmlChar *)"suns")){
                                        temp_xml_string = xmlNodeGetContent(child_node2);
                                        g_hash_table_insert(day, "day_sunset", g_strdup((char*)temp_xml_string));
                                        xmlFree(temp_xml_string);
                                        continue;
                                    }
                                }
                            }
                            fprintf(file_out,"    </period>\n");
                        }
                     }    
                   }
                }
            }
        }
    }
    g_hash_table_destroy(hash_for_translate);                                                                                                                                                   
    g_hash_table_destroy(hash_for_icons);
    fprintf(file_out,"</station>");
    fclose(file_out);

    return count_day;
}
/*******************************************************************************/
gint
parse_xml_detail_data(const gchar *station_id, xmlNode *root_node, GHashTable *data){
    xmlNode     *cur_node = NULL,
                *child_node = NULL,
                *child_node2 = NULL,
                *child_node3 = NULL;
    GHashTable  *detail = NULL,
                *hours_data = NULL;
    xmlChar     *temp_xml_string;
    gint        count_hour = 0;
    gchar       buff[256];
    struct tm   tmp_tm = {0};
    GSList      *hour_weather = NULL;
#ifdef DEBUGFUNCTIONCALL
    START_FUNCTION;
#endif
    for(cur_node = root_node->children; cur_node; cur_node = cur_node->next){
        if( cur_node->type == XML_ELEMENT_NODE ){
            if( !xmlStrcmp(cur_node->name, (const xmlChar *) "hbhf") ){
                hours_data = g_hash_table_new(g_str_hash, g_str_equal);
                for(child_node = cur_node->children; child_node; child_node = child_node->next){
                    /* last update */
                    if( child_node->type == XML_ELEMENT_NODE  &&
                            ( !xmlStrcmp(child_node->name, (const xmlChar *)"lsup") ) ){
                        temp_xml_string = xmlNodeGetContent(child_node);
                        g_hash_table_insert(hours_data, "last_update", g_strdup((char*)temp_xml_string));
                        xmlFree(temp_xml_string);
                        continue;
                    }
                    if( child_node->type == XML_ELEMENT_NODE  &&
                            (!xmlStrcmp(child_node->name, (const xmlChar *)"hour") )){
                        detail = g_hash_table_new(g_str_hash, g_str_equal);
                        /* Get an hour */
                        temp_xml_string = xmlGetProp(child_node, (const xmlChar *)"c");
                        /* prepare locale value for hour */
                        memset(buff, 0, sizeof(buff));
                        memcpy(buff, temp_xml_string, (strlen((char*)temp_xml_string)                                                                    > sizeof(buff)) ?
                                                                 (sizeof(buff) - 1) :
                                                   (strlen((char*)temp_xml_string)));
                        strptime(buff, "%H", &tmp_tm);
                        memset(buff, 0, sizeof(buff));
                        strftime(buff, sizeof(buff) - 1, "%H", &tmp_tm);
                        g_hash_table_insert(detail, "hours", g_strdup(buff));
                        xmlFree(temp_xml_string);
                        for(child_node2 = child_node->children; child_node2; child_node2 = child_node2->next){
                            if( child_node2->type == XML_ELEMENT_NODE){
                                /* hour temperature */
                               if(!xmlStrcmp(child_node2->name, (const xmlChar *)"tmp"))    {
                                    temp_xml_string = xmlNodeGetContent(child_node2);
                                    g_hash_table_insert(detail, "hour_temperature", g_strdup((char*)temp_xml_string));
                                    xmlFree(temp_xml_string);
                                    continue;
                               }
                               /* feels like for hour */
                               if(!xmlStrcmp(child_node2->name, (const xmlChar *)"flik")){
                                    temp_xml_string = xmlNodeGetContent(child_node2);
                                    g_hash_table_insert(detail, "hour_feels_like", g_strdup((char*)temp_xml_string));
                                    xmlFree(temp_xml_string);
                                    continue;
                               }
                               /* icon for hour */
                               if(!xmlStrcmp(child_node2->name, (const xmlChar *)"icon")){
                                    temp_xml_string = xmlNodeGetContent(child_node2);
                                    g_hash_table_insert(detail, "hour_icon", g_strdup((char*)temp_xml_string));
                                    xmlFree(temp_xml_string);
                                    continue;
                               }
                               /* title for hour */
                               if(!xmlStrcmp(child_node2->name, (const xmlChar *)"t")){
                                    temp_xml_string = xmlNodeGetContent(child_node2);
                                    g_hash_table_insert(detail, "hour_title", g_strdup((char*)temp_xml_string));
                                    xmlFree(temp_xml_string);
                                    continue;
                               }
                               /* wind data for hour */
                               if(!xmlStrcmp(child_node2->name, (const xmlChar *)"wind")){
                                    for(child_node3 = child_node2->children; child_node3 != NULL; child_node3 = child_node3->next){
                                        if( child_node3->type == XML_ELEMENT_NODE ){
                                            /* speed */
                                            if(!xmlStrcmp(child_node3->name, (const xmlChar *)"s")){
                                                temp_xml_string = xmlNodeGetContent(child_node3);
                                                g_hash_table_insert(detail, "hour_wind_speed", g_strdup((char*)temp_xml_string));
                                                xmlFree(temp_xml_string);
                                            }
                                            /* gust */
                                            if(!xmlStrcmp(child_node3->name, (const xmlChar *)"gust") ){
                                                temp_xml_string = xmlNodeGetContent(child_node3);
                                                g_hash_table_insert(detail, "hour_wind_gust", g_strdup((char*)temp_xml_string));
                                                xmlFree(temp_xml_string);
                                            }
                                            /* direction */
                                            if(!xmlStrcmp(child_node3->name, (const xmlChar *)"t")){
                                                temp_xml_string = xmlNodeGetContent(child_node3);
                                                g_hash_table_insert(detail, "hour_wind_direction", g_strdup((char*)temp_xml_string));
                                                xmlFree(temp_xml_string);
                                            }
                                        }
                                    }
                               }
                               /* hour humidity */
                               if(!xmlStrcmp(child_node2->name, (const xmlChar *)"hmid") ){
                                    temp_xml_string = xmlNodeGetContent(child_node2);
                                    g_hash_table_insert(detail, "hour_humidity", g_strdup((char*)temp_xml_string));
                                    xmlFree(temp_xml_string);
                                    continue;
                               }
                            }
                        }
                        if(detail){
                            hour_weather = g_slist_append(hour_weather, (gpointer)detail);
                            detail = NULL;
                            count_hour++;
                        }
                    }
                }
            }
        }
    }
    g_hash_table_insert(hours_data, "hours_data", (gpointer)hour_weather);
    g_hash_table_insert(data, "detail", (gpointer)hours_data);
    return count_hour;
}
/*******************************************************************************/
int
main(int argc, char *argv[]){
    int result; 
    if (argc < 3) {
        fprintf(stderr, "yrno <input_file> <output_file>\n");
        return -1;
    }
    result = convert_station_yrno_data(argv[1], argv[2], FALSE);
    fprintf(stderr, "\nresult = %d\n", result);
    return result;
}
