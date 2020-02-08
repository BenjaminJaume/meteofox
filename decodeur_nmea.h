#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define CHECK_OK 0x00 //_EMPTY dans le .h d'origine
#define NMEA_IIMWV 0x01
#define NMEA_IIMWV_STR "$IIMWV"
#define NMEA_WIXDR 0x02
#define NMEA_WIXDR_STR "$WIXDR"
#define NMEA_PLCJE 0x03
#define NMEA_PLCJE_STR "$PLCJE"
#define NMEA_UNKNOWN 0x00
#define _COMPLETED 0x04

#define NMEA_CHECKSUM_ERR 0x80
#define NMEA_MESSAGE_ERR 0xC0

struct IIMWV {
	// trame type: "$IIMWV,136.0,R,004.80,N,A *05"

	double angle;	// Angle du vent de 000.0° à 359.0°
	char vent_apparent;		// Vent apparent
	double vitesse;		// vitesse du vent
	char unite_vitesse;		// unité de la vitesse du vent ( N = Noeuds, M = m/s, K = km/h )
	char status;	// Status de CV7SF A = Correct V = Alarme
};

typedef struct IIMWV iimwv_t;

struct WIXDR {
	// trame type: "$WIXDR,C,007,C,,U,4.1,V,,*63"

	double temperature; // température du vent
	char unite_temperature; // unite de la température
	double tension_charge; // tension de charge du super-condensateur
};

typedef struct WIXDR wixdr_t;

struct PLCJE{
	// trame type: "$PLCJE,AAAA,BBB,CCC"

	char caractere0_identifiant_capteur;
	char caractere1_identifiant_capteur;
	char caractere2_identifiant_capteur;
	char caractere3_identifiant_capteur;
};

typedef struct PLCJE plcje_t;

uint8_t nmea_get_message_type(const char *);
uint8_t nmea_valid_checksum(const char *);

void nmea_parse_iimwv(char *, iimwv_t *);
void nmea_parse_wixdr(char *, wixdr_t *);
void nmea_parse_plcje(char *, plcje_t *);