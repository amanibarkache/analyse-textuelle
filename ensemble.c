#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ensemble.h"

/* ================================================================
                            CREATION
   ================================================================ */
Ensemble *ens_creer(const char *nom)
{
    Ensemble *E = calloc(1, sizeof(Ensemble));
    if (!E) { fprintf(stderr, "erreur dans creation de ensemble\n"); exit(1); }
    strncpy(E->nom, nom, NOM_ENS_MAX - 1);// -1 pour le '\0'
    // initialisation des champs
    E->tokens        = NULL;
    E->nb_tokens     = 0;
    E->parags        = NULL;
    E->nums          = NULL;
    E->noms_fichiers = NULL;
    E->nb_parags     = 0;
    return E;
}

/* ================================================================
                   AJOUT D'UN PARAGRAPHE
   ================================================================ */
void ens_ajouter_parag(Ensemble *E, parag *p, int num, const char *nom_fichier)
{
    if (!E || !p) return;

    /* Fusionner les tokens du paragraphe dans le BST de l ensemble */
    token *cur_stack[4096];
    int    top = 0;
    token *cur = p->Rtoken;

    while (cur || top > 0) {
        while (cur) { cur_stack[top++] = cur; cur = cur->lc; }
        cur = cur_stack[--top];
        E->tokens = tk_insert(E->tokens, cur->word);
        cur = cur->rc;
    }
    E->nb_tokens = tk_count(E->tokens);

    /* Enregistrer la source */
    int n = E->nb_parags;
    E->parags        = realloc(E->parags,        (n + 1) * sizeof(parag *));
    E->nums          = realloc(E->nums,          (n + 1) * sizeof(int));
    E->noms_fichiers = realloc(E->noms_fichiers, (n + 1) * sizeof(*E->noms_fichiers));

    if (!E->parags || !E->nums || !E->noms_fichiers) {
        fprintf(stderr, "ereur : ens_ajouter_parag\n");
        exit(1);
    }

    E->parags[n] = p;
    E->nums[n]   = num;
    strncpy(E->noms_fichiers[n], nom_fichier, 49);
    E->noms_fichiers[n][49] = '\0';
    E->nb_parags++;
}

/* ================================================================
   LIBERATION
   ================================================================ */
void ens_liberer(Ensemble *E)
{
    if (!E) return;
    tk_free_all(E->tokens);
    free(E->parags);
    free(E->nums);
    free(E->noms_fichiers);
    free(E);
}

/* ================================================================
   AFFICHAGE ENSEMBLE : { mot1, mot2, ... }
   ================================================================ */
void ens_afficher(Ensemble *E) // parcours inorder itératif d'affichage
{
    if (!E) return;

    printf("  Ensemble %s = { ", E->nom);

    token *stack[4096];
    int    top    = 0;
    token *cur    = E->tokens;
    int    premier = 1; // utiliser pour éviter d'afficher une virgule avant le premier mot

    while (cur || top > 0) {
        while (cur) { stack[top++] = cur; cur = cur->lc; }
        cur = stack[--top];
        if (!premier) printf(", ");
        printf("%s", cur->word);
        premier = 0;
        cur = cur->rc;
    }

    printf(" }  (%d mot(s))\n", E->nb_tokens);
}

/* ================================================================
   AFFICHAGE SOURCES ( les paragraghes qui composent l'ensemble)
   ================================================================ */
void ens_afficher_sources(Ensemble *E)
{
    if (!E) return;
    printf("  Sources de l'ensemble %s :\n", E->nom);
    for (int k = 0; k < E->nb_parags; k++)
        printf("    - P%d [%s]\n", E->nums[k], E->noms_fichiers[k]);
}