/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* this implements a GPS hardware library for the Android emulator.
 * the following code should be built as a shared library that will be
 * placed into /system/lib/hw/gps.goldfish.so
 *
 * it will be loaded by the code in hardware/libhardware/hardware.c
 * which is itself called from android_location_GpsLocationProvider.cpp
 */


#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <math.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <termios.h> 
#include <errno.h>

#define  LOG_TAG  "gps_ublox6M"
#include <cutils/log.h>
#include <cutils/sockets.h>
#include <hardware/gps.h>
#include <hardware/qemud.h>

/* the name of the qemud-controlled socket */
#define  QEMU_CHANNEL_NAME  "gps"

#define  GPS_DEBUG  0
#define Ublox_6M 1
#define LOGE  ALOGE

#if GPS_DEBUG
#  define  D(...)   LOGD(__VA_ARGS__)
#else
#  define  D(...)   ((void)0)
#endif

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   T O K E N I Z E R                     *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

typedef struct {
    const char*  p;
    const char*  end;
} Token;

#define  MAX_NMEA_TOKENS  16

typedef struct {
    int     count;
    Token   tokens[ MAX_NMEA_TOKENS ];
} NmeaTokenizer;

/*********************************************************************/
GpsStatus g_status;

static int
nmea_tokenizer_init( NmeaTokenizer*  t, const char*  p, const char*  end )
{
    int    count = 0;
    char*  q;

    // the initial '$' is optional
    if (p < end && p[0] == '$')
        p += 1;

    // remove trailing newline
    if (end > p && end[-1] == '\n') {
        end -= 1;
        if (end > p && end[-1] == '\r')
            end -= 1;
    }

    // get rid of checksum at the end of the sentecne
    if (end >= p+3 && end[-3] == '*') {
        end -= 3;
    }

    while (p < end) {
        const char*  q = p;

        q = memchr(p, ',', end-p);
        if (q == NULL)
            q = end;

        if (q >= p) {//
            if (count < MAX_NMEA_TOKENS) {
                t->tokens[count].p   = p;
                t->tokens[count].end = q;
                count += 1;
            }
        }
        if (q < end)
            q += 1;

        p = q;
    }

    t->count = count;
    return count;
}

static Token
nmea_tokenizer_get( NmeaTokenizer*  t, int  index )
{
    Token  tok;
    static const char*  dummy = "";

    if (index < 0 || index >= t->count) {
        tok.p = tok.end = dummy;
    } else
        tok = t->tokens[index];

    return tok;
}


static int
str2int( const char*  p, const char*  end )
{
    int   result = 0;
    int   len    = end - p;

    for ( ; len > 0; len--, p++ )
    {
        int  c;

        if (p >= end)
            goto Fail;

        c = *p - '0';
        if ((unsigned)c >= 10)
            goto Fail;

        result = result*10 + c;
    }
    return  result;

Fail:
    return -1;
}

static double
str2float( const char*  p, const char*  end )
{
    int   result = 0;
    int   len    = end - p;
    char  temp[16];

    if (len >= (int)sizeof(temp))
        return 0.;

    memcpy( temp, p, len );
    temp[len] = 0;
    return strtod( temp, NULL );
}

/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       N M E A   P A R S E R                           *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

#define  NMEA_MAX_SIZE  83

typedef struct {
    int     pos;
    int     overflow;
    int     utc_year;
    int     utc_mon;
    int     utc_day;
    int     utc_diff;
    GpsLocation  fix;
    //********************************
  	GpsSvStatus  sv_status;
   
    int     sv_status_changed; 
#ifdef Ublox_6M
    GpsCallbacks callback;
#else
    //*********************************
    gps_location_callback  callback;
#endif
	char    in[ NMEA_MAX_SIZE+1 ];
} NmeaReader;


static void
nmea_reader_update_utc_diff( NmeaReader*  r )
{
    time_t         now = time(NULL);
    struct tm      tm_local;
    struct tm      tm_utc;
    long           time_local, time_utc;

    gmtime_r( &now, &tm_utc );
    localtime_r( &now, &tm_local );

    time_local = tm_local.tm_sec +
                 60*(tm_local.tm_min +
                 60*(tm_local.tm_hour +
                 24*(tm_local.tm_yday +
                 365*tm_local.tm_year)));

    time_utc = tm_utc.tm_sec +
               60*(tm_utc.tm_min +
               60*(tm_utc.tm_hour +
               24*(tm_utc.tm_yday +
               365*tm_utc.tm_year)));

    r->utc_diff = time_local - time_utc;
}


