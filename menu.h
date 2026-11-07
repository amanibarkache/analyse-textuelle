#ifndef MENU_H
#define MENU_H

#include "tree.h"
#include "ensemble.h"

/* ================================================================
   UTILITAIRES CONSOLE
   ================================================================ */
void cls(void);
void sep_lourde(void);
void sep_legere(void);
void menu_pause(void);
void menu_banniere(void);

/* ================================================================
   MENUS PRINCIPAUX
   ================================================================ */
int  menu_principal(void);
int  menu_operations(void);
int  menu_mode(void);

/* ================================================================
   GESTION FICHIERS
   ================================================================ */
void menu_afficher_fichiers(void);
int  menu_charger_fichiers(void);
void menu_liberer_un_fichier(void);
void menu_tout_liberer(void);

/* ================================================================
   PARAMETRES
   ================================================================ */
float menu_choisir_seuil(void);
int   menu_choisir_langue(void);

/* ================================================================
   SELECTION INTERACTIVE DE PARAGRAPHES (mode paragraphes)

   Retourne un tableau de parag* avec leurs metadonnees.
   *n_out       : nombre de paragraphes selectionnes
   *nums_out    : numeros 1-based (a liberer par l appelant)
   *noms_out    : noms des fichiers sources (a liberer par l appelant)
   nom_ens      : etiquette affichee ("A", "B", ...)

   Retourne NULL si l utilisateur n a rien selectionne.
   ================================================================ */
parag **menu_selectionner_parags(int *n_out, int **nums_out,char (**noms_out)[50],const char *nom_ens);

/* ================================================================
   CONSTRUCTION D ENSEMBLES DE MOTS (mode mots)

   Retourne un tableau d Ensemble* construits interactivement.
   *nb_ens : nombre d ensembles crees.
   L appelant libere chaque Ensemble via ens_liberer().
   ================================================================ */
Ensemble **menu_construire_ensembles(int *nb_ens);

#endif