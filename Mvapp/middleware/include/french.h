#ifndef _CS_APP_FRENCH_H_
#define _CS_APP_FRENCH_H_

#include "mwsetting.h"


static U8 * FrenchString [CSAPP_STR_MAX] = {
"",
"Main Menu",
"Installation",
"Install & recherche",
"Cadre Satellite",
"Cadre TP",
"Edit Program",
"Configuration du systeme",
"Systeme d'information",
"Toutes les TV Satellite",
"Tous les Radio Satellite",
"Channel FAV",
"Aucun canal de ...",
"Aucun programme ...",
"LCN",
"Nom",
"Lock",
"Love",
"Niveau",
"SNR",
"HD",
"H264",
"AC3",
"CC",
"Progres",
"Mode",
"Frequence",
"La bande passante",
"MHz",
"Demarrer",
"Manuel",
"Auto",
"NIT",
"Progres",
"Numero de TV",
"Nombre Radio",
"Modifier la television",
"Edit Radio",
"Cadre AV",
"Configuration du systeme",
"Reglage de l'heure",
"Controle parental",
"Factory Reset",
"Type no",
"Version HW",
"Version SW",
"Timer", // ??kb: le 25 mars
"EPG", // ??kb: le 25 mars
"Simple", // ??kb: le 25 mars
"Prevu", // ??kb: le 25 mars
"Actuel", // ??kb: le 25 mars
"Suivant", // ??kb: le 25 mars
"Non ... informations detaillees", // kb: le 25 mars
"Journee pre",
"Lendemain",
"Desc.",
"Lun.",
"Mar.",
"Mer.",
"Jeu.",
"Ven.",
"Sat." ,
"Soleil",
"Volume",
"Reglage Audio",
"Lang Audio",
"Piste audio",
"Stereo",
"Gauche",
"Droite",
"Mono",
"Frequence",
"Debit symbole",
"Polarite",
"H",
"V",
"L'etat du signal:",
"Aucun signal",
"Aucun service",
"Scramble",
"Teletexte",
"Sous-titres",
"Aspect ratio",
"Aspect mode",
"Def video",
"Sortie video",
"4:3",
"16:9",
"Automatique",
"Letterbox",
"PanScan",
"Full",
"CVBS",
"YC",
"YCbCr",
"YPbPr",
"RGB",
"HDMI",
"Reglage de la langue",
"Num CH Logic",
"Taux de Parent",
"Fuseau horaire",
"Mode Time",
"Time Satellite",
"Heure locale",
"Date",
"Time",
"OK: sauvegarder et quitter,\nMENU / EXIT: annuler et quitter",  /* By KB Kim 2011.05.06 */
"Region Time",
"Transparence",
"Ordre d'apparition",
"Defini par l'operateur",
"Lock Status",
"Boot Lock",
"Verrouillage de Menu",
"Channel Edit Lock",
"Lock Channel",
"Lock Favoris",
"Change Password",
"Enable",
"Disable",
"Ancien code PIN",
"Nouveau PIN",
"Confirmer pin",
"Mot de passe d'entree",
"Mot de passe",
"Erreur Ancien mot de passe.",
"Erreur de mot de passe Confirmer",
"Nouveau succes Mot de passe",
"Avertissement",
"Cela va effacer tous les services, \n Continuer?",
"Les donnees sont modifiees, sauf?",
"OK",
"Annuler",
"Enregistrer",
"Appuyez sur OK pour comfirm, \n EXIT Press d'annuler",
"Supprimer",
"Lock",
"Lock",
"Fav",
"Trier",
"Cette operation va charger \n par defaut et effacer toutes les \n les canaux que l'utilisateur a ajoute, \n continuer?",
"Chargement des parametres par defaut, \n s'il vous plait ne pas couper l'alimentation!",
"Cette operation de sauvegarde de donnees \n - satellite et tp & Channel - \n continuer?",
"Cette operation restore \n donnees de sauvegarde de base de donnees \n -. Satellite et tp & Channel -? \n continuer",
"Cette operation va ajouter \n nouveau plug-in Program Files \n & Reboot System",
"SPDIF",
"PCM",
"AC3",
"Force",
"Qualite",
"Reglage OSD",
"Configuration du reseau",
"Fichier enregistre",
"Enregistre Config",
"Tools File",
"Peripherique USB Supprimer",
"Stockage de l'information",
"MP3 Player",
"Upgrade",
"Interface commune",
"Acces conditionnel",
"Backup",
"Sauvegarde et restauration",
"Plug-in",
"Heure d'ete",
"Annee",
"Mois",
"Jour",
"Heure",
"Minute",
"On",
"Off",
"La video",
"Bright",
"Contraste",
"Couleur",
"Medias",
"Outil",
"Le temps Banner Info",
"DHCP",
"Configuration DHCP",
"Adresse IP",
"Masque de sous-reseau",
"Gateway",
"DNS primaire",
"DNS secondaire",
"DNS tiers",
"Adresse Mac",
"Type de connexion",
"Satellite",
"TP Select",
"Type LNB",
"Power LNB",
"Tone 22K",
"Haute frequence",
"Basse frequence",
"Configuration DiSEqC",
"Type de numerisation",
"Multi Sat",
"Single Sat",
"Satellite Selectionnez",
"Transpondeur Select",
"ToneBurst",
"Renommer",
"Scan",
"Ajouter nouveau transpondeur",
"Addition manuelle du canal",
"Find",
"Avance",
"Modifier",
"Transpondeur",
"PID video",
"PID audio",
"PCR PID",
"Service ID",
"TP meme",
"Donnees non valide",
"No Data", // ??par ko: 20100406
"TP Data n", // ??par ko: 20110115
"Etes-vous sur?",
"Deplacer",
"Tous",
"TV",
"Radio",
"Supprimer actuelle TP",
"Select",
"Jump",
"A-> Z",
"Z-A Trier",
"FTA-CAS Trier",
"Trier CAS-FTA",
"Trier SD-HD",
"Trier HD-SD",
"Restaurer la normale",
"Tous les Satellite",
"Attention",
"Maintenant, Saving ... \n Don't Appuyez sur une touche!",
"Ecran Pause",
"Ecran noir",
"Changer le type de chaine",
"Pas utiliser",
"Utilisation",
"Get DHCP",
"Maintenant Get DHCP Data \n Wait Certains Deuxieme ...",
"FAIL Get DHCP Data \n Checking Reseau Environnement",
"Blancs adresse IP .. \n Reessayez!",
"PING Test",
"CLIQUETIS DNS .....",
"CLIQUETIS ..... IP",
"Ping echouent Reseau Check",
"DNS ping echouent, PING IP OK \n Verifiez DNS Address",
"PING OK !!!!",
"Aucun",
"Langue",
"Nombre Channel Blancs ...",
"Taille totale",
"Taille d'occasion",
"L'espace disponible",
"Type de stockage",
"Vender de stockage",
"Produit de stockage",
"Nombre de stockage Serial",
"Format de stockage",
"Effacer tout TP",
"S'il vous plait attendre",
"Position n ��",
"Goto X",
"Deplacer l'etape",
"Deplacer Auto",
"Reglage de limite",
"La limite ouest Set",
"Limite Est Set",
"Aller de reference",
"Recalcul",
"Premier satellite Select",
"USALS Cadre",
"Longitude Satellite",
"Longitude locaux",
"Latitude locaux",
"Modifier Longitude",
"Cadre Unicable",
"LNB Unicable Select",
"Position",
"Canal de transmission",
"Frequence de transmission",
"Tous les canaux Supprimer",
"Supprimer le canal by Satellite",
"Clavier",
"Clavier numerique",
"Condition de la recherche",
"Transpondeur Addition",
"Addition Channel",
"Time Shift",
"Record Time Shift",
"Type Stream",
"Saut dans le temps",
"Maintenant, PVR enregistrement \n Cliquez First Stop",
"Maintenant, PVR Jouer \n Cliquez First Stop",
"Pour interne de la chair",
"Pour USB",
"De Chair interne",
"D'apres les donnees par defaut",
"De USB",
"Etat des cartes",
"Carte",
"File Not Found",
"Informations detaillees CAS",
"Rappel",
"Simple",
"Multi",
"Type de modele",
"Version UBoot",
"Version du noyau",
"Version rootfs",
"Version SW Main",
"Default sam. Version TP",
"Base de donnees par defaut",
"Arriere-plan video",
"Contexte Audio",
"Copier",
"Coller",
"Meme dossier",
"Il ya meme fichier",
"Ajouter",
"Scaning ..",
"Verrouille",
"Deverrouillage",
"Elargir la Chaine d'information",
"Type de reseau",
"Client NewCamd",
"Server NewCamd",
"CCcam",
"Nombre Server",
"URL",
"Port du serveur",
"Utilisateur",
"DES Key",
"Type CA",
"Auto Connect",
"Log in",
"Demarrer",
"Police",
"Taille de la police",
"Anglais",
"Turque",
"Francais",
"Allemande",
"Greek",
"Arabe",
"Persan",
"Temps Internet",
"Methode DNS",
"Emplacement vide",
"Carte inseree",
"Fichier explorer ",
"Database",
"Tous Backup Flash",
"ALL Flash Restore",
"No Files",
"Non Selectionnez Fichier",
"Enregistrement d'erreur",
"Lecture d'erreur",
"Continuer?",
"Non",
"On / Off",
"Type",
"Repetition",
"Channel",
"End",
"Duree",
"Mode hors tension",
"Mode veille",
"Off Real",
"Affichage de l'heure en veille",
"REBOOT SYSTEM \n Voulez-vous sur?",
"Reboot System",
"Extended",
"Normal",
"Liste de la Manche",
"Wake Up",
"Sleep",
"Record",
"Une fois",
"Chaque jour",
"Chaque semaine",
"Jeu",
"Goodbye ...",
"Bon .. Next?",
"Stage",
"Redemarrer",
"Retour",
"Etape suivante",
"Stage Prev",
"Janvier",
"Fevrier",
"Mars",
"Avril",
"Mai",
"Juin",
"Juillet",
"Aout",
"Septembre",
"Octobre",
"Novembre",
"Decembre",
"Calendrier",
"Le mois prochain",
"Mois Precedent",
"L'an prochain",
"Annee precedente",
"Partition changement",
"Partition",
"Titre",
"Artiste",
"Album",
"Modifier BISS",
"Vaisselle Move",
"Le telechargement ..",
"Obtenir la liste .. FAIL! \n Reessayer!",
"Telechargement de fichier de mise a niveau .. FAIL! \n Reessayer!",
"Mise a jour .. FAIL! \n Reessayer!",
"Etes-vous de mise a niveau?",
"Space",
"Skin",
"Reset",
"Sat A",
"Sat B",
"Animation Menu",
"Bit Heart",
"FTA + CAS Recherche",
"FTA + CAS Recherche NIT",
"Recherche ALE",
"Recherche ALE NIT",
"Recherche FTA + CAS Blind",
"Recherche ALE Blind",
"Port DiSEqC",
"Moteur DiSEqC",
"USALS",
"Unicable",
"System Ready",
"Boot block",
"Noyau",
"Plug-In",
"Application 1",
"Application 2",
"Dossier racine du systeme",
"Finish",
"Bloc de botte Mise a niveau",
"Mise a niveau du noyau",
"Plug-In Mise a niveau",
"Application 1 Mise a niveau",
"Application 2 Mise a niveau",
"Mise a niveau de systeme de fichiers racine",
"Finition",
"USB",
"Reseau",
"VALORISATION logiciel ... S'il vous plait attendre",
"Cadre OSCAM",
"End Port",
"Username",
"Cles",
"Status",
"CCCamd",
"NewCamd",
"Connect",
"Disconnect",
"Connected",
"Not Connected",
"Nombre Server",
"Protocole",
"Start Port",
"DES Key",

"Inconnu", /* Par KB 09/04/1022 */
"Impossible de modifier \n erreur Pincode! Reessayer?",
"Impossible de modifier \n Nouveaux code d'erreur! Reessayer?",
"Test",
"Si de mise a niveau ne se termine pas dans les 10 minutes,\npls puissance-au loin et sur la main", /* By KB Kim : 2011.05.07*/
"USB Device CONNEXION ...",
"Monter le peripherique USB",
"Peripherique USB DECONNEXION ...",
"Demonter un peripherique USB",
"Tout effacer",
//"Enregistrer",
"Back Space",
"NAME",
"SIZE",
"Temps record",
"Programme",
//"Duree",
"End Time",
//"TIME",
"Verrouillage majuscule",
"Caps",
"Del",
"Inserer la carte SMART ...",
"SMART card Remove ...",
"Install Plug-In", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Install PlugIn .. FAIL !!\n Retry !!", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Do You Want to Install?", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Add Server", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Server Data Full", /* By KB Kim for Plugin Setting : 2011.05.07 */
"Delete Server",    /* By KB Kim for Plugin Setting : 2011.05.25 */
"Extend" ,
"Detail Information",
"No Module Inserted",
"Module Initialising......",
"Module OK !",
"Change Focus",
"Locked\nChannel",
"Site List",		/* By KB Kim for Plugin Site List : 2011.09.20 */
"Can Not Delete",		/* By KB Kim for Plugin Site List : 2011.09.20 */
"Can Not Add",		/* By KB Kim for Plugin Site List : 2011.09.20 */
"Remove channel from Favorite",
"Uygulama Ba�l�yor...",
"Installing Plugin...",
"Plugin Installed Succesfully!!",
"Currently Streaming \n Press STOP first!!",
"Streaming Starting..."
};
#endif