static void
nmea_reader_init( NmeaReader*  r )
{
    memset( r, 0, sizeof(*r) );

    r->pos      = 0;
    r->overflow = 0;
    r->utc_year = -1;
    r->utc_mon  = -1;
    r->utc_day  = -1;
#ifdef Ublox_6M
    r->callback.sv_status_cb = NULL;
    r->callback.nmea_cb = NULL;
    r->callback.location_cb = NULL;
    r->callback.status_cb = NULL;
#else
    r->callback = NULL;
#endif
    r->fix.size = sizeof(r->fix);

    nmea_reader_update_utc_diff( r );
}


static void
nmea_reader_set_callback( NmeaReader*  r, gps_location_callback  cb )
{
#ifdef Ublox_6M
    r->callback.location_cb  = cb;
#else
	 r->callback  = cb;
#endif
	if (cb != NULL && r->fix.flags != 0) {
	LOGE("%s: sending latest fix to new callback", __FUNCTION__);
#ifdef Ublox_6M
	r->callback.location_cb( &r->fix );/////////////////////////////////////////////
#else
	r->fix.flags = 0;
#endif
    }
}


static int
nmea_reader_update_time( NmeaReader*  r, Token  tok )
{
    int        hour, minute;
    double     seconds;
    struct tm  tm;
    time_t     fix_time;

    if (tok.p + 6 > tok.end)
        return -1;

    if (r->utc_year < 0) {
        // no date yet, get current one
        time_t  now = time(NULL);
        gmtime_r( &now, &tm );
        r->utc_year = tm.tm_year + 1900;
        r->utc_mon  = tm.tm_mon + 1;
        r->utc_day  = tm.tm_mday;
    }

    hour    = str2int(tok.p,   tok.p+2);
    minute  = str2int(tok.p+2, tok.p+4);
    seconds = str2float(tok.p+4, tok.end);

    tm.tm_hour  = hour;
    tm.tm_min   = minute;
    tm.tm_sec   = (int) seconds;
    tm.tm_year  = r->utc_year - 1900;
    tm.tm_mon   = r->utc_mon - 1;
    tm.tm_mday  = r->utc_day;
    tm.tm_isdst = -1;

    fix_time = mktime( &tm ) + r->utc_diff;
    r->fix.timestamp = (long long)fix_time * 1000;
    return 0;
}

