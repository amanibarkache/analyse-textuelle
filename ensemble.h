#ifndef ENSEMBLE_H
#define ENSEMBLE_H

#include "tree.h"

/* ================================================================
                             ENSEMBLE
   ================================================================ */

#define MAX_ENSEMBLES 16
#define NOM_ENS_MAX   32

typedef struct {
    char   nom[NOM_ENS_MAX]; // nom de l'ensemble 
    token *tokens;           /* BST  tous les tokens de l'ensemble */
    int    nb_tokens;        /* nobre de tous les tokens de l'ensemble                  */

    parag **parags;          /* tableau de pointeurs vers les parags   */
    int    *nums;            
    char  (*noms_fichiers)[50]; /* nom du fichier source de chaque parag */
    int    nb_parags;        /* nombre de paragraphes dans l ensemble  */
} Ensemble;

/* Alloue et retourne un Ensemble vide */
Ensemble *ens_creer(const char *nom);

/* Ajoute un paragraphe a l ensemble  */
void ens_ajouter_parag(Ensemble *E, parag *p, int num, const char *nom_fichier);

/* Libere la memoire d un Ensemble */
void ens_liberer(Ensemble *E);

/* Affiche le contenu de l ensemble sous forme */
void ens_afficher(Ensemble *E);

/* Affiche les paragraphes sources de l ensemble */
void ens_afficher_sources(Ensemble *E);

#endif /* ENSEMBLE_H */