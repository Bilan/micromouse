#include "regulator.h"

float acc = ACC;

/* przyszpieszenia dla szybkiego przejazdu */
float accSpeedRun = 0;

u8 		VMax = V_MAX;

int32_t  targetV_Right = 0;		//pr�dkosc do ktorej dazymy - V max -> wyrazi�em j� w impulsach - tyle ma by� impuls�w w 1 milisekundzie
int32_t  targetV_Left  = 0;

int32_t  path = 0;				//droga do przejechania

int32_t  pathRDef = 0;
int32_t  pathLDef = 0;

int32_t  pathRDefOld = 0;
int32_t  pathLDefOld = 0;

int32_t  pathLeftRight = 0;
int32_t  pathLeftLeft = 0;

int32_t actualV_Left_old  = 0;
int32_t actualV_Right_old = 0;

float  demandV_Left = 0;			//zadana pr�dkos�
float  demandV_Right = 0;

int32_t  actualV_Right = 0;			//aktualna pr�dkosc -> liczona w main() co 1 ms wyrazona w impulsach enkodera
int32_t  actualV_Left = 0;

int32_t errorRightOld = 0;
int32_t errorLeftOld = 0;

int32_t regRight = 0;
int32_t regLeft = 0;

int32_t errorRight = 0;
int32_t errorLeft = 0;

int32_t errorRightSensor = 0;
int32_t errorLeftSensor = 0;

int32_t gyroError		= 0;

int32_t rotationTime = 0;

u8 isLeftWall = 1;
u8 isRightWall = 1;
u8 isFrontWall = 1;

s32 pathAfterTurn = 0; //droga przejechana po wykonaniu skr�tu
s32 pathAfterTurn_old = 0; //droga przejechana po wykonaniu skr�tu

/* nastawy */
s32 K1 = 25;
s32 K2 = 15;
float K3 = 0.01;
u8 K4 = 4; //wzmocnienie dla czunik�w odleg�osci

int32_t i = 0;
u8 flaga = 0;

u8 frontCheck = 0;
u8 frontCheck2 = 0;

/* flaga wykonania skr�tu */
u8 turnFlag = 0;

unsigned char txt[50] = {0};

/* uruchamia� co 1ms */
void profiler()
{
	/* RIGHT */

	acc += accSpeedRun;

	//definiowanie innych parametr�w dla hamowania i przyspieszania
	if(pathRDef == pathLDef && targetV_Right == 0)
		acc = 0.2   + accSpeedRun;
	else
		acc = ACC + accSpeedRun;

	if(pathRDef < 0)
	{
		if(pathLeftRight >= actualV_Right*actualV_Right/(-2*acc))
			targetV_Right = 0; //trzeba hamowa�
		else
			targetV_Right = -VMax; //czyli v-max
	}
	else
	{
		if(pathLeftRight <= actualV_Right*actualV_Right/(2*acc))
			targetV_Right = 0; //trzeba hamowa�
		else
			targetV_Right = VMax; //czyli v-max
	}

	if(demandV_Right < targetV_Right)
	{
		demandV_Right +=  acc;

		if(demandV_Right > targetV_Right)
			demandV_Right = targetV_Right;
	}

	if(demandV_Right > targetV_Right)
	{
		demandV_Right -= acc;

		if(demandV_Right < targetV_Right)
			demandV_Right = targetV_Right;
	}

	/* LEFT */

	if(pathLDef < 0)
	{
		if(pathLeftLeft >= actualV_Left*actualV_Left/(-2*acc))
		{
			targetV_Left = 0; //trzeba hamowa�
		}
		else
			targetV_Left = -VMax; //czyli v-max
	}
	else
	{
		if(pathLeftLeft <= actualV_Left*actualV_Left/(2*acc))
			targetV_Left = 0; //trzeba hamowa�
		else
			targetV_Left = VMax; //czyli v-max
	}

	if(demandV_Left < targetV_Left)
	{
		demandV_Left +=  acc;

		if(demandV_Left > targetV_Left)
			demandV_Left = targetV_Left;
	}

	if(demandV_Left > targetV_Left)
	{
		demandV_Left -= acc;

		if(demandV_Left < targetV_Left)
			demandV_Left = targetV_Left;
	}


	pathLeftRight -= actualV_Right;
	pathLeftLeft  -= actualV_Left;

	//if(pathLeftRight == 0 || pathLeftLeft == 0)
		//pathLeftRight = pathLeftLeft = 0;

	pathAfterTurn = getRightEncCount()-pathAfterTurn_old;
}

