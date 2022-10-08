#include "algorithm.h"

/* deklaracja stosu */
stos Stos;
struct Stos *wsk;
/* ---------------- */

// ustawiona wartos� =/= 0 oznacza �e robot b�dzie mapowa� scianki jak jeszcze nie wjecha�
s32 aencR_old = 0;
s32 aencR_old_debug = 0;

s32 aencR_rotate = 0;

/* flaga ustawiana gdy mazeMapper ma zapisa� kom�rk�, oznacza ona r�wnie� �e z przodu jest scianka, flaga ustawiana jak zosta�y 92 mm do scianki */
u8 map_flag = 0;

u8 checkNextCell = 1;

/* wsp�rz�dne robota */
s8 X = 0;
s8 Y = 0;

/* mapa labiryntu - tablica X na Y */
u8 WALL[MAZE_X][MAZE_Y] = {0};

/* tablica z odleg�osci� od celu */
u8 ROAD[MAZE_X][MAZE_Y] =
				{
						{ 6,5,4,3 },  /* x=0 */
						{ 5,4,3,2 },  /* x=1 */
						{ 4,3,2,1 },  /* x=2 */
						{ 3,2,1,0 }  /* x=4 */

//						{ 1, 2, 3 },  /* x=0 */
//						{ 0, 1, 2 },  /* x=1 */
//						{ 1, 2, 3 },  /* x=2 */
//						{ 2, 3, 4 },  /* x=4 */
//						{ 3, 4, 5 }   /* x=5 */

//						{ 9,8,7,6,5,4 },  /* x=0 */
//						{ 8,7,6,5,4,3 },  /* x=1 */
//						{ 7,6,5,4,3,2 },  /* x=2 */
//						{ 6,5,4,3,2,1 },  /* x=4 */
//						{ 5,4,3,2,1,0 },   /* x=5 */

//						{ 14,13,12,11,10,9,8,7,8 },  /* x=0 */
//						{ 13,12,11,10,9,8,7,6,7 },  /* x=1 */
//						{ 12,11,10,9,8,7,6,5,6 },  /* x=2 */
//						{ 11,10,9,8,7,6,5,4,5 },  /* x=3 */
//						{ 10,9,8,7,6,5,4,3,4 },  /* x=4 */
//						{ 9,8,7,6,5,4,3,2,3 },  /* x=5 */
//						{ 8,7,6,5,4,3,2,1,2 },  /* x=6 */
//						{ 7,6,5,4,3,2,1,0,1 },  /* x=7 */
//						{ 8,7,6,5,4,3,2,1,2 }  /* x=8 */
//
//						{ 16,15,14,13,12,11,10,9,8 },  /* x=0 */
//						{ 15,14,13,12,11,10,9,8,7 },  /* x=1 */
//						{ 14,13,12,11,10,9,8,7,6 },  /* x=2 */
//						{ 13,12,11,10,9,8,7,6,5 },  /* x=3 */
//						{ 12,11,10,9,8,7,6,5,4 },  /* x=4 */
//						{ 11,10,9,8,7,6,5,4,3 },  /* x=5 */
//						{ 10,9,8,7,6,5,4,3,2 },  /* x=6 */
//						{ 9,8,7,6,5,4,3,2,1 },  /* x=7 */
//						{ 8,7,6,5,4,3,2,1,0 }  /* x=8 */
				};

/*
 * 	orientacja robota - pocz�tkowo ustawiona na p�noc
 * 	ustawiaj� j� funkcje turnRight i Left
 */
u8 orientation = NORTH;

u8 dataTemp 	= 0; 	//przechowuje aktualn� orientacje i scianki po przekonwertowaniu na NORTH
s8 nX, nY 		= 0;	//wsp�rz�dne s�siada
s8 aX, aY 		= 0;	//aktualne wsp�rz�dne robota

//zmienne do planowania ruch�w
u8 firstMove, secondMove, thirdMove = 0;

//liczy przejscia p�tli algorytmu
u16 algorithmLoop = 0;

//sygnalizuje �e wybrano dostepnego sasiada, gdzie wartosc komorki w kt�rej
// jestem-1 = wartosc sasiada
u8 neighbourSelected = 0;

//zmienna przyjmuj�ca wartosc 0 albo 1, kt�ra wyra�a zgod� na odpalenie g��wnej p�tli
u8 runMainAlgorithmLoop = 0;

