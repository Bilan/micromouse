#include "stack.h"

//liczba element�w na stosie
s16 stackElements = 0;

u8* pull(stos* a)
{
	u8 pos[2]; // pos[0] to X, a pos[1] to Y

	pos[0] = a->posX;
	pos[1] = a->posY;

    return pos;
}

stos*  remove(stos *a)
{
    stos *w = a;	// zapamietujemy to co usuwamy
    a = a->next;	// przeskakujemy usuwany element
    free(w); 		// zwalniamy pamiec przeskoczonego elementu
    stackElements--; //zmniejszamy liczb� element�w na stosie
    return a; 		// tak spreparowana liste zwracamy
}

stos* push(stos *a, u8 nposX, u8 nposY)
{
    stos *newElement = (stos*) malloc(sizeof(stos));//nowy element w pamiec

    newElement->posX 	= nposX;
    newElement->posY 	= nposY;
    newElement->next 	= a;		// wskaznik przypisany do reszty listy

    stackElements++; //swi�kszamy liczb� element�w na stosie

    a = newElement;// podmiana
    return a;// zwracamy nowa liste
}

/*---------------------------------*/
/* obs�uga kolejki ruch�w */
//
//void movePush(int move)
//{
//	struct moveQueue *tmp = NULL;
//
//    tmp = (struct moveQueue*)malloc(sizeof(struct moveQueue));
//    tmp->move = move;
//
//    if(pointer == NULL) first = tmp;
//    else (*pointer).ref = tmp; //b�d� pointer->ref
//    tmp -> ref = NULL; //przyk�adowy
//    pointer = tmp;
//}
