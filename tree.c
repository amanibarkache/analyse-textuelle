
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tree.h"

/* ================================================================
   MY_STRDUP : equivalent de strdup absent du standart C99
   pour que le code compile dans toutes les versions et evider les warnings
   ================================================================ */
static char *my_strdup(const char *src)
//allouer une copie de src et la retourne
{
    if (!src) 
        return NULL;
    size_t len = strlen(src) + 1; //pour \0
    char  *dst = malloc(len);
    if (!dst) return NULL;
    memcpy(dst, src, len);//copie octet par octet \0 inclus
    return dst;
}


//    ****  VARIABLES GLOBALES  ****
 
file **T       = NULL;
int    nbfiles = 0;

/* ================================================================
   STOPWORDS
   ================================================================ */
static const char *STPW_FR[] = {
    "au","aux","avec","ce","ces","cet","cette","dans","de","des",
    "du","elle","elles","en","et","eux","il","ils","je","la","le",
    "les","leur","leurs","lui","ma","mais","me","meme","mes","mon",
    "ni","nos","notre","nous","on","ou","par","pas","pour","qu",
    "que","qui","sa","se","ses","si","son","sur","ta","te","tes",
    "ton","tu","un","une","vos","votre","vous","y","a","ete","etre",
    "fait","plus","tout","dont","car","donc","or","ne","n","l","d",
    NULL
};

static const char *STPW_EN[] = {
    "a","about","above","after","again","against","all","am","an",
    "and","any","are","as","at","be","because","been","before",
    "being","below","between","both","but","by","can","did","do",
    "does","doing","down","during","each","few","for","from",
    "further","get","had","has","have","having","he","her","here",
    "hers","herself","him","himself","his","how","i","if","in","into",
    "is","it","its","itself","just","me","more","most","my","myself",
    "no","nor","not","now","of","off","on","once","only","or","other",
    "our","ours","ourselves","out","over","own","s","same","she",
    "should","so","some","such","t","than","that","the","their",
    "theirs","them","themselves","then","there","these","they","this",
    "those","through","to","too","under","until","up","us","very",
    "was","we","were","what","when","where","which","while","who",
    "whom","why","will","with","would","you","your","yours",
    "yourself","yourselves",
    NULL
};

static const char **stpW = NULL;

void lang(int langue)
//definir la liste de stopwords selon la langue
{
    stpW = (langue == 1) ? STPW_FR : STPW_EN;
}

//retourner 1 si le mot est un stopword sinon 0
static int is_stpW(const char *word)
{
    if (!stpW) 
        return 0;//pas de langue definie
    for (int i = 0; stpW[i]; i++)
        if (strcmp(word, stpW[i]) == 0) 
            return 1;//trouve dans la liste
    return 0;
}

/* ================================================================
   NORMALISATION
   Convertitr en miniscule 
   Supprimer la ponctuation
   Traiter l'apostrophe comme separateur
   ================================================================ */


//Tableau de conversion UTF-8 en ASCCI
//chaque entree contient les octets UTF-8 de l accent et la lettre ASCCI equivalente
static const struct { unsigned char b0,b1,b2; char ascii; } ACCENT_MAP[] = 
{
    /* a/A */
    {0xc3,0xa0,0,'a'},{0xc3,0xa1,0,'a'},{0xc3,0xa2,0,'a'},{0xc3,0xa3,0,'a'},
    {0xc3,0xa4,0,'a'},{0xc3,0xa5,0,'a'},
    {0xc3,0x80,0,'a'},{0xc3,0x81,0,'a'},{0xc3,0x82,0,'a'},{0xc3,0x83,0,'a'},
    {0xc3,0x84,0,'a'},{0xc3,0x85,0,'a'},
    /* e/E */
    {0xc3,0xa8,0,'e'},{0xc3,0xa9,0,'e'},{0xc3,0xaa,0,'e'},{0xc3,0xab,0,'e'},
    {0xc3,0x88,0,'e'},{0xc3,0x89,0,'e'},{0xc3,0x8a,0,'e'},{0xc3,0x8b,0,'e'},
    /* i/I */
    {0xc3,0xac,0,'i'},{0xc3,0xad,0,'i'},{0xc3,0xae,0,'i'},{0xc3,0xaf,0,'i'},
    {0xc3,0x8c,0,'i'},{0xc3,0x8d,0,'i'},{0xc3,0x8e,0,'i'},{0xc3,0x8f,0,'i'},
    /* o/O */
    {0xc3,0xb2,0,'o'},{0xc3,0xb3,0,'o'},{0xc3,0xb4,0,'o'},{0xc3,0xb5,0,'o'},
    {0xc3,0xb6,0,'o'},
    {0xc3,0x92,0,'o'},{0xc3,0x93,0,'o'},{0xc3,0x94,0,'o'},{0xc3,0x95,0,'o'},
    {0xc3,0x96,0,'o'},
    /* u/U */
    {0xc3,0xb9,0,'u'},{0xc3,0xba,0,'u'},{0xc3,0xbb,0,'u'},{0xc3,0xbc,0,'u'},
    {0xc3,0x99,0,'u'},{0xc3,0x9a,0,'u'},{0xc3,0x9b,0,'u'},{0xc3,0x9c,0,'u'},
    /* c cedille */
    {0xc3,0xa7,0,'c'},{0xc3,0x87,0,'c'},
    /* n tilde */
    {0xc3,0xb1,0,'n'},{0xc3,0x91,0,'n'},
    {0,0,0,0}
};