//zmienna przechowuj�ca info �e znaleziono centrum labirtu
u8 searchingFinished = 0;

//flaga przejazdu pomiarowego
u8 speedMode = 0;

//numer przejazdu pomiarowego
u8 numberSpeedMode = 1;

void mazeMapper()
{
	unsigned char txt[14] = {0};

	//dzia�aj tylko gdy robot jedzie prosto (lewe i prawe ko�o ma taki sam nastaw)
	if(pathRDef == pathLDef)
	{
		if(checkNextCell == 1 && getRightEncCount()-aencR_old > 40*IMP_PER_MM) //je�eli przejedzie 60mm od srodka kom�rki, sprawd� nastepna
		{
			#ifdef DEBUG_MODE
			//USART_puts("c7\r\n");
			#endif
			//66mm do srodka kom�rki
			//aencR_old_debug = aencR_old;
			shortBeep(50);

			if(orientation == NORTH)
				Y++;
			else if(orientation == WEST)
				X--;
			else if(orientation == EAST)
				X++;
			else //czyli orient = SOUTH
				Y--;

			if(!speedMode) 	//sprawdzaj scianki je�eli nie jestes na speedmode
			{
				//usuwam smieci jezeli jakies zostaly
				WALL[X][Y] = 0;

				if(R < 120)
				{
					isRightWall = 1;
					LED1_ON;
				}
				else
				{
					isRightWall = 0;
					LED1_OFF;
				}

				if(L < 120)
				{
					isLeftWall = 1;
					LED9_ON;
				}
				else
				{
					isLeftWall = 0;
					LED9_OFF;
				}

				if(RF < 250 && LF < 250) //oznacza to, �e z przodu jest scianka
				{
					WALL[X][Y] |= NORTH_WALL;
					//WALL[X][Y+1] |= NORTH_WALL;
					LED4_ON;
				}
				else
					LED4_OFF;

				if(isLeftWall)
				{
					WALL[X][Y] |= WEST_WALL;
				}

				if(isRightWall)
				{
					WALL[X][Y] |= EAST_WALL;
				}

				//zapis orientacji
				WALL[X][Y] |= orientation;

				//konwersja na NORTH
				WALL[X][Y] = orienationConv(WALL[X][Y]);
			}

			/* obijanie scianek */
			mazeWallUpdate();

			#ifdef DEBUG_MODE
			sprintf((char *)txt, "PRZEDEMNA JEST: %d|%d|%d\r\n", WALL[X][Y], X, Y);
			USART_puts(txt);
			#endif

			/* je�eli dojad� do srodka labiryntu */
			if(ROAD[X][Y] == 0 && (!searchingFinished))
			{
				#ifdef DEBUG_MODE
				printMaze();
				#endif
				i = 1;
				firstMove 	= CENTER;
				secondMove 	= TURN_BACK;
				thirdMove   = SET_CENTER_VARS;
				searchingFinished = 1;
			}

			aX = X;
			aY = Y;

			runMainAlgorithmLoop = 1;

			while(runMainAlgorithmLoop && (!searchingFinished || speedMode))
			{
				/* je�eli to nie pierwsze przejscie p�tli we�
				 * se dziabnij ze stosu
				 */
				if(algorithmLoop > 0)
				{
					//bierz ze stosu
					X = wsk->posX;
					Y = wsk->posY;

					#ifdef DEBUG_MODE
					sprintf((char *)txt, "pull x%d y%d\r\n", X, Y);
					USART_puts(txt);
					#endif
					wsk = remove(wsk);
				}

				dataTemp = orienationConv(WALL[X][Y]);

				//wyznacza now� pozycj�, nowe wsp�rz�dne nX, nY
				nextPosition();

			//while(nX != nX_stos && nY != nY_stos)
				neighbourRunner();

				algorithmLoop++;
			}
			X = aX;
			Y = aY;

			algorithmLoop = 0;

			checkNextCell = 0;

			#ifdef DEBUG_MODE
			sprintf((char *)txt, "%d|%d|%d|%d\r\n", WALL[X][Y], X, Y, orientation); //bylo datatemp
			USART_puts(txt);

			printMaze();
			#endif
		}

		if(getRightEncCount()-aencR_old > 192*IMP_PER_MM || map_flag == 1)	//jezeli przejecha� 192 mm czyli 1 kom�rk�
		{
			//sprintf((char *)txt, "%d %d %d %d\r\n", getRightEncCount(), aencR_old, getRightEncCount()-aencR_old, aencR_old-aencR_old_debug);
			//USART_puts(txt);

			aencR_old = getRightEncCount();
			LED4_OFF;
			map_flag 		= 0; //zeruje flage
			checkNextCell 	= 1;
		}
	}
	else
	{
		//trzeba zapisa� ile przejecha� jak si� obkr�ci�, a potem to odj��
		aencR_old = getRightEncCount();

		//po skr�cie, jak przejedzie 40 mm sprawd� pozycje scianek
		checkNextCell = 1;

		#ifdef DEBUG_MODE
		//USART_puts("c8\r\n");
		#endif
	}
}

