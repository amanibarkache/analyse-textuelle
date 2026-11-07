#ifndef TREE_H
#define TREE_H

#include <stdbool.h>

/* ================================================================
   STRUCTURES DE DONNEES
   ================================================================ */

/* Noeud BST -- bigramme de deux lettres */
typedef struct bigramLetter {
    char lettre1;
    char lettre2;
    struct bigramLetter *lc;
    struct bigramLetter *rc;
} bigramL;

/* Noeud BST -- bigramme de deux mots */
typedef struct bigramWord {
    char word1[50];
    char word2[50];
    struct bigramWord *lc;
    struct bigramWord *rc;
} bigramW;

/* Noeud BST -- token (mot unique) avec son BST de bigrammes de lettres */
typedef struct token {
    char    word[50];
    bigramL *RbigramL;
    struct  token *lc;
    struct  token *rc;
} token;

/* Paragraphe : texte brut + BST tokens + BST bigrammes de mots */
typedef struct parag {
    char    *texte;
    token   *Rtoken;
    bigramW *RbigramW;
} parag;

/* Fichier : nom + tableau dynamique de paragraphes */
typedef struct file {
    char   nom[50];
    int    nbParag;
    parag **paragraphe;
} file;

/* Matrice globale des fichiers charges */
extern file **T;
extern int    nbfiles;

/* ================================================================
   GESTION DE LA LANGUE (stopwords)
   ================================================================ */
void lang(int langue);   /* 1 = Francais, 2 = Anglais */

/* ================================================================
   MACHINE ABSTRAITE -- bigramL
   ================================================================ */
bigramL *allocate_bigramL(char l1, char l2);

void     bigramL_free_all(bigramL *R);
bigramL *bigramL_insert(bigramL *R, char l1, char l2);
int      bigramL_search(bigramL *R, char l1, char l2);
int      bigramL_count(bigramL *R);

/* ================================================================
   MACHINE ABSTRAITE -- bigramW
   ================================================================ */
bigramW *allocate_bigramW(const char *w1, const char *w2);

void     bigramW_free_all(bigramW *R);
bigramW *bigramW_insert(bigramW *R, const char *w1, const char *w2);
int      bigramW_search(bigramW *R, const char *w1, const char *w2);
int      bigramW_count(bigramW *R);


/* ================================================================
   MACHINE ABSTRAITE -- token
   ================================================================ */
token *allocate_tk(const char *word);
void   tk_build_bigramL(token *p);

void   tk_free_all(token *R);
token *tk_insert(token *R, const char *word);
int    tk_search(token *R, const char *word);
int    tk_count(token *R);


/* ================================================================
   MACHINE ABSTRAITE -- parag
   ================================================================ */
parag *parag_allocate(const char *texte);
void   parag_build(parag *p, bool use_stpW);
void   parag_free(parag *p);

/* ================================================================
   MACHINE ABSTRAITE -- file
   ================================================================ */
file *allocate_file(const char *nom, int nbParag);
void  file_free(file *f);

/* ================================================================
   SCORES JACCARD
   ================================================================ */
int   tk_inter(token *RA, token *RB);
int   tk_union(token *RA, token *RB);
float jaccard_tk(parag *A, parag *B);

int   bigramW_inter(bigramW *RA, bigramW *RB);
float jaccard_bigramW(parag *A, parag *B);

float jaccard_bigramL(token *A, token *B);

#endif /* TREE_H */