static char accent_to_ascii(const unsigned char *s, int *consumed)
{//cherche s[] dans ACCENT_MAP[] et retourne la lettre ASCII equivalente
//consumed recoit le nbr d octets lus 
//retourne 0 si le char n est pas dans le tableau
    for (int k=0; ACCENT_MAP[k].ascii; k++) {
        if (s[0]==ACCENT_MAP[k].b0 && s[1]==ACCENT_MAP[k].b1) {
            if (ACCENT_MAP[k].b2==0) {    
                *consumed=2;  //accent sur 2 octets,on avance de 2
                return ACCENT_MAP[k].ascii;
            }
            if (s[2]==ACCENT_MAP[k].b2) { 
                *consumed=3; //accent sur 3 octets,on avance de 3
                return ACCENT_MAP[k].ascii;
            }
        }
    }
    return 0;//char non trouve dans ACCENT_MAP
}

static void normalize(char *word)
/* convertir un mot en min ASCII:
    remplacer les lettres avec des accents par leur equivalent ascii
    ignorer les chiffres et ponctuation
   sauter les les char non definie avant
*/
{
    int j = 0, i = 0;//j pst d ecriture et i pst de lecture
    while (word[i]) {
        unsigned char c = (unsigned char)word[i];
        if (c < 128) {  
            //char ascci normal
            
            if (c>='A' && c<='Z')
                 word[j++] = (char)(c+32);//maj en min
            else if (c>='a' && c<='z') word[j++] = (char)c;//deja min on garde
            
            //sinon c est un chiffre ou ponctuatuation on ignore
            i++;
        } 
            else {
                // char UTF-8 multi octets
            int consumed=0;
            char ascii = accent_to_ascii((const unsigned char *)(word + i), &consumed);
            if (ascii) {
                word[j++] = ascii; //on remplace l accent par sa lettre de base
                i += consumed;//sauter les octets qu on vient de lire
            } else {
                //caractere qu on connait pas on saute tous ses octets 
                i++;
                while ((unsigned char)word[i] >= 0x80 &&
                       (unsigned char)word[i] < 0xc0) i++;
            }
        }
    }
    word[j] = '\0';
}

static void tokenize(const char *texte,void (*callback)(const char *, void *),void *ctx)
// decoupe le texte en mots et appelle callback() sur chaque mot trouve.
// ctx est le contexte transmis tel quel a callback .
// l apostrophe est traite comme un separateur de mots. 
{
    char *copy = my_strdup(texte);//on travaille sur une copie car strtok va modifier la chaine
    if (!copy) return;

     

    for (int i=0;copy[i]; ) ////transformer les ' en espaces pour que strtok les traite comme des separateurs
    {
        unsigned char c = (unsigned char)copy[i];
        if (c == '\'' || c == '`') 
        {
            copy[i] = ' '; i++;
        } 
        else  if (c == 0xe2 && (unsigned char)copy[i+1] == 0x80 && (unsigned char)copy[i+2] == 0x99) 
        {
            // l apostrophe prend 3 octet donc on met 3 espaces
            copy[i] = copy[i+1] = copy[i+2] = ' ';
            i += 3;
        } else {
            i++;
        }
    }

    //decouper sur les separateurs et on envoie chaque mot a callback

    char *tok = strtok(copy, " \t\n\r,;:!?.\"()[]{}");
    while (tok) {
        normalize(tok);
        if (strlen(tok) > 1)  //ignorer les mots d'une seule lettre apres nettoyage
            callback(tok, ctx);//envoyer le mot a build_callback
        tok = strtok(NULL, " \t\n\r,;:!?.\"()[]{}");
    }
    free(copy);//liberer la copie
}

/* ================================================================
   bigramL
   ================================================================*/