void nextPosition()
{
	unsigned char txt[14] = {0};

	//nAvailable = 0;
	neighbourSelected = 0;

	/* sprawd� czy wartosc komorki -1 = wartosc, kt�rego z dostepnych s�sisad�w */
	if((dataTemp & NORTH_WALL) != NORTH_WALL) //na p�nocy nie ma sciany, sprawdz to
	{
	    nX = X;
	    nY = Y+1;

	    //sprawdzanie czy komorka w ktorej jestem -1 = dostepny sasiad
	    if(ROAD[X][Y] - 1 == ROAD[nX][nY])
	    {
	            neighbourSelected = 1;
	    }
	    else
	    {
	            //je�eli nie to nie zmieniaj pozycji
	            nX = X;
	            nY = Y;
	    }

		#ifdef DEBUG_MODE
	    sprintf((char *)txt, "N nx %d ny %d\r\n", nX, nY);
	    USART_puts(txt);
		#endif
	}

	if(!neighbourSelected && (dataTemp & SOUTH_WALL) != SOUTH_WALL)
	{
	    nX = X;
	    nY = Y-1;

	    if(ROAD[X][Y] - 1 == ROAD[nX][nY])
	    {
	            neighbourSelected = 1;
	    }
	    else
	    {
	            nX = X;
	            nY = Y;
	    }
		#ifdef DEBUG_MODE
	    sprintf((char *)txt, "S nx %d ny %d\r\n", nX, nY);
	    USART_puts(txt);
		#endif
	}

	if(!neighbourSelected && (dataTemp & EAST_WALL) != EAST_WALL)
	{
		// TUTAJ MODYFIKOWA�EM JAK BY�EM ZM�CZONY, PATRZ W RAZIE W
	    if(ROAD[X][Y]==ROAD[X+1][Y]+1)
	    {
		    nX = X+1;
		    nY = Y;

	        neighbourSelected = 1;
	    }
	    else
	    {
	            nX = X;
	            nY = Y;
	    }
		#ifdef DEBUG_MODE
	    sprintf((char *)txt, "E nx %d ny %d\r\n", nX, nY);
	    USART_puts(txt);
		#endif
	}

	if(!neighbourSelected && (dataTemp & WEST_WALL) != WEST_WALL)
	{
	    nX = X-1;
	    nY = Y;

	    if(ROAD[X][Y] - 1 == ROAD[nX][nY])
	    {
	            neighbourSelected = 1;
	    }
	    else
	    {
	            nX = X;
	            nY = Y;
	    }
		#ifdef DEBUG_MODE
	    sprintf((char *)txt, "W nx %d ny %d\r\n", nX, nY);
	    USART_puts(txt);
		#endif
	}



	/* w tym momencie mamy albo wsp�rz�dne s�siada do kt�ego mo�emy jecha�: nX, nY
	 * albo mamy aktualne wsp�rz�dne bo nie ma dost�pnego s�siada
	 * do kt�rego mo�emy jecha�, czyli nX = X, nY = Y;
	 */
}

