#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <SoftwareSerial.h>
#include <decodeur_nmea\decodeur_nmea.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "SFE_BMP180\SFE_BMP180.h"

#define anemoRX 2// Broche 2 Arduino en reception
#define	anemoTX 3// Broche 3 Arduino en transmission

//BMP180_ADDR	0x77 // Définition de l'adresse i²c du module de pression pin 
					 // A4 = SDA pin A5 = SCL
					// Déjà déclaré dans SFE_BMP180.h
SoftwareSerial SerialAnemometre(anemoRX, anemoTX); // RX,TX

char carac_dispo = 0;
char carac_recu = 0;
int caractere = 0;
char phrase[100]; // Stockage reception NMEA
int i_nmea = 0; // indice de navigation tableau NMEA phrase

iimwv_t reception_phrase_iimwv;
wixdr_t reception_phrase_wixdr;
plcje_t reception_phrase_plcje;

SFE_BMP180 bmp180;

double vent_minimum = 0;
double vent_maximum = 0;
double total_vent = 0;
double vent_moyen = 0;
int nombre_valeurs = 0;

int premiere_affectation = 0;

unsigned char trame[10];

unsigned long temps; //variable contenant le temps d'execution du programme en millisecondes

void setup()
{
	SerialAnemometre.begin(4800);
	Serial.begin(9600);
	pinMode(anemoRX, INPUT);
	pinMode(anemoTX, OUTPUT);
	
	if (bmp180.begin()) // initialise BMP180 avant utilisation
	{
		Serial.println("BMP180 init success");
	}
	else
	{
		Serial.println("Erreur initialisation pression");
	}

	temps = millis();
}

