#ifndef EGYEB_H
#define EGYEB_H

#include "szintek.h"
#include "utvonalkereso.h"

int szam_beolvas(int min, int max);
void jatek_ment(int pont, Szintek meretek);
int jatek_betolt(int *szint, int *pont);
void segitseg(void);
Cella*** cella_tomb_foglal(Szintek meretek);
Csucs ***csucs_tomb_foglal(Szintek meretek);
bool **bool_tomb_foglal(Szintek meretek);
void cella_tomb_szabadit(Szintek meretek);
void kilep(int code, char s[100], Szintek meretek);
int menu(void);

#endif