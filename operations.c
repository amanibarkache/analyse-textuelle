#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "operations.h"

/* ================================================================
   SEPARATEURS (ASCII uniquement)
   ================================================================ */
static void SEP_L(void) { printf("=========================================================\n"); }
static void SEP_l(void) { printf("---------------------------------------------------------\n"); }

/* ================================================================
   APERCU D UN PARAGRAPHE (50 premiers caracteres, sur une ligne)
   ================================================================ */
static void apercu(const char *texte)
{
    char buf[54];
    strncpy(buf, texte, 50);
    buf[50] = '\0';
    for (int i = 0; buf[i]; i++)
        if (buf[i] == '\n' || buf[i] == '\r') buf[i] = ' ';
    printf("%s...", buf);
}

/* ================================================================
   SCORE JACCARD -- dispatch selon mode
   mode 0 : Jaccard tokens
   mode 1 : Jaccard bigramW
   ================================================================= */
static float jaccard_score(parag *A, parag *B, int mode)
{
    if (mode == 0)
        return jaccard_tk(A, B);
    else
        return jaccard_bigramW(A, B);
}
static const char *nom_mode(int mode)
{
    if (mode == 0) return "Jaccard tokens";
    else           return "Jaccard bigramW";
}

/* ================================================================
   UTILITAIRES MODE MOTS
   ================================================================ */

static float upper_bound(int sA, int sB) 
//évite le parqourt entier de deux bst bigramL .

{
    int mn = sA < sB ? sA : sB;
    int mx = sA > sB ? sA : sB;
    if (mx == 0) return 0.0f;
    return (float)mn / (float)mx;
}

/*
 * Cherche dans RB le meilleur token similaire a tkA (score >= seuil).
 * Anti-doublon via BST marques.
 */
static token *search_similar(token *tkA, token *RB, float seuil, token **marques)
{
    if (!tkA || !RB) return NULL;

    if (seuil >= 1.0f) { //cas de comparaison exacte sueil = 1
        token *cur = RB;
        while (cur) {
            int cmp = strcmp(tkA->word, cur->word);
            if (cmp == 0) {
                if (marques && tk_search(*marques, cur->word)) return NULL;
                return cur;
            }
            cur = (cmp < 0) ? cur->lc : cur->rc;
        }
        return NULL;
    }

    int    sA   = bigramL_count(tkA->RbigramL);
    token *best = NULL;
    float  best_sc = -1.0f;
    /* parcours inorder iteratif avec une pile manuelle*/

    token *stack[4096];
    int    top = 0;
    token *cur = RB;

    while (cur || top > 0) {
        while (cur) { stack[top++] = cur; cur = cur->lc; }
        cur = stack[--top];
        if (!marques || !tk_search(*marques, cur->word)) {
            int sB = bigramL_count(cur->RbigramL);
            if (upper_bound(sA, sB) >= seuil) {
                float sc = jaccard_bigramL(tkA, cur);
                if (sc >= seuil && sc > best_sc) { best_sc = sc; best = cur; }
            }
        }
        cur = cur->rc;
    }
    return best;
}

/* Affiche un BST de tokens comme :  nom = { a, b, c }  (N element(s)) */
static void afficher_bst(token *R, const char *nom)
{
        /* parcours inorder iteratif avec pile */
    printf("  %s = { ", nom);
    token *stack[4096];
    int    top = 0, premier = 1;
    token *cur = R;
    while (cur || top > 0) {
                /* descendre jusqu au noeud le plus a gauche */
        while (cur) { stack[top++] = cur; cur = cur->lc; }
        cur = stack[--top];
        if (!premier) printf(", ");
        printf("%s", cur->word);
        premier = 0;
        cur = cur->rc;         /* passer au sous-arbre droit */

    }
        /* fermer l accolade et afficher le nombre total de mots */
    printf(" }  (%d element(s))\n", tk_count(R));
}

