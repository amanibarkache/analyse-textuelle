#ifndef OPERATIONS_H
#define OPERATIONS_H

#include "tree.h"
#include "ensemble.h"

/*
 * ================================================================
 * MODE PARAGRAPHES
 *
 * Comparaison deux a deux via Jaccard (mode 0=tokens, mode 1=bigramW).
 * Pour N ensembles (N>2), on itere 2 a 2 de gauche a droite.
 * ================================================================
 */
void parag_intersection(parag **tab_A, int na, int *nums_A, char (*noms_A)[50], const char *nom_ens_A,
                        parag **tab_B, int nb, int *nums_B, char (*noms_B)[50], const char *nom_ens_B,
                        float seuil, int mode);

void parag_union       (parag **tab_A, int na, int *nums_A, char (*noms_A)[50], const char *nom_ens_A,
                        parag **tab_B, int nb, int *nums_B, char (*noms_B)[50], const char *nom_ens_B,
                        float seuil, int mode);

void parag_difference  (parag **tab_A, int na, int *nums_A, char (*noms_A)[50], const char *nom_ens_A,
                        parag **tab_B, int nb, int *nums_B, char (*noms_B)[50], const char *nom_ens_B,
                        float seuil, int mode);

void parag_subset      (parag **tab_A, int na, int *nums_A, char (*noms_A)[50], const char *nom_ens_A,
                        parag **tab_B, int nb, int *nums_B, char (*noms_B)[50], const char *nom_ens_B,
                        float seuil, int mode);

/*
 * ================================================================
 * MODE MOTS -- N ensembles
 *
 * Intersection/Difference : itere 2 a 2 (resultat = R inter Ei)
 * Union                   : fusion globale de tous les vocabulaires
 * Subset                  : verifie A(0) c= A(1) c= ... c= A(N-1)
 * Similarite : Jaccard bigramL.
 * ================================================================
 */
void words_intersection(Ensemble **ens, int nb_ens, float seuil);
void words_union       (Ensemble **ens, int nb_ens, float seuil);
void words_difference  (Ensemble **ens, int nb_ens, float seuil);
void words_subset      (Ensemble **ens, int nb_ens, float seuil);

#endif