static int
nmea_reader_update_date( NmeaReader*  r, Token  date, Token  time )
{
    Token  tok = date;
    int    day, mon, year;

    if (tok.p + 6 != tok.end) {
        LOGE("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    day  = str2int(tok.p, tok.p+2);
    mon  = str2int(tok.p+2, tok.p+4);
    year = str2int(tok.p+4, tok.p+6) + 2000;

    if ((day|mon|year) < 0) {
        LOGE("date not properly formatted: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }

    r->utc_year  = year;
    r->utc_mon   = mon;
    r->utc_day   = day;

    return nmea_reader_update_time( r, time );
}


static double
convert_from_hhmm( Token  tok )
{
    double  val     = str2float(tok.p, tok.end);
    int     degrees = (int)(floor(val) / 100);
    double  minutes = val - degrees*100.;
    double  dcoord  = degrees + minutes / 60.0;
    return dcoord;
}


static int
nmea_reader_update_latlong( NmeaReader*  r,
                            Token        latitude,
                            char         latitudeHemi,
                            Token        longitude,
                            char         longitudeHemi )
{
    double   lat, lon;
    Token    tok;

    tok = latitude;
    if (tok.p + 6 > tok.end) {
        LOGE("latitude is too short: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    lat = convert_from_hhmm(tok);
    if (latitudeHemi == 'S')
        lat = -lat;

    tok = longitude;
    if (tok.p + 6 > tok.end) {
        LOGE("longitude is too short: '%.*s'", tok.end-tok.p, tok.p);
        return -1;
    }
    lon = convert_from_hhmm(tok);
    if (longitudeHemi == 'W')
        lon = -lon;

    r->fix.flags    |= GPS_LOCATION_HAS_LAT_LONG;
    r->fix.latitude  = lat;
    r->fix.longitude = lon;
    return 0;
}


static int
nmea_reader_update_altitude( NmeaReader*  r,
                             Token        altitude,
                             Token        units )
{
    double  alt;
    Token   tok = altitude;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_ALTITUDE;
    r->fix.altitude = str2float(tok.p, tok.end);
    return 0;
}


static int
nmea_reader_update_bearing( NmeaReader*  r,
                            Token        bearing )
{
    double  alt;
    Token   tok = bearing;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_BEARING;
    r->fix.bearing  = str2float(tok.p, tok.end);
    return 0;
}


static int
nmea_reader_update_speed( NmeaReader*  r,
                          Token        speed )
{
    double  alt;
    Token   tok = speed;

    if (tok.p >= tok.end)
        return -1;

    r->fix.flags   |= GPS_LOCATION_HAS_SPEED;
    r->fix.speed    = str2float(tok.p, tok.end);
    return 0;
}
static int nmea_reader_update_accuracy(NmeaReader * r,  Token accuracy)
{
	double acc;
	Token  tok = accuracy;

	if(tok.p >= tok.end)
		return -1;

	r->fix.accuracy = str2float(tok.p, tok.end);

	if(r->fix.accuracy == 99.99){
	return 0;
	}

	r->fix.flags |= GPS_LOCATION_HAS_ACCURACY;
	return 0;
}

/* this is the state of our connection to the qemu_gpsd daemon */
typedef struct {
    int                     init;
    int                     fd;
    GpsCallbacks            callbacks;
    pthread_t               thread;
    int                     control[2];
} GpsState;

static GpsState  _gps_state[1];

static void
nmea_reader_parse( NmeaReader*  r )
{
   /* we received a complete sentence, now parse it to generate
    * a new GPS fix...
    */
    NmeaTokenizer  tzer[1];
    Token          tok;

    LOGE("Received: '%.*s'", r->pos, r->in);
    if (r->pos < 9) {
        LOGE("Too short. discarded.");
        return;
    }

    nmea_tokenizer_init(tzer, r->in, r->in + r->pos);
#if GPS_DEBUG
    {
        int  n;
        LOGE("Found %d tokens", tzer->count);
        for (n = 0; n < tzer->count; n++) {
            Token  tok = nmea_tokenizer_get(tzer,n);
            LOGE("%2d: '%.*s'", n, tok.end-tok.p, tok.p);
        }
    }
#endif

    tok = nmea_tokenizer_get(tzer, 0);
    if (tok.p + 5 > tok.end) {
        LOGE("sentence id '%.*s' too short, ignored.", tok.end-tok.p, tok.p);
        return;
    }

    // ignore first two characters.
    tok.p += 2;
    if ( !memcmp(tok.p, "GGA", 3) ) {
        // GPS fix
        Token  tok_time          = nmea_tokenizer_get(tzer,1);
        Token  tok_latitude      = nmea_tokenizer_get(tzer,2);
        Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,3);
        Token  tok_longitude     = nmea_tokenizer_get(tzer,4);
        Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,5);
        Token  tok_altitude      = nmea_tokenizer_get(tzer,9);
        Token  tok_altitudeUnits = nmea_tokenizer_get(tzer,10);

        nmea_reader_update_time(r, tok_time);
        nmea_reader_update_latlong(r, tok_latitude,
                                      tok_latitudeHemi.p[0],
                                      tok_longitude,
                                      tok_longitudeHemi.p[0]);
        nmea_reader_update_altitude(r, tok_altitude, tok_altitudeUnits);

    } else if ( !memcmp(tok.p, "GSA", 3) ) {
        // do something ?
       
 #ifdef Ublox_6M     
		{  
			LOGE("may%s,%d,%s,gsa\n",__FILE__,__LINE__,__FUNCTION__);  
			Token tok_fixStatus = nmea_tokenizer_get(tzer, 2);  
			int i;  
	  
			if (tok_fixStatus.p[0] != '\0' && tok_fixStatus.p[0] != '1') {  
				Token tok_accuracy = nmea_tokenizer_get(tzer, 15);//position dilution of precision dop   
				nmea_reader_update_accuracy(r, tok_accuracy);  
				r->sv_status.used_in_fix_mask = 0ul;  
				LOGE("\n");  
				for (i = 3; i <= 14; ++i){  
					Token tok_prn = nmea_tokenizer_get(tzer, i);  
					int prn = str2int(tok_prn.p, tok_prn.end);  
					LOGE("gsa,prn=%d,",prn);  
					if (prn > 0){  
						r->sv_status.used_in_fix_mask |= (1ul << ( prn-1));  
						r->sv_status_changed = 1;       
				   }  
			   }LOGE("\n");  
				LOGE("%s: fix mask is %x", __FUNCTION__, r->sv_status.used_in_fix_mask);  
				//D(" [log hit][%s:%d] fix.flags=0x%x ", __FUNCTION__, __LINE__, r->fix.flags);    
		   }  
		   LOGE(" [log hit][%s:%d] fix.flags=0x%x ", __FUNCTION__, __LINE__, r->fix.flags);   
		}  
#endif   
    }
    
#ifdef Ublox_6M   
    else if ( !memcmp(tok.p, "GSV", 3) ) {  
		LOGE("may%s,%d,%s,gsV\n",__FILE__,__LINE__,__FUNCTION__);  
		Token tok_noSatellites = nmea_tokenizer_get(tzer, 3);  
		int noSatellites = str2int(tok_noSatellites.p, tok_noSatellites.end);  
		LOGE("%d,inview=%d,\n",__LINE__,noSatellites);    
		if (noSatellites > 0) {  
		Token tok_noSentences = nmea_tokenizer_get(tzer, 1);  
		Token tok_sentence     = nmea_tokenizer_get(tzer, 2);  

		int sentence = str2int(tok_sentence.p, tok_sentence.end);  
		int totalSentences = str2int(tok_noSentences.p, tok_noSentences.end);  
		LOGE("%d,gsv_index=%d,gsv_total=%d\n",__LINE__,sentence,totalSentences);    
		int curr;  
		int i;  

		if (sentence == 1) {  
			LOGE("msg_index=%d\n",sentence);  
			//  r->sv_status_changed = 0;   
			r->sv_status.num_svs = 0;  
			r->sv_status.ephemeris_mask=0ul;  
			r->sv_status.almanac_mask=0ul;  
		}  

		curr = r->sv_status.num_svs;  
		i = 0;  
		while (i < 4 && r->sv_status.num_svs < noSatellites){  
			Token    tok_prn = nmea_tokenizer_get(tzer, i * 4 + 4);  
			Token    tok_elevation = nmea_tokenizer_get(tzer, i * 4 + 5);  
			Token    tok_azimuth = nmea_tokenizer_get(tzer, i * 4 + 6);  
			Token    tok_snr = nmea_tokenizer_get(tzer, i * 4 + 7);  

			r->sv_status.sv_list[curr].prn = str2int(tok_prn.p, tok_prn.end);  
			r->sv_status.sv_list[curr].elevation = str2float(tok_elevation.p, tok_elevation.end);  
			r->sv_status.sv_list[curr].azimuth = str2float(tok_azimuth.p, tok_azimuth.end);  
			r->sv_status.sv_list[curr].snr = str2float(tok_snr.p, tok_snr.end);  
			r->sv_status.ephemeris_mask|=(1ul << (r->sv_status.sv_list[curr].prn-1));  
			r->sv_status.almanac_mask|=(1ul << (r->sv_status.sv_list[curr].prn-1));           
			r->sv_status.num_svs += 1;  
			LOGE("**********curr=%d\n",curr);  

			LOGE("%d,prn=%d:snr=%f\n",__LINE__,r->sv_status.sv_list[curr].prn,r->sv_status.sv_list[curr].snr);  
			curr += 1;  

			i += 1;  
		}  

		if (sentence == totalSentences) {  
			LOGE("msg=%d,msgindex=%d",totalSentences,sentence);  
#ifdef Ublox_6M   
			r->callback.sv_status_cb=_gps_state->callbacks.sv_status_cb;  

			if (r->sv_status_changed !=0) {  
				if (r->callback.sv_status_cb) {  
#if GPS_DEBUG   
				LOGE("%d,SV_STATSU,change=%d\n",__LINE__,r->sv_status_changed);  
				int nums=r->sv_status.num_svs;  
				LOGE("num_svs=%d,emask=%x,amask=%x,inusemask=%x\n",r->sv_status.num_svs,r->sv_status.ephemeris_mask,r->sv_status.almanac_mask,r->sv_status.used_in_fix_mask);  
				LOGE("************88\n");        
				while(nums)  
				{  
				nums--;  
				LOGE("prn=%d:snr=%f\n",r->sv_status.sv_list[nums].prn,r->sv_status.sv_list[nums].snr);  

				}LOGE("************88\n");  
#endif   
				r->callback.sv_status_cb( &(r->sv_status) );  
				r->sv_status_changed = 0;  
				}else {  
				LOGE("no callback, keeping status data until needed !");  
				}  
			}  
#endif   
			}  
			LOGE("%s: GSV message with total satellites %d", __FUNCTION__, noSatellites);   
		}                 
    }  
#endif

	else if ( !memcmp(tok.p, "GLL", 3) ) {  
		    Token tok_fixstaus = nmea_tokenizer_get(tzer,6);  
		    if (tok_fixstaus.p[0] == 'A') {  
		     Token tok_latitude = nmea_tokenizer_get(tzer,1);  
		     Token tok_latitudeHemi = nmea_tokenizer_get(tzer,2);  
		     Token tok_longitude = nmea_tokenizer_get(tzer,3);  
		     Token tok_longitudeHemi = nmea_tokenizer_get(tzer,4);  
		     Token tok_time = nmea_tokenizer_get(tzer,5);  
		     nmea_reader_update_time(r, tok_time);  
		     nmea_reader_update_latlong(r, tok_latitude, tok_latitudeHemi.p[0], tok_longitude, tok_longitudeHemi.p[0]);  
		    }  
    }  

	else if ( !memcmp(tok.p, "RMC", 3) ) {
#ifndef Ublox_6M
		Token  tok_time          = nmea_tokenizer_get(tzer,1);
		Token  tok_fixStatus     = nmea_tokenizer_get(tzer,2);
		Token  tok_latitude      = nmea_tokenizer_get(tzer,3);
		Token  tok_latitudeHemi  = nmea_tokenizer_get(tzer,4);
		Token  tok_longitude     = nmea_tokenizer_get(tzer,5);
		Token  tok_longitudeHemi = nmea_tokenizer_get(tzer,6);
		Token  tok_speed         = nmea_tokenizer_get(tzer,7);
		Token  tok_bearing       = nmea_tokenizer_get(tzer,8);
		Token  tok_date          = nmea_tokenizer_get(tzer,9);

        LOGE("in RMC, fixStatus=%c", tok_fixStatus.p[0]);
        if (tok_fixStatus.p[0] == 'A')
        {
            nmea_reader_update_date( r, tok_date, tok_time );
            nmea_reader_update_latlong( r, tok_latitude,
                                           tok_latitudeHemi.p[0],
                                           tok_longitude,
                                           tok_longitudeHemi.p[0] );
            nmea_reader_update_bearing( r, tok_bearing );
            nmea_reader_update_speed  ( r, tok_speed );
        }
#else
        Token tok_time = nmea_tokenizer_get(tzer,1);  
        Token tok_fixStatus = nmea_tokenizer_get(tzer,2);  
        Token tok_latitude = nmea_tokenizer_get(tzer,3);  
        Token tok_latitudeHemi = nmea_tokenizer_get(tzer,4);  
        Token tok_longitude = nmea_tokenizer_get(tzer,5);  
        Token tok_longitudeHemi = nmea_tokenizer_get(tzer,6);  
        Token tok_speed = nmea_tokenizer_get(tzer,7);  
        Token tok_bearing = nmea_tokenizer_get(tzer,8);  
        Token tok_date = nmea_tokenizer_get(tzer,9);  
  
        LOGE("in RMC, fixStatus=%c", tok_fixStatus.p[0]);  
		if (tok_fixStatus.p[0] == 'A')  
        {  
            nmea_reader_update_date( r, tok_date, tok_time );  
  
            nmea_reader_update_latlong( r, tok_latitude,  
                                           tok_latitudeHemi.p[0],  
                                           tok_longitude,  
                                           tok_longitudeHemi.p[0] );  
  
            nmea_reader_update_bearing( r, tok_bearing );  
            nmea_reader_update_speed ( r, tok_speed );
              
#ifdef Ublox_6M   
		r->callback.location_cb=_gps_state->callbacks.location_cb;  
		r->callback.nmea_cb=_gps_state->callbacks.nmea_cb;  
		r->callback.status_cb=_gps_state->callbacks.status_cb;  
		if (r->callback.status_cb) {  
		LOGE("report,status,flags=%d\n",r->fix.flags);  
		            r->callback.status_cb( (struct GpsStatus *)&(r->fix.flags) );  
		        }  
		     if (r->callback.location_cb) {  
		LOGE("location_cb report:r->fix.flags=%d,r->latitude=%f,r->longitude=%f,r->altitude=%f,r->speed=%f,r->bearing=%f,r->accuracy=%f\n",r->fix.flags,r->fix.latitude,r->fix.longitude,r->fix.altitude,r->fix.speed,r->fix.bearing,r->fix.accuracy);  
		            r->callback.location_cb( &r->fix );  
		//LOGE("%d,cc=%d",__LINE__,cc);  
		        r->fix.flags = 0;  
		          
		        }  
		if (r->callback.nmea_cb) {  
		LOGE("report,timestamp=%llx,%llu\n",r->fix.timestamp,r->fix.timestamp);  
		            r->callback.nmea_cb( r->fix.timestamp,r->in,r->pos );           
        }  
#else   
		r->callback=_gps_state.callbacks->location_cb;  
		//r->callback.nmea_cb=_gps_state->callbacks.nmea_cb;   
		        if (r->callback) {D("if2 (r->callback.location_cb)\n");  
		             r->callback( &r->fix );  
		             r->fix.flags = 0;  
         }  
#endif   
        }  
#endif
    }  

	else 
    {
        tok.p -= 2;
        D("unknown sentence '%.*s", tok.end-tok.p, tok.p);
    }
    if (r->fix.flags != 0) {
#if GPS_DEBUG
        char   temp[256];
        char*  p   = temp;
        char*  end = p + sizeof(temp);
        struct tm   utc;

        p += snprintf( p, end-p, "sending fix" );
        if (r->fix.flags & GPS_LOCATION_HAS_LAT_LONG) {
            p += snprintf(p, end-p, " lat=%g lon=%g", r->fix.latitude, r->fix.longitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ALTITUDE) {
            p += snprintf(p, end-p, " altitude=%g", r->fix.altitude);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_SPEED) {
            p += snprintf(p, end-p, " speed=%g", r->fix.speed);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_BEARING) {
            p += snprintf(p, end-p, " bearing=%g", r->fix.bearing);
        }
        if (r->fix.flags & GPS_LOCATION_HAS_ACCURACY) {
            p += snprintf(p,end-p, " accuracy=%g", r->fix.accuracy);
        }
        gmtime_r( (time_t*) &r->fix.timestamp, &utc );
        p += snprintf(p, end-p, " time=%s", asctime( &utc ) );
        D(temp);
#endif
       
#ifndef Ublox_6M
    if (r->callback ) {
            r->callback( &r->fix );
            r->fix.flags = 0;
        }
        else {
            D("no callback, keeping data until needed !");
        }
#endif
    }
}


static void
nmea_reader_addc( NmeaReader*  r, int  c )
{
    if (r->overflow) {
        r->overflow = (c != '\n');
        return;
    }

    if (r->pos >= (int) sizeof(r->in)-1 ) {
        r->overflow = 1;
        r->pos      = 0;
        return;
    }

    r->in[r->pos] = (char)c;
    r->pos       += 1;

    if (c == '\n') {
        nmea_reader_parse( r );
        r->pos = 0;
    }
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       C O N N E C T I O N   S T A T E                 *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/

/* commands sent to the gps thread */
enum {
    CMD_QUIT  = 0,
    CMD_START = 1,
    CMD_STOP  = 2
};

static void
gps_state_done( GpsState*  s )
{
    // tell the thread to quit, and wait for it
    char   cmd = CMD_QUIT;
    void*  dummy;
    write( s->control[0], &cmd, 1 );
    pthread_join(s->thread, &dummy);

    // close the control socket pair
    close( s->control[0] ); s->control[0] = -1;
    close( s->control[1] ); s->control[1] = -1;

    // close connection to the QEMU GPS daemon
    close( s->fd ); s->fd = -1;
    s->init = 0;
}


static void
gps_state_start( GpsState*  s )
{
    char  cmd = CMD_START;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        D("%s: could not send CMD_START command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}


static void
gps_state_stop( GpsState*  s )
{
    char  cmd = CMD_STOP;
    int   ret;

    do { ret=write( s->control[0], &cmd, 1 ); }
    while (ret < 0 && errno == EINTR);

    if (ret != 1)
        D("%s: could not send CMD_STOP command: ret=%d: %s",
          __FUNCTION__, ret, strerror(errno));
}


static int
epoll_register( int  epoll_fd, int  fd )
{
    struct epoll_event  ev;
    int                 ret, flags;

    /* important: make the fd non-blocking */
    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_ADD, fd, &ev );
    } while (ret < 0 && errno == EINTR);
    return ret;
}


static int
epoll_deregister( int  epoll_fd, int  fd )
{
    int  ret;
    do {
        ret = epoll_ctl( epoll_fd, EPOLL_CTL_DEL, fd, NULL );
    } while (ret < 0 && errno == EINTR);
    return ret;
}

/* this is the main thread, it waits for commands from gps_state_start/stop and,
 * when started, messages from the QEMU GPS daemon. these are simple NMEA sentences
 * that must be parsed to be converted into GPS fixes sent to the framework
 */
static void*
gps_state_thread( void*  arg )
{
    GpsState*   state = (GpsState*) arg;
    NmeaReader  reader[1];
    int         epoll_fd   = epoll_create(2);
    int         started    = 0;
    int         gps_fd     = state->fd;
    int         control_fd = state->control[1];

    nmea_reader_init( reader );

    // register control file descriptors for polling
    epoll_register( epoll_fd, control_fd );
    epoll_register( epoll_fd, gps_fd );

    LOGE("gps thread running");

    // now loop
    for (;;) {
        struct epoll_event   events[2];
        int                  ne, nevents;

        nevents = epoll_wait( epoll_fd, events, 2, -1 );
        if (nevents < 0) {
            if (errno != EINTR)
                LOGE("epoll_wait() unexpected error: %s", strerror(errno));
            continue;
        }
        LOGE("gps thread received %d events", nevents);
        for (ne = 0; ne < nevents; ne++) {
            if ((events[ne].events & (EPOLLERR|EPOLLHUP)) != 0) {
                LOGE("EPOLLERR or EPOLLHUP after epoll_wait() !?");
                goto Exit;
            }
            if ((events[ne].events & EPOLLIN) != 0) {
                int  fd = events[ne].data.fd;

                if (fd == control_fd)
                {
                    char  cmd = 255;
                    int   ret;
                    LOGE("gps control fd event");
                    do {
                        ret = read( fd, &cmd, 1 );
                    } while (ret < 0 && errno == EINTR);

                    if (cmd == CMD_QUIT) {
                        LOGE("gps thread quitting on demand");
                        goto Exit;
                    }
                    else if (cmd == CMD_START) {
                        if (!started) {
                            LOGE("gps thread starting  location_cb=%p", state->callbacks.location_cb);
                            started = 1;
//******************************************************************
				g_status.status=GPS_STATUS_SESSION_BEGIN;//¿ªÊ¼µ¼º½
				state->callbacks.status_cb(&g_status); 
//******************************************************************	
                            nmea_reader_set_callback( reader, state->callbacks.location_cb );
                        }
                    }
                    else if (cmd == CMD_STOP) {
                        if (started) {
                            LOGE("gps thread stopping");
                            started = 0;
//********************************************************************
				g_status.status=GPS_STATUS_SESSION_END; //Í£Ö¹
				state->callbacks.status_cb(&g_status); 
//********************************************************************
                            nmea_reader_set_callback( reader, NULL );
                        }
                    }
                }
                else if (fd == gps_fd)
                {
                    char  buff[32];
                    LOGE("gps fd event");
                    for (;;) {
                        int  nn, ret;

                        ret = read( fd, buff, sizeof(buff) );
                        if (ret < 0) {
                            if (errno == EINTR)
                                continue;
                            if (errno != EWOULDBLOCK)
                                LOGE("error while reading from gps daemon socket: %s:", strerror(errno));
                            break;
                        }
                        LOGE("received %d bytes: %.*s", ret, ret, buff);
                        for (nn = 0; nn < ret; nn++)
                            nmea_reader_addc( reader, buff[nn] );
                    }
                    LOGE("gps fd event end");
                }
                else
                {
                    LOGE("epoll_wait() returned unkown fd %d ?", fd);
                }
            }
        }
    }
Exit:
    return NULL;
}


static void
gps_state_init( GpsState*  state )
{
    state->init       = 1;
    state->control[0] = -1;
    state->control[1] = -1;
    state->fd         = -1;
	int err;
	struct termios gps_termios;

	state->fd= open("/dev/ttyS7",O_RDWR|O_NOCTTY|O_NDELAY);//?a¨¤?¨®?¦Ì?¨º?UART1
	if(state->fd < 0){
		LOGE("open port /dev/ttyS7 ERROR..state->fd=%d\n",state->fd); 
		return;
	}else
		LOGE("open port:/dev/ttyS7 succceed..state->fd=%d\n",state->fd);

	if(fcntl(state->fd,F_SETFL,0)<0)
		LOGE("fcntl F_SETFL\n");
	
	tcflush(state->fd, TCIOFLUSH);//¨°???¨º?????¡ä??¨²¦Ì?2?¨ºy
	if ((err = tcgetattr(state->fd, &gps_termios)) != 0)
	{
		LOGE("tcgetattr(%d) = %d,errno %d\r\n",state->fd,err,errno);
		close(state->fd);
	}

	gps_termios.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	gps_termios.c_oflag &= ~OPOST;
	gps_termios.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	gps_termios.c_cflag &= ~(CSIZE|PARENB);
	gps_termios.c_cflag |= CS8;
	gps_termios.c_cflag &= ~CRTSCTS;//no flow control

	tcsetattr(state->fd, TCSANOW, &gps_termios);
	tcflush(state->fd, TCIOFLUSH);
	tcsetattr(state->fd, TCSANOW, &gps_termios);
	tcflush(state->fd, TCIOFLUSH);
	tcflush(state->fd, TCIOFLUSH);

    if (cfsetispeed(&gps_termios,B9600))//??¨¬?¦Ì?2???GPS??¦Ì¦Ì
	{
        LOGE("cfsetispeed.. errno..\r\n");
        close(state->fd);
        //return(-1);
	}
	// Set the output baud rates in the termios.
	if (cfsetospeed(&gps_termios,B9600))
	{
		LOGE("cfsetispeed.. errno..\r\n");
		close(state->fd);
		//return(-1);
	}

    tcsetattr(state->fd,TCSANOW,&gps_termios);
    LOGE("Port setup finished..\n");

	if (state->fd < 0) {
		LOGE("no gps emulation detected");
		return;
	}

    if ( socketpair( AF_LOCAL, SOCK_STREAM, 0, state->control ) < 0 ) {
        LOGE("could not create thread control socket pair: %s", strerror(errno));
        goto Fail;
    }
	
	state->thread=state->callbacks.create_thread_cb("gps_state_thread",gps_state_thread,state);
	
    LOGE("gps state initialized");
    return;

Fail:
    gps_state_done( state );
}


/*****************************************************************/
/*****************************************************************/
/*****                                                       *****/
/*****       I N T E R F A C E                               *****/
/*****                                                       *****/
/*****************************************************************/
/*****************************************************************/


static int
qemu_gps_init(GpsCallbacks* callbacks)
{
    GpsState*  s = _gps_state;
	
	s->callbacks = *callbacks; //¡Á¡é2¨¢??¦Ì¡Âo¡¥¨ºy,JNI¡ä???¨¤¡ä¦Ì???¦Ì¡Âo¡¥¨ºy
    g_status.status=GPS_STATUS_ENGINE_ON;//¨¦¨¨??¡Á¡ä¨¬? ¨ª¡§¦Ì?¦Ì??1???a¨º?¦Ì?o?
    s->callbacks.status_cb(&g_status);

    if (!s->init)
        gps_state_init(s);

    if (s->fd < 0)
        return -1;

    return 0;
}

static void
qemu_gps_cleanup(void)
{
    GpsState*  s = _gps_state;

    if (s->init)
        gps_state_done(s);
}


static int
qemu_gps_start()
{
    GpsState*  s = _gps_state;

    if (!s->init) {
        D("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    D("%s: called", __FUNCTION__);
    gps_state_start(s);
    return 0;
}


static int
qemu_gps_stop()
{
    GpsState*  s = _gps_state;

    if (!s->init) {
        D("%s: called with uninitialized state !!", __FUNCTION__);
        return -1;
    }

    D("%s: called", __FUNCTION__);
    gps_state_stop(s);
    return 0;
}


static int
qemu_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    return 0;
}

static int
qemu_gps_inject_location(double latitude, double longitude, float accuracy)
{
    return 0;
}

static void
qemu_gps_delete_aiding_data(GpsAidingData flags)
{
}

static int qemu_gps_set_position_mode(GpsPositionMode mode, int fix_frequency)
{
    // FIXME - support fix_frequency
    return 0;
}

static const void*
qemu_gps_get_extension(const char* name)
{
    // no extensions supported
    return NULL;
}

static const GpsInterface  qemuGpsInterface = {
    sizeof(GpsInterface),
    qemu_gps_init,
    qemu_gps_start,
    qemu_gps_stop,
    qemu_gps_cleanup,
    qemu_gps_inject_time,
    qemu_gps_inject_location,
    qemu_gps_delete_aiding_data,
    qemu_gps_set_position_mode,
    qemu_gps_get_extension,
};

const GpsInterface* gps__get_gps_interface(struct gps_device_t* dev)
{
    return &qemuGpsInterface;
}

static int open_gps(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    struct gps_device_t *dev = malloc(sizeof(struct gps_device_t));
    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
//    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->get_gps_interface = gps__get_gps_interface;

    *device = (struct hw_device_t*)dev;
    return 0;
}


static struct hw_module_methods_t gps_module_methods = {
    .open = open_gps
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = GPS_HARDWARE_MODULE_ID,
    .name = "Goldfish GPS Module",
    .author = "The Android Open Source Project",
    .methods = &gps_module_methods,
};
