#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "tree.h"
#include "lecture.h"
#include "ensemble.h"
#include "operations.h"
#include "menu.h"

/* ================================================================
   BOUCLE PARAGRAPHES -- N ensembles traites 2 a 2

   L utilisateur cree autant d ensembles qu il veut (min 2).
   On applique l operation sur les paires consecutives :
     (A,B), puis (resultat,C)
   ================================================================ */
static void boucle_paragraphes(void)
{
    float seuil = menu_choisir_seuil();
    int   mode  = menu_mode();
    int   op    = menu_operations();
    if (op <= 0) return;

    /* Demander combien d ensembles */
    printf("\n");
    sep_legere();
    printf("  Combien d'ensembles de paragraphes ? (min 2) : ");
    int nb_ens = 2;
    if (scanf("%d", &nb_ens) != 1 || nb_ens < 2) nb_ens = 2;
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    /* Tableaux pour chaque ensemble */
    parag ***tabs  = calloc(nb_ens, sizeof(parag **));
    int    **numss = calloc(nb_ens, sizeof(int *));
    char  (**nomss)[50] = calloc(nb_ens, sizeof(*nomss));
    int   *ns    = calloc(nb_ens, sizeof(int));

    if (!tabs || !numss || !nomss || !ns) {
        fprintf(stderr, "[!] Memoire boucle_paragraphes\n");
        exit(1);
    }

    /* Selectionner chaque ensemble */
    for (int k = 0; k < nb_ens; k++) {
        char nom_ens[8];
        if (nb_ens <= 26)
            snprintf(nom_ens, 8, "%c", 'A' + k);
        else
            snprintf(nom_ens, 8, "E%d", (k + 1) % 100);

        tabs[k] = menu_selectionner_parags(&ns[k], &numss[k], &nomss[k], nom_ens);
        if (!tabs[k] || ns[k] == 0) {
            printf("  [!] Aucun paragraphe selectionne pour %s. Abandon.\n", nom_ens);
            for (int i = 0; i < k; i++) {
                free(tabs[i]); free(numss[i]); free(nomss[i]);
            }
            free(tabs); free(numss); free(nomss); free(ns);
            return;
        }
    }

    /* Noms d ensembles pour l affichage */
    char (*noms_ens)[8] = calloc(nb_ens, 8);
    for (int k = 0; k < nb_ens; k++) {
        if (nb_ens <= 26)
            snprintf(noms_ens[k], 8, "%c", 'A' + k);
        else
            snprintf(noms_ens[k], 8, "E%d", (k + 1) % 100);
    }

    /* Executer l operation 2 a 2 */
    for (int k = 0; k < nb_ens - 1; k++) {
        switch (op) {
            case 1:
                parag_intersection(tabs[k],   ns[k],   numss[k],   nomss[k],   noms_ens[k],
                                   tabs[k+1], ns[k+1], numss[k+1], nomss[k+1], noms_ens[k+1],
                                   seuil, mode);
                break;
            case 2:
                parag_union(tabs[k],   ns[k],   numss[k],   nomss[k],   noms_ens[k],
                            tabs[k+1], ns[k+1], numss[k+1], nomss[k+1], noms_ens[k+1],
                            seuil, mode);
                break;
            case 3:
                parag_difference(tabs[k],   ns[k],   numss[k],   nomss[k],   noms_ens[k],
                                 tabs[k+1], ns[k+1], numss[k+1], nomss[k+1], noms_ens[k+1],
                                 seuil, mode);
                break;
            case 4:
                parag_subset(tabs[k],   ns[k],   numss[k],   nomss[k],   noms_ens[k],
                             tabs[k+1], ns[k+1], numss[k+1], nomss[k+1], noms_ens[k+1],
                             seuil, mode);
                break;
            default:
                printf("  [!] Operation invalide.\n");
                break;
        }
    }

    /* Liberation */
    for (int k = 0; k < nb_ens; k++) {
        free(tabs[k]); free(numss[k]); free(nomss[k]);
    }
    free(tabs); free(numss); free(nomss); free(ns);
    free(noms_ens);
}

/* ================================================================
   BOUCLE MOTS -- N ensembles
l'utilisateur cree autant d ensembles qu il veut (min 2).
on applique l operation sur les N ensembles :
    intersection : mots dans tous les ensembles
    union        : mots dans au moins un ensemble 
    difference   : mots dans le premier ensemble mais pas les autres
   ================================================================ */
static void boucle_mots(void)
{
    float seuil = menu_choisir_seuil();
    int   op    = menu_operations();
    if (op <= 0) return;

    int nb_ens = 0;
    Ensemble **ens = menu_construire_ensembles(&nb_ens);

    if (nb_ens < 2) {
        printf("  [!] Il faut au moins 2 ensembles.\n");
        for (int k = 0; k < nb_ens; k++) ens_liberer(ens[k]);
        free(ens);
        return;
    }

    switch (op) {
        case 1: words_intersection(ens, nb_ens, seuil); break;
        case 2: words_union       (ens, nb_ens, seuil); break;
        case 3: words_difference  (ens, nb_ens, seuil); break;
        case 4: words_subset      (ens, nb_ens, seuil); break;
        default: printf("  [!] Operation invalide.\n"); break;
    }

    for (int k = 0; k < nb_ens; k++) ens_liberer(ens[k]);
    free(ens);
}

/* ================================================================
   MAIN
   ================================================================ */
int main(void)
{
    lang(1); /* Francais par defaut */

    cls();
    menu_banniere();
    printf("  Bienvenue. Chargez au moins un fichier pour commencer.\n");
    printf("  Langue : Francais (stopwords FR actifs)\n");
    menu_pause();

    int continuer = 1;

    while (continuer) {
        int choix = menu_principal();

        switch (choix) {

            case 1:
                menu_charger_fichiers();
                menu_pause();
                break;

            case 2:
                menu_afficher_fichiers();
                menu_pause();
                break;

            case 3:
                if (nbfiles == 0) {
                    printf("  [!] Chargez d'abord un fichier (option 1).\n");
                    menu_pause();
                } else {
                    boucle_paragraphes();
                    menu_pause();
                }
                break;

            case 4:
                if (nbfiles == 0) {
                    printf("  [!] Chargez d'abord un fichier (option 1).\n");
                    menu_pause();
                } else {
                    boucle_mots();
                    menu_pause();
                }
                break;

            case 5: {
                int l = menu_choisir_langue();
                lang(l);
                printf("  [OK] Langue : %s\n", l == 1 ? "Francais" : "Anglais");
                printf("  Note : rechargez les fichiers pour appliquer.\n");
                menu_pause();
                break;
            }

            case 6:
                menu_liberer_un_fichier();
                menu_pause();
                break;

            case 7:
                menu_tout_liberer();
                menu_pause();
                break;

            case 0:
                continuer = 0;
                break;

            default:
                printf("  [!] Choix invalide (0 a 7).\n");
                menu_pause();
                break;
        }
    }

    if (nbfiles > 0) menu_tout_liberer();
    cls();
    menu_banniere();
    printf("  Au revoir !\n\n");
    return 0;
}