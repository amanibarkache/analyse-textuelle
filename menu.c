#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "menu.h"
#include "lecture.h"

/* ================================================================
   VIDER LE BUFFER STDIN
   ================================================================ */
static void vider_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* ================================================================
   CONSOLE
   ================================================================ */
void cls(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void sep_lourde(void) { printf("=========================================================\n"); }
void sep_legere(void) { printf("---------------------------------------------------------\n"); }

void menu_pause(void)
{
    printf("\n  Appuyez sur Entree pour continuer...");
    fflush(stdout);
    getchar();
}

/* ================================================================
   BANNIERE
   ================================================================ */
void menu_banniere(void)
{
    sep_lourde();
    printf("     ANALYSE TEXTUELLE PAR SIMILARITE JACCARD\n");
    printf("     Tokens | BigramW | BigramL | BST\n");
    sep_lourde();
    printf("\n");
}

/* ================================================================
   MENU PRINCIPAL
   ================================================================ */
int menu_principal(void)
{
    int choix = 0;

    cls();
    menu_banniere();
    sep_legere();
    printf("  MENU PRINCIPAL\n");
    sep_legere();
    printf("  1. Charger un ou plusieurs fichiers\n");
    printf("  2. Afficher les fichiers charges\n");
    printf("  3. Operations sur les paragraphes:(les elements des resultats sont des paragraghes)\n");
    printf("  4. Operations sur le vocabulaire (les elements des resultats sont des paragraghes)\n");
    printf("  5. Changer la langue (stopwords)\n");
    printf("  6. Liberer un fichier specifique\n");
    printf("  7. Liberer tous les fichiers\n");
    printf("  0. Quitter\n");
    sep_legere();

    if (nbfiles > 0) {
        printf("  Fichiers en memoire : %d  |  ", nbfiles);
        for (int i = 0; i < nbfiles && i < 4; i++)
            printf("[%d]%s  ", i + 1, T[i] ? T[i]->nom : "?");
        if (nbfiles > 4) printf("...");
        printf("\n");
        sep_legere();
    }

    printf("  Votre choix : ");
    if (scanf("%d", &choix) != 1) choix = -1;
    vider_buffer();
    return choix;
}

/* ================================================================
   SOUS-MENU OPERATION
   ================================================================ */
int menu_operations(void)
{
    int choix = 0;

    printf("\n");
    sep_legere();
    printf("  OPERATION ENSEMBLISTE\n");
    sep_legere();
    printf("  1. Intersection  (A inter B inter ...)\n");
    printf("  2. Union         (A union B union ...)\n");
    printf("  3. Difference    (A \\ B \\ ...)\n");
    printf("  4. Sous-ensemble (A c= B c= ... ?)\n");
    printf("  0. Retour\n");
    sep_legere();
    printf("  Votre choix : ");

    if (scanf("%d", &choix) != 1) choix = -1;
    vider_buffer();
    return choix;
}

/* ================================================================
   SOUS-MENU MODE JACCARD
   ================================================================ */
int menu_mode(void)
{
    int choix = 0;

    printf("\n");
    sep_legere();
    printf("  MESURE DE SIMILARITE\n");
    sep_legere();
    printf("  0. Jaccard tokens  (mots exacts)\n");
    printf("  1. Jaccard bigramW ( expressions)\n");
    sep_legere();
    printf("  Votre choix : ");

    if (scanf("%d", &choix) != 1) choix = 0;
    vider_buffer();
    return (choix == 1) ? 1 : 0;
}

/* ================================================================
   AFFICHER LES FICHIERS CHARGES
   ================================================================ */
void menu_afficher_fichiers(void)
{
    cls();
    menu_banniere();
    sep_legere();
    printf("  FICHIERS CHARGES (%d)\n", nbfiles);
    sep_legere();

    if (nbfiles == 0) {
        printf("  Aucun fichier charge.\n");
    } else {
        for (int i = 0; i < nbfiles; i++) {
            if (T[i])
                printf("  [%d] %-35s  %d paragraphe(s)\n",
                       i + 1, T[i]->nom, T[i]->nbParag);
        }
    }
    sep_legere();
}

/* ================================================================
   CHARGER UN OU PLUSIEURS FICHIERS
   ================================================================ */
int menu_charger_fichiers(void)
{
    int nb_a_charger = 1;
    int use_stpW_int = 0;

    cls();
    menu_banniere();
    sep_legere();
    printf("  CHARGEMENT DE FICHIERS\n");
    sep_legere();

    printf("  Combien de fichiers a charger ? : ");
    if (scanf("%d", &nb_a_charger) != 1 || nb_a_charger < 1) {
        vider_buffer();
        printf("  [!] Nombre invalide.\n");
        return -1;
    }
    vider_buffer();

    printf("  Filtrer les stopwords ? (1=Oui  0=Non) : ");
    if (scanf("%d", &use_stpW_int) != 1) use_stpW_int = 0;
    vider_buffer();

    bool use_stpW = (use_stpW_int == 1);
    int  charges  = 0;

    for (int k = 0; k < nb_a_charger; k++) {
        char chemin[256];
        printf("\n  Fichier %d/%d\n  Chemin : ", k + 1, nb_a_charger);
        if (scanf("%255s", chemin) != 1) { vider_buffer(); continue; }
        vider_buffer();

        file **tmp = realloc(T, (nbfiles + 1) * sizeof(file *));
        if (!tmp) { fprintf(stderr, "  [!] Erreur memoire.\n"); continue; }
        T = tmp;

        T[nbfiles] = lire_fichier(chemin, use_stpW);
        if (!T[nbfiles]) {
            printf("  [!] Echec : %s\n", chemin);
            continue;
        }
        nbfiles++;
        charges++;
        printf("  [OK] Index [%d] assigne.\n", nbfiles);
    }

    printf("\n  Bilan : %d/%d fichier(s) charge(s).\n", charges, nb_a_charger);
    return charges;
}

/* ================================================================
   LIBERER UN FICHIER SPECIFIQUE
   ================================================================ */
void menu_liberer_un_fichier(void)
{
    cls();
    menu_banniere();

    if (nbfiles == 0) { printf("  Aucun fichier en memoire.\n"); return; }

    menu_afficher_fichiers();

    printf("  Index du fichier a liberer (1-%d) : ", nbfiles);
    int idx;
    if (scanf("%d", &idx) != 1) { vider_buffer(); return; }
    vider_buffer();
    idx--;

    if (idx < 0 || idx >= nbfiles || !T[idx]) {
        printf("  [!] Index invalide.\n");
        return;
    }

    printf("  Liberation de [%s]...\n", T[idx]->nom);
    file_free(T[idx]);

    for (int i = idx; i < nbfiles - 1; i++)
        T[i] = T[i + 1];
    T[nbfiles - 1] = NULL;
    nbfiles--;

    if (nbfiles > 0) {
        file **tmp = realloc(T, nbfiles * sizeof(file *));
        if (tmp) T = tmp;
    } else {
        free(T);
        T = NULL;
    }

    printf("  [OK] Fichier libere. %d fichier(s) restant(s).\n", nbfiles);
}

/* ================================================================
   LIBERER TOUS LES FICHIERS
   ================================================================ */
void menu_tout_liberer(void)
{
    if (nbfiles == 0) { printf("  Rien a liberer.\n"); return; }
    for (int i = 0; i < nbfiles; i++) { file_free(T[i]); T[i] = NULL; }
    free(T);
    T       = NULL;
    nbfiles = 0;
    printf("  [OK] Tous les fichiers liberes.\n");
}

/* ================================================================
   CHOISIR LE SEUIL JACCARD
   ================================================================ */
float menu_choisir_seuil(void)
{
    float seuil = 0.5f;
    printf("  Seuil Jaccard [0.0 a 1.0]  (defaut 0.5) : ");
    if (scanf("%f", &seuil) != 1) seuil = 0.5f;
    vider_buffer();
    if (seuil < 0.0f) seuil = 0.0f;
    if (seuil > 1.0f) seuil = 1.0f;
    return seuil;
}

/* ================================================================
   CHOISIR LA LANGUE
   ================================================================ */
int menu_choisir_langue(void)
{
    int langue = 1;
    cls();
    menu_banniere();
    sep_legere();
    printf("  LANGUE DES STOPWORDS\n");
    sep_legere();
    printf("  1. Francais\n");
    printf("  2. Anglais\n");
    sep_legere();
    printf("  Votre choix : ");
    if (scanf("%d", &langue) != 1) langue = 1;
    vider_buffer();
    if (langue != 1 && langue != 2) langue = 1;
    return langue;
}

/* ================================================================
   APERCU D UN PARAGRAPHE
   ================================================================ */
static void apercu_parag(parag *p, int num, const char *nom_f)
{
    char buf[58];
    strncpy(buf, p->texte, 54);
    buf[54] = '\0';
    for (int j = 0; buf[j]; j++)
        if (buf[j] == '\n' || buf[j] == '\r') buf[j] = ' ';
    printf("    P%-2d [%-15s] : %s...\n", num, nom_f, buf);
}

/* ================================================================
   SELECTION INTERACTIVE DE PARAGRAPHES
   ================================================================ */
parag **menu_selectionner_parags(int *n_out, int **nums_out,
                                 char (**noms_out)[50],
                                 const char *nom_ens)
{
    *n_out    = 0;
    *nums_out = NULL;
    *noms_out = NULL;

    int     cap   = 16;
    parag **tab   = malloc(cap * sizeof(parag *));
    int    *nums  = malloc(cap * sizeof(int));
    char  (*noms)[50] = malloc(cap * sizeof(*noms));
    if (!tab || !nums || !noms) {
        fprintf(stderr, "[!] Memoire : menu_selectionner_parags\n");
        exit(1);
    }

    int encore = 1;

    while (encore) {
        cls();
        menu_banniere();
        sep_lourde();
        printf("  SELECTION DE PARAGRAPHES -- Ensemble %s\n", nom_ens);
        sep_lourde();
        printf("  Paragraphes selectionnes : %d\n", *n_out);
        sep_legere();
        printf("  1. Ajouter des paragraphes depuis un fichier\n");
        printf("  2. Terminer la selection\n");
        sep_legere();
        printf("  Votre choix : ");

        int choix = 0;
        if (scanf("%d", &choix) != 1) choix = 2;
        vider_buffer();

        if (choix != 1) { encore = 0; continue; }

        menu_afficher_fichiers();
        if (nbfiles == 0) {
            printf("  [!] Aucun fichier charge.\n");
            encore = 0;
            continue;
        }

        printf("  Choisir le fichier source (1-%d) : ", nbfiles);
        int fidx;
        if (scanf("%d", &fidx) != 1) { vider_buffer(); continue; }
        vider_buffer();
        fidx--;

        if (fidx < 0 || fidx >= nbfiles || !T[fidx]) {
            printf("  [!] Index invalide.\n");
            continue;
        }

        file *F = T[fidx];

        printf("\n");
        sep_legere();
        printf("  Paragraphes de [%s] :\n", F->nom);
        sep_legere();
        for (int i = 0; i < F->nbParag; i++)
            if (F->paragraphe[i])
                apercu_parag(F->paragraphe[i], i + 1, F->nom);
        sep_legere();
        printf("  0  = tous les paragraphes\n");
        printf("  ou entrez les indices (ex: 1 3 5), terminez par -1\n");
        sep_legere();
        printf("  > ");

        int premier;
        if (scanf("%d", &premier) != 1) { vider_buffer(); continue; }

        if (premier == 0) {
            vider_buffer();
            for (int i = 0; i < F->nbParag; i++) {
                if (!F->paragraphe[i]) continue;
                int deja = 0;
                for (int k = 0; k < *n_out; k++)
                    if (tab[k] == F->paragraphe[i]) { deja = 1; break; }
                if (deja) { printf("  [!] P%d deja selectionne, ignore\n", i+1); continue; }
                if (*n_out >= cap) {
                    cap *= 2;
                    tab  = realloc(tab,  cap * sizeof(parag *));
                    nums = realloc(nums, cap * sizeof(int));
                    noms = realloc(noms, cap * sizeof(*noms));
                }
                tab[*n_out]  = F->paragraphe[i];
                nums[*n_out] = i + 1;
                strncpy(noms[*n_out], F->nom, 49);
                noms[*n_out][49] = '\0';
                (*n_out)++;
                printf("  [+] P%d [%s] ajoute\n", i + 1, F->nom);
            }
        } else {
            int idx = premier;
            while (idx != -1) {
                int i = idx - 1;
                if (i >= 0 && i < F->nbParag && F->paragraphe[i]) {
                    int deja = 0;
                    for (int k = 0; k < *n_out; k++)
                        if (tab[k] == F->paragraphe[i]) { deja = 1; break; }
                    if (!deja) {
                        if (*n_out >= cap) {
                            cap *= 2;
                            tab  = realloc(tab,  cap * sizeof(parag *));
                            nums = realloc(nums, cap * sizeof(int));
                            noms = realloc(noms, cap * sizeof(*noms));
                        }
                        tab[*n_out]  = F->paragraphe[i];
                        nums[*n_out] = i + 1;
                        strncpy(noms[*n_out], F->nom, 49);
                        noms[*n_out][49] = '\0';
                        (*n_out)++;
                        printf("  [+] P%d [%s] ajoute\n", i + 1, F->nom);
                    } else {
                        printf("  [!] P%d deja selectionne, ignore\n", idx);
                    }
                } else {
                    printf("  [!] P%d invalide, ignore\n", idx);
                }
                if (scanf("%d", &idx) != 1) { vider_buffer(); break; }
            }
            vider_buffer();
        }
    }

    if (*n_out == 0) {
        free(tab); free(nums); free(noms);
        return NULL;
    }

    printf("\n  [OK] Ensemble %s : %d paragraphe(s) selectionne(s).\n",
           nom_ens, *n_out);

    *nums_out = nums;
    *noms_out = noms;
    return tab;
}

/* ================================================================
   CONSTRUCTION D UN ENSEMBLE DE MOTS (helper interne)
   ================================================================ */
static Ensemble *construire_un_ensemble(const char *nom_ens, int ens_index)
{
    Ensemble *E = ens_creer(nom_ens);

    cls();
    menu_banniere();
    sep_lourde();
    printf("  REMPLISSAGE DE L'ENSEMBLE %s (ensemble %d)\n", nom_ens, ens_index);
    sep_lourde();

    int encore = 1;

    while (encore) {
        printf("\n");
        sep_legere();
        printf("  Contenu actuel de l'ensemble %s : %d mot(s)\n",
               nom_ens, E->nb_tokens);
        if (E->nb_parags > 0) ens_afficher_sources(E);
        sep_legere();
        printf("  1. Ajouter des paragraphes depuis un fichier\n");
        printf("  2. Terminer cet ensemble\n");
        sep_legere();
        printf("  Votre choix : ");

        int choix = 0;
        if (scanf("%d", &choix) != 1) choix = 2;
        vider_buffer();

        if (choix != 1) { encore = 0; continue; }

        menu_afficher_fichiers();
        if (nbfiles == 0) { encore = 0; continue; }

        printf("  Choisir le fichier source (1-%d) : ", nbfiles);
        int fidx;
        if (scanf("%d", &fidx) != 1) { vider_buffer(); continue; }
        vider_buffer();
        fidx--;

        if (fidx < 0 || fidx >= nbfiles || !T[fidx]) {
            printf("  [!] Index invalide.\n");
            continue;
        }

        file *F = T[fidx];

        printf("\n");
        sep_legere();
        printf("  Paragraphes de [%s] :\n", F->nom);
        sep_legere();
        for (int i = 0; i < F->nbParag; i++)
            if (F->paragraphe[i]) {
                char buf[56];
                strncpy(buf, F->paragraphe[i]->texte, 52);
                buf[52] = '\0';
                for (int j = 0; buf[j]; j++)
                    if (buf[j]=='\n'||buf[j]=='\r') buf[j]=' ';
                printf("    P%-2d : %s...\n", i + 1, buf);
            }
        sep_legere();
        printf("  0 = tous  |  indices ex: 1 3 5  |  terminer par -1\n");
        printf("  > ");

        int premier;
        if (scanf("%d", &premier) != 1) { vider_buffer(); continue; }

        if (premier == 0) {
            vider_buffer();
            for (int i = 0; i < F->nbParag; i++) {
                if (!F->paragraphe[i]) continue;
                int deja = 0;
                for (int k = 0; k < E->nb_parags; k++)
                    if (E->parags[k] == F->paragraphe[i]) { deja = 1; break; }
                if (!deja) {
                    ens_ajouter_parag(E, F->paragraphe[i], i + 1, F->nom);
                    printf("  [+] P%d [%s]\n", i + 1, F->nom);
                }
            }
        } else {
            int idx = premier;
            while (idx != -1) {
                int i = idx - 1;
                if (i >= 0 && i < F->nbParag && F->paragraphe[i]) {
                    int deja = 0;
                    for (int k = 0; k < E->nb_parags; k++)
                        if (E->parags[k] == F->paragraphe[i]) { deja = 1; break; }
                    if (!deja) {
                        ens_ajouter_parag(E, F->paragraphe[i], i + 1, F->nom);
                        printf("  [+] P%d [%s]\n", i + 1, F->nom);
                    } else {
                        printf("  [!] P%d deja dans l'ensemble\n", idx);
                    }
                } else {
                    printf("  [!] P%d invalide\n", idx);
                }
                if (scanf("%d", &idx) != 1) { vider_buffer(); break; }
            }
            vider_buffer();
        }
    }

    printf("  [OK] Ensemble %s : %d mot(s) depuis %d paragraphe(s).\n",
           E->nom, E->nb_tokens, E->nb_parags);
    return E;
}

/* ================================================================
   CONSTRUCTION DE N ENSEMBLES DE MOTS
   ================================================================ */
Ensemble **menu_construire_ensembles(int *nb_ens)
{
    *nb_ens = 0;

    cls();
    menu_banniere();
    sep_legere();
    printf("  CREATION DES ENSEMBLES\n");
    sep_legere();
    printf("  Combien d'ensembles ? (min 2, max %d) : ", MAX_ENSEMBLES);

    int nb = 2;
    if (scanf("%d", &nb) != 1 || nb < 2) nb = 2;
    vider_buffer();
    if (nb > MAX_ENSEMBLES) nb = MAX_ENSEMBLES;

    Ensemble **liste = calloc(nb, sizeof(Ensemble *));
    if (!liste) { fprintf(stderr, "[!] Memoire ensembles\n"); exit(1); }

    for (int k = 0; k < nb; k++) {
        char nom[NOM_ENS_MAX];
        if (nb <= 26)
            snprintf(nom, NOM_ENS_MAX, "%c", 'A' + k);
        else
            snprintf(nom, NOM_ENS_MAX, "E%d", k + 1);

        liste[k] = construire_un_ensemble(nom, k + 1);
        (*nb_ens)++;
    }

    return liste;
}