void regulator()
{
	s32 impRemain = 0; //pozosta�e impulsy do obrotu
	s32 angleRemain = 0; //pozosta�y k�t

	if(demandV_Right != 0 && demandV_Left == demandV_Right && pathAfterTurn > 40*IMP_PER_MM) //robot chce jecha� prosto, poprawiaj na podstawie odczytu ze scianek
	{
		if(L < hasLeftWall && R < hasRightWall)
		{
			//opieraj si� na dw�ch scianach po lewej i prawej
			errorRightSensor = (L - R);
			errorLeftSensor  = (R - L);

			LED2_OFF; LED4_OFF; LED5_ON; LED6_ON;
		}
		else if(hasLeftWall > L)
		{
			//tylko lewa strona, prawej nie wida�
			errorRightSensor = 0;
			errorLeftSensor  = (leftMiddleValue - L)/3;

			LED5_ON; LED6_OFF; //LED4_OFF;
		}
		else if(hasRightWall > R)
		{
			//tylko prawa strona, lwewj nie wida�
			errorLeftSensor  = 0;
			errorRightSensor  = (rightMiddleValue - R)/3;

			LED2_OFF; LED4_OFF; LED5_OFF; LED6_ON;
		}
	}
	else if(gyroError != 0 && demandV_Left == 0 && demandV_Right == 0) //robot chce si� obkr�ci�, korekcja �yroskopem, aktywna gdy wg enkoder�w obr�t zosta� zako�czony, a �yroskop wskazuje �e obr�t nie zosta� do ko�ca wykonany
	{
		angleRemain = gyroError - angle; //k�t jaki jeszcze trzeba przejecha�

		impRemain = abs(angleRemain)/10000*IMP_PER_DEGREE; //pozosta�e impulsy do przejechania

		if (angleRemain > GYRO_ERROR_MAX) //chce sie w prawo obrocic, tolerancja p� stopnia (5000)
		{
			pathRDef = pathLeftRight = -impRemain;
			pathLDef = pathLeftLeft  =  impRemain;
			//turnRight(abs(angleRemain)/10000);
			//gyroError = 0;
		}
		else if(angleRemain < -GYRO_ERROR_MAX) //chce sie w lewo obrocic
		{
			pathRDef = pathLeftRight =  impRemain;
			pathLDef = pathLeftLeft  = -impRemain;
			//turnLeft(abs(angleRemain)/10000);
			//gyroError = 0;
		}
		else //jest w zakresie -5000<x<5000 -> zako�czono obr�t
		{
			pathAfterTurn = 0; //zerowanie drogi przejechanej po wykonaniu skr�tu
			pathAfterTurn_old = getRightEncCount();
			gyroError = 0;
			//flaga = 1; //zezw�l na ruch

			LED2_OFF;
		}

		//je�eli zatnie si�, to po 2 sekundach odpusc
		if(millis() - rotationTime > 2000)
		{
			pathAfterTurn = 0; //zerowanie drogi przejechanej po wykonaniu skr�tu
			pathAfterTurn_old = getRightEncCount();
			gyroError = 0;
			//flaga = 1; //zezw�l na ruch
			//i++;
			LED2_OFF;
		}
		//flaga = 1;

		LED2_ON; LED4_OFF; LED5_OFF; LED6_OFF;
	}
	else
	{
		errorLeftSensor  = 0;
		errorRightSensor = 0;
		//gyroError = 0;

		//flaga = 1; //zezw�l na ruch

		LED2_OFF; LED4_OFF; LED5_OFF; LED6_OFF;
	}

	/* obliczenie b��d�w z enkoder�w */
	errorRight = demandV_Right - actualV_Right;
	errorLeft  = demandV_Left  - actualV_Left;

	regRight = K1 * errorRight - K2 * (errorRight - errorRightOld)  + K4*errorRightSensor;// + gyroRightError;
	regLeft  = K1 * errorLeft  - K2 * (errorLeft - errorLeftOld)    + K4*errorLeftSensor;// + gyroLeftError;

	/* zapis poprzednich b��d�w */
	errorRightOld = errorRight;
	errorLeftOld  = errorLeft;

	/* ustawienie PWM */
	setRightPwm(regRight);
	setLeftPwm(regLeft);

	/* ustawianie flagi */
	if(flaga == 0 && demandV_Right == 0 && demandV_Left == 0 && gyroError == 0)
	{
		flaga = 1; //mozna jecha� dalej
		i++;
	}
}

