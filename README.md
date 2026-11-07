# Analyse textuelle par similarité de Jaccard

Outil console en **langage C** qui compare des textes à l'aide de la **similarité de Jaccard** et d'opérations ensemblistes (intersection, union, différence, inclusion), au niveau des **paragraphes** ou du **vocabulaire**.

Projet individuel réalisé en 2026 dans le cadre du cours d'algorithmique et structures de données.

## Fonctionnalités

- Chargement de plusieurs fichiers texte, découpés automatiquement en paragraphes (séparés par une ligne vide).
- Prétraitement du texte : passage en minuscules, suppression des accents (UTF-8), de la ponctuation et des chiffres, apostrophe traitée comme séparateur, filtrage optionnel des **mots vides** (stopwords) en français ou en anglais.
- Trois mesures de similarité (Jaccard) :
  - sur les **mots** (tokens) d'un paragraphe ;
  - sur les **bigrammes de mots** (paires de mots consécutifs) ;
  - sur les **bigrammes de lettres**, pour rapprocher des mots proches (par exemple « algorithme » et « algorithmes »).
- Opérations sur les **paragraphes** (ensembles comparés deux à deux) : intersection, union, différence, inclusion, avec un **seuil de similarité** réglable.
- Opérations sur le **vocabulaire** (N ensembles de mots) : intersection, union, différence, inclusion.
- Menu interactif en console, libération propre de la mémoire.

## Structures de données

Tout est implémenté à la main, sans bibliothèque externe :

| Structure | Rôle |
|---|---|
| BST de `token` | vocabulaire d'un paragraphe ou d'un ensemble, mots uniques triés |
| BST de `bigramL` | bigrammes de lettres de chaque mot |
| BST de `bigramW` | bigrammes de mots d'un paragraphe |
| `parag`, `file`, `Ensemble` | paragraphe, fichier (tableau dynamique de paragraphes), ensemble de mots ou de paragraphes |

Un score de Jaccard entre deux ensembles A et B se calcule par `|A ∩ B| / |A ∪ B|`. Pour éviter des calculs inutiles entre mots, une **borne supérieure** (rapport des tailles des bigrammes) écarte les candidats qui ne peuvent pas atteindre le seuil.

## Organisation du code

```
├── main.c         boucles principales (paragraphes / mots)
├── menu.c/.h      interface console
├── lecture.c/.h   lecture des fichiers et découpage en paragraphes
├── tree.c/.h      structures BST, normalisation, tokenisation, scores de Jaccard
├── ensemble.c/.h  ensembles de mots et leurs sources
└── operations.c/.h opérations ensemblistes
```

## Compilation et exécution

Prérequis :
gcc -o analyse main.c menu.c lecture.c tree.c operations.c ensemble.c
```bash
make
./analyse
```

Le menu permet ensuite de charger des fichiers (par exemple ceux du dossier `textes/`), puis de choisir une opération.

## Limites connues et pistes d'amélioration

- Les BST ne sont **pas équilibrés** : sur des mots insérés dans l'ordre alphabétique, l'arbre dégénère en liste. Un AVL améliorerait les performances.
- Les parcours itératifs utilisent des piles de taille fixe (4096), ce qui suppose un vocabulaire raisonnable.
## Auteure

Barkache Amani, étudiante en informatique, ESI, Algérie, 2026.
