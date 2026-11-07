#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "lecture.h"

#define MAX_LIGNE 1024
#define MAX_PARAG 8192

/*  le détecteur de frontière entre deux paragraphes.*/
static int ligne_vide(const char *ligne)
{
    for (int i = 0; ligne[i] && ligne[i] != '\n'; i++) // parcourt la ligne caractère par caractère
        if (ligne[i] != ' ' && ligne[i] != '\t') return 0;
    return 1;
//retourne 1 si elle ne contient que des espaces ou tabulations
}

int compter_paragraphes(const char *chemin)
{
    FILE *f = fopen(chemin, "r"); // ouvre le fichier en mode lecture
    if (!f) return 0;

    char ligne[MAX_LIGNE]; 
    int  nb       = 0; // compteur de paragraghers
    int  dans_par = 0; // indique si on est dans un paragraghe

    while (fgets(ligne, MAX_LIGNE, f)) {
        if (!ligne_vide(ligne)) {
            if (!dans_par) { nb++; dans_par = 1; }
        } else {
            dans_par = 0;
        }
    }

    fclose(f);
    return nb;
}

file *lire_fichier(const char *chemin, bool use_stpW)
{
    int nb = compter_paragraphes(chemin);
    if (nb == 0) {
        fprintf(stderr, "Erreur : fichier vide ou introuvable : %s\n", chemin);
        return NULL;
    }

    const char *nom = chemin;
    for (const char *p = chemin; *p; p++)
        if (*p == '/' || *p == '\\') nom = p + 1;

    file *fi = allocate_file(nom, nb);

    FILE *f = fopen(chemin, "r");
    if (!f) { file_free(fi); return NULL; }

    char ligne[MAX_LIGNE];
    char buf[MAX_PARAG];
    buf[0] = '\0';

    int idx      = 0; //indice du paragraphe en cours de construction
    int dans_par = 0;

    while (fgets(ligne, MAX_LIGNE, f) && idx < nb) {
        if (!ligne_vide(ligne)) {
            strncat(buf, ligne, MAX_PARAG - strlen(buf) - 1);
            dans_par = 1;
        } else if (dans_par) {
            fi->paragraphe[idx] = parag_allocate(buf);
            parag_build(fi->paragraphe[idx], use_stpW);
            idx++;
            buf[0]   = '\0';
            dans_par = 0;
        }
    }

    if (dans_par && idx < nb && strlen(buf) > 0) {
        fi->paragraphe[idx] = parag_allocate(buf);
        parag_build(fi->paragraphe[idx], use_stpW);
    } // Gère les fichiers qui ne se terminent pas par une ligne vide 

    fclose(f);
    printf("  [OK] %s -- %d paragraphe(s) charge(s)\n", nom, nb);
    return fi;
}