bigramL *allocate_bigramL(char l1, char l2)
// allouer un noeud bigramL et l 'initailiser
{
    bigramL *p = malloc(sizeof(bigramL));
    if (!p) {
         fprintf(stderr, "malloc bigramL\n");
          exit(1); 
        }
    p->lettre1 = l1; 
    p->lettre2 = l2;
    p->lc = p->rc = NULL;
    return p;
}



void bigramL_free_all(bigramL *R)
//liberer  tout le bst de bigramL
{
    if (!R) return;
    bigramL_free_all(R->lc);
    bigramL_free_all(R->rc);
    free(R);
}

static int cmp_bigramL(bigramL *p, char l1, char l2)
//comparer un noeud bigramL avec les lettres (l1,l2) et retourner 0,nombre + ou -

{
    //on compare la premiere lettre
    if (p->lettre1 != l1)
        return (unsigned char)l1 - (unsigned char)p->lettre1;
//si elles sont egales on departage avec  la 2eme lettre
    return (unsigned char)l2 - (unsigned char)p->lettre2;
}

bigramL *bigramL_insert(bigramL *R, char l1, char l2)
//inserer les lettre (l1,l2) dans le bst ou ignorer si elles existent deja
{
    if (!R) {
     return allocate_bigramL(l1, l2);//trouver la bonne place
    }
    int cmp = cmp_bigramL(R, l1, l2);
    if      (cmp < 0) 
        R->lc = bigramL_insert(R->lc, l1, l2);
    else if (cmp > 0)
         R->rc = bigramL_insert(R->rc, l1, l2);
    return R;
    // si cmp==0 le bigramme existe deja,on ne fait trien
}

int bigramL_search(bigramL *R, char l1, char l2)
//retourne 1 si la paire(l1,l2) est dans le bst sinon 0
{
    if (!R) 
     return 0;
    int cmp = cmp_bigramL(R,l1,l2);
    if (cmp == 0)
      return 1;
    if (cmp < 0)  
     return bigramL_search(R->lc, l1, l2);
    return bigramL_search(R->rc, l1, l2);
}

int bigramL_count(bigramL *R)
//retourne le nombre de noeuds dans le bst
{
    if (!R)
         return 0;
//le noeud + tout ce qu il y a de chaque cote
    return 1 + bigramL_count(R->lc) + bigramL_count(R->rc);
}







/* ================================================================
   bigramW
   ================================================================ */

bigramW *allocate_bigramW(const char *w1, const char *w2)
//alloue un noeud bigramW (paire de mots) et l initialise 

{
    bigramW *p = malloc(sizeof(bigramW));
    if (!p) 
    {
         fprintf(stderr, "malloc bigramW\n"); 
         exit(1); 
        }
    strncpy(p->word1, w1, 49);
     p->word1[49] = '\0';//copie mot1 et force le \0 par precotion
    strncpy(p->word2, w2, 49); 
    p->word2[49] = '\0';
    p->lc = p->rc = NULL;
    return p;
}

void bigramW_free_all(bigramW *R)
// libere recursivement tout le BST de bigramW 

{
    if (!R) return;
    bigramW_free_all(R->lc);
    bigramW_free_all(R->rc);
    free(R);
}

static int cmp_bigramW(bigramW *p, const char *w1, const char *w2)
{
    int c = strcmp(w1, p->word1);
    // on compare  le 1er mot 
    if (c != 0) 
        return c;
    // premier mot egal on departage avec le deuxieme 
    return strcmp(w2, p->word2);
    // si les deux sont egaux strcmp bigramme deja present 
}

bigramW *bigramW_insert(bigramW *R, const char *w1, const char *w2)
{
    if (!R) 
        return allocate_bigramW(w1, w2);
    int cmp = cmp_bigramW(R, w1, w2);
    if      (cmp < 0)
         R->lc = bigramW_insert(R->lc, w1, w2);
    else if (cmp > 0) 
        R->rc = bigramW_insert(R->rc, w1, w2);
    return R;
    // si cmp=0 bigramme deja present
}

int bigramW_search(bigramW *R, const char *w1, const char *w2)
// retourne 1 si la paire (w1,w2) est dans le BST0 sinon 0 */

{
    if (!R) 
        return 0;
    int cmp = cmp_bigramW(R, w1, w2);
    if (cmp == 0) return 1;
    if (cmp < 0)  return bigramW_search(R->lc, w1, w2);
    return bigramW_search(R->rc, w1, w2);
}

int bigramW_count(bigramW *R)
/* retourne le nombre de noeuds dans le BST */