void neighbourRunner(void)
{
	unsigned char txt[14] = {0};
	u8 p = 0;

	/* sprawd� czy wartosc komorki-1 = wartosc KT�REGOKOLWIEK sasiada
	 */

	if(ROAD[X][Y] - 1 == ROAD[nX][nY])
	{
		/* sprawd� czy stos jest pusty */
		if(stackElements == 0)
		{
			/* je�eli to nie pierwsze przejscie to bierz kom�rk�
			 * kt�ra wreszcie si� okaza�a o mniejszym numerze, a nie
			 * jej s�siada
			 * jest to zwi�zane z tym co na karce Konrad podkresli� na zielono
			 */
			if(algorithmLoop > 0)
			{
//				// sprawd� czy badana kom�rka jest w otoczeniu komorki w kt�rej jestem
//				// jesli jest to jedz do niej
//				if((abs(nX-aX) == 1 && abs(nY-aY) == 0) || (abs(nY-aY) == 1 && abs(nX-aX) == 0))
//				{
//					//to nic nie zmieniajd
//				}
//				else
//				{
//					nX = X;
//					nY = Y;
//				}
				//sprawdz czy jakis s�siad ma wartos komorki -1
				if(ROAD[aX+1][aY] == ROAD[aX][aY]-1)
				{
					nX = aX+1;
					nY = aY;
				}
				else if(ROAD[aX-1][aY] == ROAD[aX][aY]-1)
				{
					nX = aX-1;
					nY = aY;
				}
				else if(ROAD[aX][aY+1] == ROAD[aX][aY]-1)
				{
					nX = aX;
					nY = aY+1;
				}
				else if(ROAD[aX][aY-1] == ROAD[aX][aY]-1)
				{
					nX = aX;
					nY = aY-1;
				}
				else
				{
					//niby jest s�siad a nie ma, b�ad krytyczny no bo wtf?
					#ifdef DEBUG_MODE
					sprintf((char *)txt, "nX%d nY%d aX%d aY%d\r\n", nX, nY, aX, aY);
					USART_puts(txt);
					printMaze();
					USART_puts("BK: nie ma sasiada ktory ma o 1 mniejsza wartosc!\r\n");
					DEBUG_STOP
					#endif
				}
			}

			i=1; //reset planera ruch�w

			#ifdef DEBUG_MODE
		    USART_puts("Pusty st!\r\n");
			#endif
		    if(orientation == SOUTH)
		    {
				#ifdef DEBUG_MODE
		    	USART_puts("oriS\r\n");
				#endif
			    if(nX-aX<0)
			    {
					#ifdef DEBUG_MODE
			    	USART_puts("1\r\n");
					#endif
					sTurnRight();
			    }
			    else if(nX-aX>0)
			    {

					#ifdef DEBUG_MODE
			    	USART_puts("2\r\n");
					#endif
					sTurnLeft();
			    }
			    else if(nY-aY<0)
			    {
					#ifdef DEBUG_MODE
			    	USART_puts("3\r\n");
					#endif
			    	sGoStraight();
			    }
			    else //(nY-Y>0
			    {
                    #ifdef DEBUG_MODE
			    	USART_puts("4\r\n");
					#endif
					sTurnBack();
			    }
		    }
		    else if(orientation == NORTH)
			{
				#ifdef DEBUG_MODE
				USART_puts("oriN\r\n");
				//sprintf((char *)txt, "oriN nX%d aX%d\r\n", nX, aX);
				//USART_puts(txt);
				#endif
				if(nX-aX<0)
				{
					#ifdef DEBUG_MODE
					USART_puts("1\r\n");
					#endif
					sTurnLeft();
				}
				else if(nX-aX>0)
				{
					#ifdef DEBUG_MODE
					USART_puts("2\r\n");
					#endif
					sTurnRight();
				}
				else if(nY-aY<0)
				{
					#ifdef DEBUG_MODE
					USART_puts("3\r\n");
					#endif
					sTurnBack();
				}
				else //(nY-aY>0
				{
					#ifdef DEBUG_MODE
					USART_puts("4\r\n");
					#endif
					sGoStraight();
				}
				//DEBUG_STOP
			}
		    else if(orientation == EAST)
			{
				#ifdef DEBUG_MODE
				USART_puts("oriE\r\n");
				#endif
				if(nX-aX<0)
				{
					#ifdef DEBUG_MODE
					USART_puts("1\r\n");
					#endif
					sTurnBack();
				}
				else if(nX-aX>0)
				{
					#ifdef DEBUG_MODE
					USART_puts("2\r\n");
					#endif
					sGoStraight();
				}
				else if(nY-aY<0)
				{
					#ifdef DEBUG_MODE
					USART_puts("3\r\n");
					#endif
					sTurnRight();
				}
				else //(nY-Y>0
				{
					#ifdef DEBUG_MODE
					USART_puts("4\r\n");
					#endif
					sTurnLeft();
				}
				//DEBUG_STOP
			}
		    else if(orientation == WEST)
			{
				#ifdef DEBUG_MODE
		    	USART_puts("oriW\r\n");
				#endif
			    if(nX-aX<0)
			    {
					#ifdef DEBUG_MODE
			    	USART_puts("1\r\n");
					#endif
			    	sGoStraight();
			    }
			    else if(nX-aX>0)
			    {
					#ifdef DEBUG_MODE
			    	USART_puts("2\r\n");
					#endif
			    	sTurnBack();
			    }
			    else if(nY-aY<0)
			    {
					#ifdef DEBUG_MODE
			    	USART_puts("3\r\n");
					#endif
			    	sTurnLeft();
			    }
			    else //(nY-Y>0
			    {
					#ifdef DEBUG_MODE
			    	USART_puts("4\r\n");
					#endif
			    	sTurnRight();
			    }
			    //DEBUG_STOP
			}
			else
			{
				#ifdef DEBUG_MODE
				USART_puts("ori = UNKNOWN \r\n");
				#endif
			}

		    //zezw�l na ruch
		    flaga = 1;
			runMainAlgorithmLoop = 0;
		}
		else
		{
			#ifdef DEBUG_MODE
			USART_puts("St. not empt \r\n");
			#endif
			runMainAlgorithmLoop = 1;
		}
//		//dzi�ki temu zako�cz� p�tle while
//		nX_stos = nX;
//		nY_stos = nY;
	}
	else //�odyn s�siad nie jest = wartosc komorki-1
	{
		#ifdef DEBUG_MODE
		USART_puts("NA STOS!\r\n");
		#endif

		/* zmien wartosc kom�rki w kt�rej jestem na (s�siad + 1)
		 * bierzemy dowolnego DOST�PNEGEGO s�siada
		 */

		/* ten zestaw szuka dost�pnego s�siada */
		if((dataTemp & NORTH_WALL) != NORTH_WALL)
		{
			nX = X;
			nY = Y+1;
		}
		else if((dataTemp & EAST_WALL) != EAST_WALL)
		{
			nX = X+1;
			nY = Y;
		}
		else if((dataTemp & WEST_WALL) != WEST_WALL)
		{
			nX = X-1;
			nY = Y;
		}
		else if((dataTemp & SOUTH_WALL) != SOUTH_WALL)
		{
			nX = X;
			nY = Y-1;
		}
		else
		{
			#ifdef DEBUG_MODE
			USART_puts("BK: nie ma sasiada dostepnego \r\n");
			DEBUG_STOP
			#endif
		}

		ROAD[X][Y] = ROAD[nX][nY]+1;

		/* wsad� wszystkich dost�pnych s�siad�w na stos */
		if((dataTemp & NORTH_WALL) != NORTH_WALL)
		{
		    nX = X;
		    nY = Y+1;

		    wsk = push(wsk, nX, nY);
			#ifdef DEBUG_MODE
		    sprintf((char *)txt, "NS: x%d y%d\r\n", nX, nY);
		    USART_puts(txt);
			#endif
		}
		if((dataTemp & SOUTH_WALL) != SOUTH_WALL)
		{
		    nX = X;
		    nY = Y-1;

		    wsk = push(wsk, nX, nY);
			#ifdef DEBUG_MODE
		    sprintf((char *)txt, "NS: x%d y%d\r\n", nX, nY);
		    USART_puts(txt);
			#endif
		}
		if((dataTemp & EAST_WALL) != EAST_WALL)
		{
		    nX = X+1;
		    nY = Y;

		    wsk = push(wsk, nX, nY);
			#ifdef DEBUG_MODE
		    sprintf((char *)txt, "NS: x%d y%d\r\n", nX, nY);
		    USART_puts(txt);
			#endif
		}
		if((dataTemp & WEST_WALL) != WEST_WALL)
		{
		    nX = X-1;
		    nY = Y;

		    wsk = push(wsk, nX, nY);
			#ifdef DEBUG_MODE
		    sprintf((char *)txt, "NS: x%d y%d\r\n", nX, nY);
		    USART_puts(txt);
			#endif
		}

		/* kontynuuj p�tle, oznacza to w tym miejscu
		 * skok do pocz�tku while(runMainAlgorithmLoop)
		 */
		runMainAlgorithmLoop = 1;

		//wyjmij ze stosu
		//dzi�ki temu kontunuuje p�tle while
//		nX_stos = wsk->posX;
//		nY_stos = wsk->posY;
//
//		wsk = remove(wsk);

//		for(p=0; p<5; p++)
//		{
//			sprintf((char *)txt, "stos: k %d x %d y %d \r\n", p, wsk->posX,  wsk->posY);
//			USART_puts(txt);
//
//			wsk = remove(wsk);
//		}
	}
}