void loop()
{
	char etat;
	double T, P;
	//--------------Partie Capteur de pression--------------//

	etat = bmp180.startTemperature(); // Ordonne au BMP180 de démarrer la mesure de la pression et retourne le temps d'attente dans etat, 0 si echec
	if (etat != 0)
	{
		delay(etat);	// Attends le temps de la mesure

		etat = bmp180.getTemperature(T); // Recupere la temperature mesuree depuis le start précédent.
		if (etat != 0)	// Test si la mesure s'est bien deroulée 
		{
			Serial.print("Temperature : ");
			Serial.print(T,2);
			Serial.print(" deg C, ");
		}

		etat = bmp180.startPressure(3);
		if (etat != 0)
		{
			delay(etat);

			etat = bmp180.getPressure(P, T);
			if (etat != 0)
			{
				Serial.print("Pression absolue: ");
				Serial.print(P, 2); // Affiche la pression, en mbar , sachant que 1 mbar = 1 hPa
				Serial.println(" hPa, ");
				Serial.println("");
			}
		}
	}
	


	//--------------Partie Anemometre--------------//
	

		/*if (SerialAnemometre.available() > 0)
		{
			char c = SerialAnemometre.read();
			Serial.print(c);
		}*/
	
		
	while (SerialAnemometre.available() > 0)  //Tant qu'il y a des données à récuperer sur la liaison série
	{
		caractere = SerialAnemometre.read();
		phrase[i_nmea] = caractere; // Rempli le tableau de chaque caractere reçus
		i_nmea++; // incrémentation de l'indice du tableau

		if (caractere == '\n')
		{
			phrase[i_nmea] = '\0';

			uint8_t type = nmea_get_message_type(phrase); // On récupère le type de phrase NMEA auquel on a affaire 

			switch (type) { // On switch de "case" en fonction du code de la phrase récupérée dans la variable type

			case 0x01:
			{
				nmea_parse_iimwv(phrase, &reception_phrase_iimwv);

				double angle = reception_phrase_iimwv.angle;
				char vent_apparent = reception_phrase_iimwv.vent_apparent;
				double vitesse = reception_phrase_iimwv.vitesse;
				char unite_vitesse = reception_phrase_iimwv.unite_vitesse;
				char status = reception_phrase_iimwv.status;

				Serial.print("Angle : ");
				Serial.println(angle);
				Serial.print("Vent apparent : ");
				Serial.println(vent_apparent);
				Serial.print("vitesse : ");
				Serial.println(vitesse);
				Serial.print("unite_vitesse : ");
				Serial.println(unite_vitesse);
				Serial.print("status : ");
				Serial.println(status);

				if (premiere_affectation == 0) {
					vent_minimum = vitesse;
					vent_maximum = vitesse;
					vent_moyen = vitesse;
					premiere_affectation = 1;
				}
				else {
					if (vitesse < vent_minimum) {
						vent_minimum = vitesse;
					}
					if (vitesse > vent_maximum) {
						vent_maximum = vitesse;
					}
				}

				Serial.print("Vitesse maximum : ");
				Serial.print(vent_maximum);
				Serial.print(" ");
				Serial.println(unite_vitesse);

				Serial.print("Vitesse minimum : ");
				Serial.print(vent_minimum);
				Serial.print(" ");
				Serial.println(unite_vitesse);
				
				total_vent = total_vent + vitesse;
				nombre_valeurs++;

				Serial.print("Vitesse moyenne : ");
				Serial.print(total_vent / nombre_valeurs);
				Serial.print(" ");
				Serial.println(unite_vitesse);

				ecrire12bits(trame, (int)(angle), 0);
			}
			
				break;

			case 0x02:
			{
				nmea_parse_wixdr(phrase, &reception_phrase_wixdr);

				double temperature = reception_phrase_wixdr.temperature;
				char unite_temperature = reception_phrase_wixdr.unite_temperature;
				double tension_charge = reception_phrase_wixdr.tension_charge;
			
				Serial.print("temperature : ");
				Serial.print(temperature);
				Serial.println(unite_temperature);
				Serial.print("tension de charge : ");
				Serial.println(tension_charge);

				ecrire12bits(trame, (int)(temperature), 1);

				break;
			}

			case 0x03:
			{
				nmea_parse_plcje(phrase, &reception_phrase_plcje);

				char caractere0_identifiant_capteur =
					reception_phrase_plcje.caractere0_identifiant_capteur;
				char caractere1_identifiant_capteur =
					reception_phrase_plcje.caractere1_identifiant_capteur;
				char caractere2_identifiant_capteur =
					reception_phrase_plcje.caractere2_identifiant_capteur;
				char caractere3_identifiant_capteur =
					reception_phrase_plcje.caractere3_identifiant_capteur;

				char numero_capteur[] = { caractere0_identifiant_capteur,
					caractere1_identifiant_capteur,
					caractere2_identifiant_capteur,
					caractere3_identifiant_capteur, '\0' };

				Serial.print("numéro du capteur : ");
				Serial.println(numero_capteur);

				break;
			}
				
			case 0x80:
			{
				Serial.println("Erreur de Checksum");
				break;
			}
		
		} // fin du switch

		if ((millis() - temps) >= 30000) // test si programme s'est deroulé + de 15 minutes 900 000 ms = 15 minutes
		{
			vent_moyen = (total_vent / nombre_valeurs);

			//Création du message SIGFOX
			//ecrire12bits(trame, (angle), 0);
			//ecrire12bits(trame, (temperature), 1);
			ecrire12bits(trame, (vent_moyen), 2); // vitesse moyenne
			ecrire12bits(trame, (vent_maximum), 3); // vitesse max
			ecrire12bits(trame, (vent_minimum), 4); // vitesse min
			ecrire12bits(trame, 0x555, 5); // pression
			
			char message[10] = { trame[0], trame[1], trame[2],
				trame[3], trame[4], trame[5], trame[6],
				trame[7], trame[8], trame[9] };

			Serial.print("trame : ");
			Serial.println(message);

			//Envoie du message SIGFOX


			temps = millis();
		}
			i_nmea = 0;
		}
	}
	
	
}