/* Fusionne les tokens de R dans global: parcours inorder recursif 
  garantit que les mots sont traites en ordre alphabetique */
static token *fusion_bst(token *global, token *R)
{
    if (!R) return global;
    global = fusion_bst(global, R->lc);
    global = tk_insert(global, R->word);
    global = fusion_bst(global, R->rc);
    return global;
}

/*
 * Calcule l'intersection de deux BST de tokens.
 * Retourne un nouveau BST contenant les mots presents dans les deux.
 * Utilise search_similar avec anti-doublon (marques).
 */
static token *bst_intersection(token *RA, token *RB, float seuil)
{
    token *resultat = NULL;
    token *marques  = NULL;
        /* parcours inorder iteratif de RA */
    token *stack[4096]; int top = 0; token *cur = RA;
    while (cur || top > 0) {
      /* descendre jusqu au noeud le plus a gauche */
        while (cur) { stack[top++] = cur; cur = cur->lc; }
        cur = stack[--top];
        /* chercher le meilleur equivalent de ce mot dans RB
        en respectant le seuil et l anti-doublon */
        token *match = search_similar(cur, RB, seuil, &marques);
        if (match) {
            resultat = tk_insert(resultat, cur->word);
            marques  = tk_insert(marques, match->word);
        }
        cur = cur->rc;         /* passer au sous-arbre droit */

    }
    tk_free_all(marques);
    return resultat;
}

/*
  Calcule la difference RA \ RB.
  Retourne un nouveau BST avec les mots de RA sans similaire dans RB.
 */
static token *bst_difference(token *RA, token *RB, float seuil)
{
    token *resultat = NULL;
    token *marques  = NULL;
    /* parcours inorder iteratif de RA */
    token *stack[4096]; int top = 0; token *cur = RA;
    while (cur || top > 0) {
   /* descendre jusqu au noeud le plus a gauche */
        while (cur) { stack[top++] = cur; cur = cur->lc; }
        cur = stack[--top];
   /* chercher un equivalent de ce mot dans RB */
        token *match = search_similar(cur, RB, seuil, &marques);
        // garder si AUCUN match trouve
        if (!match)
            resultat = tk_insert(resultat, cur->word);
        else
            marques = tk_insert(marques, match->word);

        cur = cur->rc;
    }
    tk_free_all(marques);
    return resultat;
}


/* ================================================================
   MODE PARAGRAPHES parcours inorder recursif 
   ================================================================

   Chaque operation recoit deux tableaux de parag*.
   Pour N ensembles main.c appelle ces fonctions 2 a 2 en iterant.
*/

/* ----------------------------------------------------------------
   INTERSECTION  des paragraghes A inter B
   ---------------------------------------------------------------- */
void parag_intersection(parag **tab_A, int na, int *nums_A, char (*noms_A)[50], const char *nom_ens_A,
                        parag **tab_B, int nb, int *nums_B, char (*noms_B)[50], const char *nom_ens_B,
                        float seuil, int mode)
{
    SEP_L();
    printf("  INTERSECTION PARAGRAPHES : %s inter %s\n", nom_ens_A, nom_ens_B);
    printf("  Seuil %.0f%%  |  %s\n", seuil * 100, nom_mode(mode));
    SEP_l();

    (void)noms_B;

    int *matched_A = calloc(na, sizeof(int));
    if (!matched_A) return;

    int trouve = 0; /* compteur de paires similaires trouvees */
    for (int i = 0; i < na; i++) { 
         /* comparer chaque paragraphe de A avec chaque paragraphe de B */
        if (!tab_A[i]) continue;
        for (int j = 0; j < nb; j++) {
            if (!tab_B[j]) continue;
            float score = jaccard_score(tab_A[i], tab_B[j], mode);
            if (score >= seuil) {
                trouve++;
                matched_A[i] = 1;
                printf("\n  Paire similaire (%.1f%%) :\n", score * 100);
                printf("    [%s] P%d : ", nom_ens_A, nums_A[i]);
                apercu(tab_A[i]->texte); printf("\n");
                printf("    [%s] P%d : ", nom_ens_B, nums_B[j]);
                apercu(tab_B[j]->texte); printf("\n");
            }
        }
    }

    SEP_l();
    printf("  %s inter %s = { ", nom_ens_A, nom_ens_B);
    int premier = 1;
    for (int i = 0; i < na; i++) {
        if (matched_A[i]) {
            if (!premier) printf(", ");
            printf("P%d[%s]", nums_A[i], noms_A[i]);
            premier = 0;
        }
    }
    if (premier) printf("vide");
    printf(" }\n");

    if (!trouve)
        printf("  Resultat : { }  -- aucune paire >= %.0f%%\n", seuil * 100);
    else
        printf("  Resultat : %d paire(s) similaire(s) trouvee(s).\n", trouve);
    SEP_L();

    free(matched_A);
}