u8 orienationConv(u8 data)
{
	unsigned char txt[50] = {0};
	u8 ori; 		//orientacja danych
	u8 bitMemory;	// do zapamietywania bit�w kt�re tracimy, spieszmy si� kocha� bity, tak szybko odchodz�

	ori = (data & 0b01010101);

	if(ori == EAST)
	{
		USART_puts(txt);

		bitMemory = (data & 0b00000011);
		bitMemory <<= 6;

		data = ((data >> 2) | bitMemory);
	}
	else if(ori == WEST)
	{
		bitMemory = (data & 0b11000000);
		bitMemory >>= 6;

		data = ((data << 2) | bitMemory);
	}
	else if(ori == SOUTH)
	{
		bitMemory = (data & 0b00001111);
		bitMemory <<= 4;

		data = ((data >> 4) | bitMemory);
	}
	else
	{
		//ori to NORTH albo jaki error - nieznana ori
	}

	//korekcja orientacji
	data &= 0b10111010;

	return data;
}

void printMaze()
{
	unsigned char txt[20] = {0};
	//zmienne licznikowe
	u8 r, c = 0;

	USART_puts("Maze:\r\n");
	for(c = 0; c<MAZE_Y; c++)
	{
		for(r=0; r<MAZE_X; r++)
		{
			sprintf((char *)txt, "%d|%d|%d|%d\r\n", WALL[r][c], r, c, ROAD[r][c]);
			USART_puts(txt);
		}
	}
}