void nmea_parse_iimwv(char *nmea, iimwv_t *message) // préambule NMEA de vent
{
	char *p = nmea;
	p = strchr(p, ',') + 1;
	message->angle = atof(p); // Conversion float du premier nombre

	p = strchr(p, ',') + 1;
	message->vent_apparent = 'R';

	p = strchr(p, ',') + 1;
	message->vitesse = atof(p); // Attribution de la vitesse au membre vitesse de la struct

	p = strchr(p, ',') + 1;
	if (*p == 'N') // test de l'unité du vent
		message->unite_vitesse = 'N'; // Noeuds
	if (*p == 'M')
		message->unite_vitesse = 'M'; // M/s
	if (*p == 'K')
		message->unite_vitesse = 'K'; // Km/h

	p = strchr(p, ',') + 1;
	if (*p == 'A')
		message->status = 'A'; // Status de CV7SF Correct
	else
		message->status = 'V'; // Status de CV7SF en alarme

}

void nmea_parse_wixdr(char *nmea, wixdr_t *message)
{
	char *p = nmea;
	p = strchr(p, ',') + 1;
	p = strchr(p, ',') + 1;
	message->temperature = atof(p); // Attribution de la temperature au membre temperature de la struct

	p = strchr(p, ',') + 1;
	message->unite_temperature = 'C'; // Attribution de l'unité °C ( Celsius )

	p = strchr(p, ',') + 4; // On passe les deux virgules de la trame, le U et une autre virgule ("C,,U,")
	message->tension_charge = atof(p);

}

void nmea_parse_plcje(char *nmea, plcje_t *message)
{
	char *p = nmea;
	p = strchr(p, ',') + 1;
	message->caractere0_identifiant_capteur = phrase[0];
	message->caractere1_identifiant_capteur = phrase[1];
	message->caractere2_identifiant_capteur = phrase[2];
	message->caractere3_identifiant_capteur = phrase[3];
}

uint8_t nmea_get_message_type(const char *message)
{
	uint8_t checktest = nmea_valid_checksum(message);
	if ((checktest = nmea_valid_checksum(message)) != CHECK_OK)
	{
		return checktest;
	}

	if (strstr(message, NMEA_IIMWV_STR) != NULL)
	{
		return NMEA_IIMWV;
	}

	if (strstr(message, NMEA_WIXDR_STR) != NULL)
	{
		return NMEA_WIXDR;
	}

	if (strstr(message, NMEA_PLCJE_STR) != NULL)
	{
		return NMEA_PLCJE;
	}

	return NMEA_UNKNOWN;
}

uint8_t nmea_valid_checksum(const char *message)
{
	uint8_t checktest = (uint8_t)strtol(strchr(message, '*') + 1, NULL, 16);

	char c;
	uint8_t sum = 0;
	++message;
	while ((c = *message++) != '*')
	{
		sum ^= c;
	}

	if (sum != checktest
		)
	{
		return NMEA_CHECKSUM_ERR;
	}

	return CHECK_OK;
}


void ecrire12bits(unsigned char *pointeur_sur_trame, int valeur, int numero) {
	unsigned char *adresse_base;

	// si numero est pair
	if (estPair(numero)) {
		adresse_base = pointeur_sur_trame + ((numero / 2) * 3);

		// Ecrit le premier octet
		*adresse_base = (unsigned char)(valeur);

		// Ecrit les 4 bits suivant
		*(adresse_base + 1) |= (unsigned char)((valeur >> 8) & 0x0F);
	}
	else {
		// tfp_printf("\r\n\r\nCEST IMPAIR \r\n\r\n");

		adresse_base = pointeur_sur_trame + 1 + (((numero - 1) / 2) * 3);

		// ecrit le premier quartet
		*adresse_base |= (unsigned char)((valeur << 4) & 0xF0);

		// ecrit l'octet suivant
		*(adresse_base + 1) |= (unsigned char)(valeur >> 4);
	}
}

bool estPair(unsigned int n) {
	return !(n & 1);
}