/* ----------------------------------------------------------------
   UNION des paragraghes  A union B
   ---------------------------------------------------------------- */
void parag_union(parag **tab_A, int na, int *nums_A, char (*noms_A)[50], const char *nom_ens_A,
                 parag **tab_B, int nb, int *nums_B, char (*noms_B)[50], const char *nom_ens_B,
                 float seuil, int mode)
{
    SEP_L();
    printf("  UNION PARAGRAPHES : %s union %s\n", nom_ens_A, nom_ens_B);
    printf("  Seuil %.0f%%  |  %s\n", seuil * 100, nom_mode(mode));
    SEP_l();
   /*flag_B = 1 si Pj de B a deja ete signale comme similaire
       a un paragraphe de A (evite les doublons dans l affichage final) */
    int *flag_B = calloc(nb, sizeof(int));
    if (!flag_B) return;

    printf("\n  -- Ensemble %s --\n", nom_ens_A);
    for (int i = 0; i < na; i++) {
        if (!tab_A[i]) continue;
        printf("  P%d [%s] : ", nums_A[i], noms_A[i]);
        apercu(tab_A[i]->texte); printf("\n");

        for (int j = 0; j < nb; j++) {
            if (!tab_B[j]) continue;
            float score = jaccard_score(tab_A[i], tab_B[j], mode);
            if (score >= seuil) {
                printf("       <-> similaire a P%d[%s]  (%.1f%%)\n",
                       nums_B[j], nom_ens_B, score * 100);
                flag_B[j] = 1;
            }
        }
    }

    printf("\n  -- Ensemble %s --\n", nom_ens_B);
    for (int j = 0; j < nb; j++) {
        if (!tab_B[j]) continue;
        printf("  P%d [%s] : ", nums_B[j], noms_B[j]);
        apercu(tab_B[j]->texte);
        if (flag_B[j])
            printf("  <- (similaire dans %s)", nom_ens_A);
        printf("\n");
    }

    SEP_l();
    printf("  %s union %s = { ", nom_ens_A, nom_ens_B);
    int premier = 1;
    for (int i = 0; i < na; i++) {
        if (!tab_A[i]) continue;
        if (!premier) printf(", ");
        printf("P%d[%s]", nums_A[i], noms_A[i]);
        premier = 0;
    }
    for (int j = 0; j < nb; j++) {
        if (!tab_B[j] || flag_B[j]) continue;
        if (!premier) printf(", ");
        printf("P%d[%s]", nums_B[j], noms_B[j]);
        premier = 0;
    }
    if (premier) printf("vide");
    printf(" }\n");

    printf("  Resultat : %d paragraphe(s) au total (%s=%d, %s=%d).\n",
           na + nb, nom_ens_A, na, nom_ens_B, nb);
    SEP_L();
    free(flag_B);
}

/* ----------------------------------------------------------------
   DIFFERENCE  A \ B
   Paragraphes de A sans aucun similaire dans B.
   ---------------------------------------------------------------- */