void wallDetector()
{
//	if(frontCheck2 == 1 && RF < 100 && LF < 100) // | 32 mm od przodu robota
//	{
//		//ustawia flage dla mazeMapper()
//		//map_flag = 1;
//		frontCheck2 = 0;
//	}

	//jezeli pozosta�o 12 cm do przedniej scianki aktualizuj nastawy
	if(frontCheck == 1 && (RF < 82 || LF < 82)) // | 32 mm od przodu robota
	{
		//shortBeep(50);
		//flaga = 0; //zabro� zmian� ruchu

		pathRDef = pathLDef = pathLeftRight = pathLeftLeft = 50*IMP_PER_MM;
		//goProsto(140);
		frontCheck = 0;
		//frontCheck2 = 1;
	}
	else
	{
		//LED4_OFF;
	}
}

void turnLeft(int32_t degree)
{
	flaga = 0;

	//lewo
	u32 imp;
	imp = degree*IMP_PER_DEGREE;

	angle = 0; //zerowanie �yro
	gyroError = -degree*10000;	//razy 10 bo chce mie� po przecinku 1 liczbe, np 360,1 stopnia

	//pathRDef = pathLeftRight =  imp;
	//pathLDef = pathLeftLeft  = -imp;

	if(degree >= 89)
	{
		if(orientation == WEST)
			orientation = SOUTH;
		else
			orientation = orientation << 2;
	}

	//turnFlag = 1; // flaga informuj�ca �e robot skr�ci�

	rotationTime = millis();
}

void turnRight(int32_t degree)
{
	flaga = 0;
	//prawo
	u32 imp;
	u8 rotations = degree/90; //ile obrot�w o 90 st wykona robot
	u8 k = 0; //zmienna licznikowa;

	imp = degree*IMP_PER_DEGREE;

	angle = 0; //zerowanie �yro
	gyroError = degree*10000; //razy 10 bo chce mie� po przecinku 1 liczbe, np 360,1 stopnia

	//pathRDef = pathLeftRight = -imp;
	//pathLDef = pathLeftLeft  =  imp;

	if(degree >= 89)
	{
		for(k=0; k<rotations; k++)
		{
			//shortBeep(30);
			if(orientation == SOUTH)
				orientation = WEST;
			else
				orientation = orientation >> 2;
		}
	}
	//turnFlag = 1; // flaga informuj�ca �e robot skr�ci�

	rotationTime = millis();
}

void goProsto(int32_t milimiters)
{
	//przelicznik milimetry na impulsy
	pathRDef = pathLeftRight = pathLDef = pathLeftLeft =  milimiters*IMP_PER_MM;

	gyroError = 0;

	frontCheck = 1; //zastopuj mysz jak bedzie sciana
	//turnFlag = 0; //wyzerowanie flagi informujacej o skr�cie

	flaga = 0;
}