{
    if (!R) 
        return 0;
    return 1 + bigramW_count(R->lc) + bigramW_count(R->rc);
//ce noeud + gauche + droite
}







/* ================================================================
   token
   ================================================================ */

token *allocate_tk(const char *word)
/* alloue un noeud token avec le mot donne et bigramL non encore construit */

{
    token *p = malloc(sizeof(token));
    if (!p)
    {
         fprintf(stderr, "malloc token\n");
          exit(1); }
    strncpy(p->word, word, 49); 
    p->word[49] = '\0';//copie le mot et on force le \0
    p->RbigramL = NULL;
    p->lc = p->rc = NULL;
    return p;
}

void tk_build_bigramL(token *p)
{
    if (!p) 
        return;
    int len = strlen(p->word);
    //on parcourt le mot lettre par lettre et on cree le bigrammeL
    for (int i = 0; i < len - 1; i++)
        p->RbigramL = bigramL_insert(p->RbigramL, p->word[i], p->word[i+1]);
}

void tk_free(token *p)
//libere un noeud token et son BST de bigramL 

{
    if (!p) return;
    bigramL_free_all(p->RbigramL);//liberer les bigraml d abord
    free(p);//liberer le token
}

void tk_free_all(token *R)
{
    if (!R) return;
    tk_free_all(R->lc);
    tk_free_all(R->rc);
    tk_free(R);
}

token *tk_insert(token *R, const char *word)
{//inserer un mot dans le BST de tokens et on construit son BST de bigramL 
    if (!R) {
        token *p = allocate_tk(word);
        tk_build_bigramL(p);
        return p;
    }
    int cmp = strcmp(word, R->word);
    if      (cmp < 0) 
        R->lc = tk_insert(R->lc, word);
    else if (cmp > 0) 
        R->rc = tk_insert(R->rc, word);
    return R;
}

int tk_search(token *R, const char *word)
{
    if (!R)
         return 0;
    int cmp = strcmp(word, R->word);
    if (cmp == 0)
         return 1;
    if (cmp < 0)  
        return tk_search(R->lc, word);
    return tk_search(R->rc, word);
}

int tk_count(token *R)
{
    if (!R)
         return 0;
    return 1 + tk_count(R->lc) + tk_count(R->rc);
    //ce noeud +gauche+droite
}



/* ================================================================
   parag
   ================================================================ */

parag *parag_allocate(const char *texte)
//alloue un paragraphe avec son texte complet

{
    parag *p = malloc(sizeof(parag));
    if (!p)
     { fprintf(stderr, "malloc parag\n"); 
        exit(1); 
    }
    p->texte = my_strdup(texte);//garder une copir du texte original
    if (!p->texte) 
    { 
        fprintf(stderr, "my_strdup parag\n"); 
        exit(1);
     }
    p->Rtoken   = NULL;
    p->RbigramW = NULL;
    return p;
}
/*struct qui sauvegare le contexte partage entre les appels successifs de build_callback.
tokenize appelle build_callback mot par mot,cette strucuture permet de sauvegarder ce qui s est passeau mot precedent*/
typedef struct {
    parag *p;//le paragraphe qu on est entrain de construire
    char   prev[50];//le mot precedent pour formerles bigramW
    int    has_prev;//1 si on a deja un mot precedent sinon 0
    bool   use_stpW;//1 si on ignore les stpw
} BuildCtx;

static void build_callback(const char *tok, void *ctx_raw)
// appel pour chaque mot du texte,ignorer

{
    BuildCtx *ctx = (BuildCtx *)ctx_raw;//recup le type du contexte
    if (ctx->use_stpW && is_stpW(tok)) {
        //on ignore le stpw et on coupe la chaine de bigramW
        ctx->has_prev = 0;
        return;
    }
    //insere le mot dans bst de tokens du parag
    ctx->p->Rtoken = tk_insert(ctx->p->Rtoken, tok);
    
    if (ctx->has_prev) //si on a un mot precedent on forme un bigramW avec lui 
    {    
    ctx->p->RbigramW = bigramW_insert(ctx->p->RbigramW, ctx->prev, tok);
    }
        //memoriser ce mot pour le prochian appel
        strncpy(ctx->prev, tok, 49);
    ctx->prev[49] = '\0';
    ctx->has_prev  = 1;
}

void parag_build(parag *p, bool use_stpW)
//tokenise le texte du paragraphe et construit ses BST de tokens et de bigramW

{
    if (!p || !p->texte) 
     return;

//on prepare le contexte qui sera mis a jour a chaque mot 
    BuildCtx ctx;
    ctx.p        = p;
    ctx.has_prev = 0;//pas encore de mot precedent
    ctx.use_stpW = use_stpW;
    ctx.prev[0]  = '\0';
    tokenize(p->texte, build_callback, &ctx);
}