void parag_difference(parag **tab_A, int na, int *nums_A, char (*noms_A)[50], const char *nom_ens_A,
                      parag **tab_B, int nb, int *nums_B, char (*noms_B)[50], const char *nom_ens_B,
                      float seuil, int mode)
{
    SEP_L();
    printf("  DIFFERENCE PARAGRAPHES : %s \\ %s\n", nom_ens_A, nom_ens_B);
    printf("  Paragraphes de %s sans similaire dans %s\n", nom_ens_A, nom_ens_B);
    printf("  Seuil %.0f%%  |  %s\n", seuil * 100, nom_mode(mode));
    SEP_l();

    (void)noms_B; (void)nums_B;

    int *in_diff = calloc(na, sizeof(int));
    if (!in_diff) return;

    int trouve = 0;

    for (int i = 0; i < na; i++) {
        if (!tab_A[i]) continue;
        int similaire = 0;         /* on suppose que Pi n a pas de similaire dans B */

   /* parcourir tous les paragraphes de B a la recherche
           d un similaire pour Pi */
        for (int j = 0; j < nb; j++) {
            if (!tab_B[j]) continue;
            if (jaccard_score(tab_A[i], tab_B[j], mode) >= seuil) {
                similaire = 1; break;
            }
        }

        if (!similaire) {
            in_diff[i] = 1; trouve++;
            printf("  P%d [%s] : ", nums_A[i], noms_A[i]);
            apercu(tab_A[i]->texte); printf("\n");
        }
    }

    SEP_l();
    printf("  %s \\ %s = { ", nom_ens_A, nom_ens_B);
    int premier = 1;
    for (int i = 0; i < na; i++) {
        if (in_diff[i]) {
            if (!premier) printf(", ");
            printf("P%d[%s]", nums_A[i], noms_A[i]);
            premier = 0;
        }
    }
    if (premier) printf("vide");
    printf(" }\n");

    if (!trouve)
        printf("  Resultat : { }  -- tous les paragraphes de %s ont un similaire dans %s.\n",
               nom_ens_A, nom_ens_B);
    else
        printf("  Resultat : %d paragraphe(s) unique(s) dans %s.\n", trouve, nom_ens_A);
    SEP_L();

    free(in_diff);
}

/* ----------------------------------------------------------------
   SOUS-ENSEMBLE  
   Chaque paragraphe de A a-t-il au moins un similaire dans B ?
   ---------------------------------------------------------------- */
void parag_subset(parag **tab_A, int na, int *nums_A, char (*noms_A)[50], const char *nom_ens_A,
                  parag **tab_B, int nb, int *nums_B, char (*noms_B)[50], const char *nom_ens_B,
                  float seuil, int mode)
{
    SEP_L();
    printf("  SOUS-ENSEMBLE PARAGRAPHES : %s c= %s ?\n", nom_ens_A, nom_ens_B);
    printf("  Seuil %.0f%%  |  %s\n", seuil * 100, nom_mode(mode));
    SEP_l();

    (void)noms_B;

    int total = 0, couverts = 0;
    /* tester chaque paragraphe de A */
    for (int i = 0; i < na; i++) {
        if (!tab_A[i]) continue;
        total++;
        int    trouve  = 0;
        int    match_j = -1;
        float  best    = 0.0f;
    /* parcourir tous les paragraphes de B pour trouver
    le meilleur similaire de Pi (pas juste le premier) */
        for (int j = 0; j < nb; j++) {
            if (!tab_B[j]) continue;
            float sc = jaccard_score(tab_A[i], tab_B[j], mode);
             /* mettre a jour le meilleur match si ce score est plus eleve */
            if (sc >= seuil && sc > best) { best = sc; match_j = j; trouve = 1; }
        }

        printf("  P%d [%s] : ", nums_A[i], noms_A[i]);
        apercu(tab_A[i]->texte); printf("\n");

        if (trouve) {
            couverts++;
            printf("    -> [OK] similaire a P%d[%s]  (%.1f%%)\n",
                   nums_B[match_j], nom_ens_B, best * 100);
        } else {
            printf("    -> [ABSENT] aucun similaire dans %s\n", nom_ens_B);
        }
    }

    SEP_l();
    printf("  %d / %d paragraphe(s) de %s couverts dans %s\n",
           couverts, total, nom_ens_A, nom_ens_B);
    printf("  Resultat : %s %s %s\n",
           nom_ens_A,
           (couverts == total) ? "c=  (INCLUS DANS)" : "not c=  (NON INCLUS DANS)",
           nom_ens_B);
    SEP_L();
}


