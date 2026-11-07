#ifndef LECTURE_H
#define LECTURE_H

#include <stdbool.h>
#include "tree.h"

int   compter_paragraphes(const char *chemin); //fait une première passe pour compter
file *lire_fichier(const char *chemin, bool use_stpW);//fait une deuxième passe pour construire

#endif /* LECTURE_H */