void parag_free(parag *p)
/* libere un paragraphe et toutes ses structures internes */
{
    if (!p)
      return;
    free(p->texte);
    tk_free_all(p->Rtoken);//liberer tous les tokens et leurs bigramL
    bigramW_free_all(p->RbigramW);
    free(p);
}

/* ================================================================
   file
   ================================================================ */

file *allocate_file(const char *nom, int nbParag)
//allouer une structure file avec son tableau de paragraphes initialise a NULL
{
    file *f = malloc(sizeof(file));
    if (!f) { 
        fprintf(stderr, "malloc file\n"); 
        exit(1);
     }
    strncpy(f->nom, nom, 49); 
    f->nom[49] = '\0';
    f->nbParag    = nbParag;
    //tableau de pointeurs initialise a NULL
    f->paragraphe = calloc(nbParag, sizeof(parag *));
    if (!f->paragraphe) 
    { 
        fprintf(stderr, "calloc paragraphes\n");
         exit(1);
     }
    return f;
}

void file_free(file *f)
//libere un fichier et tous ses paragraphes 

{
    if (!f) return;
    /* liberer chaque paragraphe un par un */
    for (int i = 0; i < f->nbParag; i++)
        parag_free(f->paragraphe[i]);//liberer le tableau de pointeurs
    free(f->paragraphe);//liberer file struct
    free(f);
}

/* ================================================================
   SCORES JACCARD
   ================================================================ */

int tk_inter(token *RA, token *RB)
//retourne le nombre de tokens communs entre les BST RA et RB 
{
    if (!RA || !RB) //un arbre est vide,inter nul
     return 0;
     // pour chaque token de RA on verifie s il existe dans RB
    
    int found = tk_search(RB, RA->word);
    return found + tk_inter(RA->lc, RB) + tk_inter(RA->rc, RB);
}

int tk_union(token *RA, token *RB)
{
    // |A|+|B|-|A inter B|
    return tk_count(RA) + tk_count(RB) - tk_inter(RA, RB);
}

float jaccard_tk(parag *A, parag *B)
/*calcule le score Jaccard entre deux paragraphes sur leurs tokens
   formule = |A inter B| / |A union B|*/
{
    if (!A || !B) 
     return 0.0f;
    int inter = tk_inter(A->Rtoken,B->Rtoken);
    int uni   = tk_union(A->Rtoken,B->Rtoken);
    if (uni == 0) //les 2 parag sont vides
     return 0.0f;
    return (float)inter / (float)uni;// score entre 0.0 et 1.0 
}

int bigramW_inter(bigramW *RA, bigramW *RB)
//retourne le nombre de bigrammes de mots communs entre RA et RB
{
    if (!RA || !RB) 
       return 0;
    // pour chaque bigramW de RA on verifie s il existe dans RB
    int found = bigramW_search(RB, RA->word1, RA->word2);
    return found + bigramW_inter(RA->lc, RB) + bigramW_inter(RA->rc, RB);
}

float jaccard_bigramW(parag *A, parag *B)
/* calcule le score Jaccard entre deux paragraphes sur leurs bigrammes de mots */
{
    if (!A || !B) 
    return 0.0f;
    int inter = bigramW_inter(A->RbigramW, B->RbigramW);
    int uni   = bigramW_count(A->RbigramW) + bigramW_count(B->RbigramW) - inter;
    if (uni == 0) 
        return 0.0f;//aucun bigramme dans les 2 parag
    return (float)inter / (float)uni;
}

float jaccard_bigramL(token *A, token *B)
// calcule le score Jaccard entre deux tokens sur leurs bigrammes de lettres
   
{
    if (!A || !B) 
    return 0.0f;
    bigramL *bA = A->RbigramL;
    bigramL *bB = B->RbigramL;

    int inter = 0;
     //parcours iteratif du BST bA avec une pile explicite

    bigramL *stack[1024];
    int top = 0;
     
    bigramL *cur = bA;
    while (cur || top > 0)
     /* descendre jusqu a la feuille la plus a gauche */

    {
        while (cur) { stack[top++] = cur; cur = cur->lc; }
        cur = stack[--top];
        if (bigramL_search(bB, cur->lettre1, cur->lettre2))
            inter++;
        cur = cur->rc;
        // erifier si ce bigramme est dans bB  
    }

    int uni = bigramL_count(bA) + bigramL_count(bB) - inter;
    if (uni == 0) /* les deux tokens n ont aucun bigramme */
     return 0.0f; 
    return (float)inter / (float)uni;
}