/* ================================================================
   MODE MOTS  --  N ENSEMBLES
   ================================================================
   - UNION        : fusion globale de tous les vocabulaires
   - INTERSECTION : itere 2 a 2 de gauche a droite
   - DIFFERENCE   : itere 2 a 2 de gauche a droi
   - SUBSET       : chaine des inclusions
*/
/* ----------------------------------------------------------------
                        INTERSECTION  
   ---------------------------------------------------------------- */
void words_intersection(Ensemble **ens, int nb_ens, float seuil)
{
    if (nb_ens < 2) { printf("  [!] Il faut au moins 2 ensembles.\n"); return; }

    SEP_L();
    printf("  INTERSECTION VOCABULAIRE (%d ensembles)\n", nb_ens);
    printf("  Seuil %.0f%%  |  Jaccard bigramL\n", seuil * 100);
    SEP_l();
        /* afficher le contenu de chaque ensemble avant le calcul */
    for (int k = 0; k < nb_ens; k++) ens_afficher(ens[k]);
    SEP_l();
        /* commencer par l intersection des deux premiers ensembles */
    token *courant = bst_intersection(ens[0]->tokens, ens[1]->tokens, seuil);
        /* intersecte le resultat courant avec chaque ensemble suivant */
    for (int k = 2; k < nb_ens; k++) {
        token *tmp = bst_intersection(courant, ens[k]->tokens, seuil);
        tk_free_all(courant);
        courant = tmp;
    }

    /* Affichage de la notation ensembliste */
    printf("  %s", ens[0]->nom);
    for (int k = 1; k < nb_ens; k++) printf(" inter %s", ens[k]->nom);
    printf(" =\n");

    if (!tk_count(courant))
        printf("  { vide }\n");
    else
        afficher_bst(courant, "resultat");

    SEP_L();
    tk_free_all(courant);
}

/* ----------------------------------------------------------------
                          UNION 
   Fusion globale (tous vocabulaires confondus).
   ---------------------------------------------------------------- */
void words_union(Ensemble **ens, int nb_ens, float seuil)
{
    (void)seuil;
    if (nb_ens < 2) { printf("  [!] Il faut au moins 2 ensembles.\n"); return; }

    SEP_L();
    printf("  UNION VOCABULAIRE (%d ensembles)\n", nb_ens);
    SEP_l();
        /* ajouter un par un les vocabulaires de chaque ensemble */
    for (int k = 0; k < nb_ens; k++) ens_afficher(ens[k]);
    SEP_l();

    token *resultat = NULL;
    for (int k = 0; k < nb_ens; k++)
         /* fusion_bst parcourt ens[k] et insere chaque mot dans resultat*/
        resultat = fusion_bst(resultat, ens[k]->tokens);

    /* Notation */
    printf("  %s", ens[0]->nom);
    for (int k = 1; k < nb_ens; k++) printf(" union %s", ens[k]->nom);
    printf(" =\n");
    afficher_bst(resultat, "resultat");
    SEP_L();

    tk_free_all(resultat);
}

/* ----------------------------------------------------------------
                          DIFFERENCE  
   ---------------------------------------------------------------- */