void sTurnLeft()
{
	firstMove 	= CENTER;
	secondMove 	= TURN_LEFT;
	thirdMove 	= GO_STRAIGHT;
}

void sTurnRight()
{
	firstMove 	= CENTER;
	secondMove 	= TURN_RIGHT;
	thirdMove 	= GO_STRAIGHT;
}

void sTurnBack()
{
	firstMove 	= CENTER;
	secondMove 	= TURN_BACK;
	thirdMove 	= GO_STRAIGHT;
}

void sGoStraight()
{
	i = 3;
	thirdMove 	= GO_STRAIGHT;
}

/* odbija scianki pomiedzy komorkami, labirynt powinien byc zapisany w WALL w orientacji NORTH */
void mazeWallUpdate()
{
	u8 x, y = 0;

	//interacja po wszystkich kom�rkach
	for(y=0; y<MAZE_Y; y++)
	{
		for(x=0; x<MAZE_X; x++)
		{
			//sprintf((char *)txt, "%d|%d|%d|%d\r\n", WALL[r][c], r, c, ROAD[r][c]);
			//USART_puts(txt);

			if((WALL[x][y] & NORTH_WALL) == NORTH_WALL) //je�eli jest na p�nocy sciana
			{
				//sprawdzenie czy nie wykraczam poza labirynt
				if(y+1<MAZE_Y)
				//to kom�rka wy�ej ma j� na po�udniu
					WALL[x][y+1] |= SOUTH_WALL;
			}
			if((WALL[x][y] & EAST_WALL) == EAST_WALL)
			{
				if(x+1<MAZE_X)
					WALL[x+1][y] |= WEST_WALL;
			}
			if((WALL[x][y] & WEST_WALL) == WEST_WALL)
			{
				if(x-1>=0)
					WALL[x-1][y] |= EAST_WALL;
			}
			if((WALL[x][y] & SOUTH_WALL) == SOUTH_WALL)
			{
				if(y-1>=0)
					WALL[x][y-1] |= NORTH_WALL;
			}
		}
	}
}