void words_difference(Ensemble **ens, int nb_ens, float seuil)
{
    if (nb_ens < 2) { printf("  [!] Il faut au moins 2 ensembles.\n"); return; }

    SEP_L();
    printf("  DIFFERENCE VOCABULAIRE (%d ensembles)\n", nb_ens);
    printf("  Seuil %.0f%%  |  Jaccard bigramL\n", seuil * 100);
    SEP_l();
    for (int k = 0; k < nb_ens; k++) ens_afficher(ens[k]);
    SEP_l();
        /* commencer par une copie du premier ensemble */
    token *courant = fusion_bst(NULL, ens[0]->tokens);
     // iterer de gauche a droite en difference avec chaque ensemble suivant 
    for (int k = 1; k < nb_ens; k++) {
        token *tmp = bst_difference(courant, ens[k]->tokens, seuil);
        tk_free_all(courant);
     /* le resultat de la difference devient le nouveau 
     courant pour la prochaine iteration*/
        courant = tmp;
    }

    /* Notation */
    printf("  %s", ens[0]->nom);
    for (int k = 1; k < nb_ens; k++) printf(" \\ %s", ens[k]->nom);
    printf(" =\n");

    if (!tk_count(courant))
        printf("  { vide }\n");
    else
        afficher_bst(courant, "resultat");

    SEP_L();
    tk_free_all(courant);
}

/* ----------------------------------------------------------------
   SOUS-ENSEMBLE
   Verifie la chaine d inclusion.
   ---------------------------------------------------------------- */
void words_subset(Ensemble **ens, int nb_ens, float seuil)
{
    if (nb_ens < 2) { printf("  [!] Il faut au moins 2 ensembles.\n"); return; }

    SEP_L();
    printf("  SOUS-ENSEMBLE VOCABULAIRE (%d ensembles)\n", nb_ens);
    printf("  Seuil %.0f%%  |  Jaccard bigramL\n", seuil * 100);
    SEP_l();
    for (int k = 0; k < nb_ens; k++) ens_afficher(ens[k]);
    // on suppose que toute la chaine est vraie jusqu a preuve du contraire 
    int chaine_ok = 1;  
    // tester chaque paire consecutive (Ek, Ek+1) 
    for (int k = 0; k < nb_ens - 1; k++) {
        Ensemble *EA = ens[k];
        Ensemble *EB = ens[k + 1];

        SEP_l();
        printf("  Verification : %s c= %s ?\n", EA->nom, EB->nom);
        SEP_l();

        token *marques = NULL;
        int    total   = 0, couverts = 0;
        // parcours inorder iteratif de EA->tokens 
        token *stack[4096]; int top = 0; token *cur = EA->tokens;
        while (cur || top > 0) {
            // descendre jusqu au noeud le plus a gauche 

            while (cur) { stack[top++] = cur; cur = cur->lc; }
            cur = stack[--top];
            total++;
            //chercher un equivalent de mot dans EB
            token *match = search_similar(cur, EB->tokens, seuil, &marques);
            if (match) {
                couverts++;
                marques = tk_insert(marques, match->word);
                if (seuil >= 1.0f)
                    printf("  %-25s  [OK]\n", cur->word);
                else
                    printf("  %-25s  [OK] ~ %s\n", cur->word, match->word);
            } else {
                printf("  %-25s  [ABSENT]\n", cur->word);
            }
             // passer au sous-arbre droit 
            cur = cur->rc;
        }
        tk_free_all(marques);

        int ok = (couverts == total);
        if (!ok) chaine_ok = 0;
        printf("  %d / %d mots de %s presents dans %s\n",
               couverts, total, EA->nom, EB->nom);
        printf("  %s %s %s\n",
               EA->nom,
               ok ? "c=  (INCLUS DANS)" : "not c=  (NON INCLUS DANS)",
               EB->nom);
    }
     // affichage
    SEP_l();
    if (nb_ens > 2) {
        printf("  Resultat global : ");
        printf("%s", ens[0]->nom);
        for (int k = 1; k < nb_ens; k++) printf(" c= %s", ens[k]->nom);
        printf(" : %s\n", chaine_ok ? "VRAIE" : "FAUSSE");
    }
    SEP_L();
}