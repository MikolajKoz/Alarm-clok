#include "MKL05Z4.h"
#include "frdm_bsp.h"
#include "lcd1602.h"
#include "leds.h"
#include "klaw.h"
#include "tsi.h"
#include "i2c.h"
#include "TPM.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MOD_500Hz 1310
#define MOD_600Hz 935		// MOD dla czestotliwosci ok. 1002Hz
#define ZEGAR 655360

volatile uint8_t S2_press = 0;
volatile uint8_t S3_press = 0;
uint8_t licznik=0;			// Licznik przerwan ( do 10)
uint8_t sekunda_OK=0;		// "1"oznacza, ze handler od SysTick zliczyl 10 przerwan, kazde po 0.1s, czyli jedna sekunde
volatile uint8_t godzinaZegar=0;
volatile uint8_t minutaZegar=0;
volatile uint8_t sekundaZegar=0;
volatile uint8_t godzinaBudzik=0;
volatile uint8_t minutaBudzik=0;
volatile uint8_t sekundaBudzik=0;
volatile uint8_t alarm=0;
uint16_t currentMOD=MOD_600Hz;
volatile uint8_t drzemka=0;
volatile uint8_t czasDrzemka=15;
volatile uint8_t kolizjaCzasu=0;

void PORTA_IRQHandler(void)	// Podprogram obslugi przerwania od klawiszy S2, S3 i S4
{
	uint32_t buf;
	buf=PORTA->ISFR & (S2_MASK | S3_MASK);

	switch(buf)
	{
		case S2_MASK:	DELAY(10)
									if(!(PTA->PDIR&S2_MASK))		// Minimalizacja drgan zestyków
									{
										if(!(PTA->PDIR&S2_MASK))	// Minimalizacja drgan zestyków (c.d.)
										{
											if(!S2_press)
											{
												S2_press=1;
											}
										}
									}
									break;
		case S3_MASK:	DELAY(10)
									if(!(PTA->PDIR&S3_MASK))		// Minimalizacja drgan zestyków
									{
										if(!(PTA->PDIR&S3_MASK))	// Minimalizacja drgan zestyków (c.d.)
										{
											if(!S3_press)
											{
												S3_press=1;
											}
										}
									}
									break;
		default:			break;
	}
	PORTA->ISFR |=  (S2_MASK | S3_MASK);	// Kasowanie wszystkich bitów ISF
	NVIC_ClearPendingIRQ(PORTA_IRQn);
}

void PIT_IRQHandler(void)
{
	licznik++;
	if(licznik == 10)
	{
		licznik=0;
		sekunda_OK=1;
	}
	PIT->CHANNEL[0].TFLG &= PIT_TFLG_TIF_MASK;
	NVIC_ClearPendingIRQ(PIT_IRQn);
}

int main (void)
{
	uint8_t w=0;
	char display[]={0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20};
	
	// Inicjalizacja timera PIT
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;									//wlacz PIT
	PIT->MCR &= ~PIT_MCR_MDIS_MASK;										//wlacz clock
	PIT->MCR |= PIT_MCR_FRZ_MASK;											//zatrzymaj podczas debugowania
	PIT->CHANNEL[0].LDVAL = PIT_LDVAL_TSV(2399999);		//100ms
	PIT->CHANNEL[0].TCTRL &= PIT_TCTRL_CHN_MASK;			//nie lacz zegarow
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;			//wlacz prosbe o przerwanie
	NVIC_SetPriority(PIT_IRQn,0);											//ustaw priorytet
	NVIC_ClearPendingIRQ(PIT_IRQn);										//wyczczysc poprzednie przerwania
	NVIC_EnableIRQ(PIT_IRQn);													//wlacza przerwania
	// Koniec inicjalizacji
	
	Klaw_Init();								// Inicjalizacja klawiatury
	Klaw_S2_4_Int();						// Klawisze S2, S3 i S4 zglaszaja przerwanie
	LED_Init();									// Inicjalizacja diod LED
	LCD1602_Init();		 					// Inicjalizacja LCD
	LCD1602_Backlight(TRUE);  	// Wlaczenie podswietlenia
	TSI_Init();									// Inicjalizacja panelu dotykowego
	PWM_Init();
	
	LCD1602_ClearAll();					// Wyczysc ekran
	LCD1602_SetCursor(0,0);			// Ustaw kursor na poczatku drugiej linii
	LCD1602_Print("00");
	
	//////////////////////////////////// USTAWIANIE CZASU /////////////////////////////////////////

	while(!S2_press)					// jezeli wcisne S2 wyjde z petli
	{
		w=TSI_ReadSlider();
		if(w!=0)
		{
			if(w>23)
			{
				godzinaZegar=23;	
			}
			else
			{
				godzinaZegar=w;
			}
		}
				
		if(godzinaZegar>0 && godzinaZegar<24)
		{
			LCD1602_SetCursor(0,0);
			sprintf(display,"%02d",godzinaZegar);
			LCD1602_Print(display);
		}
	}
	S2_press=0;
	LCD1602_SetCursor(2,0);
	sprintf(display,":00");
	LCD1602_Print(display);
			
	while(!S2_press)					// jezeli wcisne S2 wyjde z petli
	{
		w=TSI_ReadSlider();
		if(w!=0)
		{
			if(w>59)
			{
				minutaZegar=59;	
			}
			else
			{
				minutaZegar=w;
			}
		}
				
		if(minutaZegar>0 && minutaZegar<60)
		{
			LCD1602_SetCursor(2,0);
			sprintf(display,":%02d",minutaZegar);
			LCD1602_Print(display);
		}
	}
	S2_press=0;
	LCD1602_SetCursor(5,0);
	sprintf(display,":00");
	LCD1602_Print(display);
	while(!S2_press)					// jezeli wcisne S2 wyjde z petli
	{
		w=TSI_ReadSlider();
		if(w!=0)
		{
			if(w>59)
			{
				sekundaZegar=59;	
			}
			else
			{
				sekundaZegar=w;
			}
		}
				
		if(sekundaZegar>0 && sekundaZegar<60)
		{
			LCD1602_SetCursor(5,0);
			sprintf(display,":%02d",sekundaZegar);
			LCD1602_Print(display);
		}
	}
	S2_press=0;
	LCD1602_SetCursor(0,1);
	sprintf(display,"00");
	LCD1602_Print(display);
	
	///////////////////////////////////// KONIEC USTAWIANIA CZASU	///////////////////////////////////////////
	
	///////////////////////////////////// USTAWIANIE BUDZIKA ///////////////////////////////////////////
	
	while(!S2_press)					// jezeli wcisne S2 wyjde z petli
	{
		w=TSI_ReadSlider();
		if(w!=0)
		{
			if(w>23)
			{
				godzinaBudzik=23;	
			}
			else
			{
				godzinaBudzik=w;
			}
		}
				
		if(godzinaBudzik>0 && godzinaBudzik<24)
		{
			LCD1602_SetCursor(0,1);
			sprintf(display,"%02d",godzinaBudzik);
			LCD1602_Print(display);
		}
	}
	S2_press=0;
	LCD1602_SetCursor(2,1);
	sprintf(display,":00");
	LCD1602_Print(display);
			
	while(!S2_press)					// jezeli wcisne S2 wyjde z petli
	{
		w=TSI_ReadSlider();
		if(w!=0)
		{
			if(w>59)
			{
				minutaBudzik=59;	
			}
			else
			{
				minutaBudzik=w;
			}
		}
				
		if(minutaBudzik>0 && minutaBudzik<60)
		{
			LCD1602_SetCursor(2,1);
			sprintf(display,":%02d",minutaBudzik);
			LCD1602_Print(display);
		}
	}
	S2_press=0;
	LCD1602_SetCursor(5,1);
	sprintf(display,":00");
	LCD1602_Print(display);
	while(!S2_press)					// jezeli wcisne S2 wyjde z petli
	{
		w=TSI_ReadSlider();
		if(w!=0)
		{
			if(w>59)
			{
				sekundaBudzik=59;	
			}
			else
			{
				sekundaBudzik=w;
			}
		}
				
		if(sekundaBudzik>0 && sekundaBudzik<60)
		{
			LCD1602_SetCursor(5,1);
			sprintf(display,":%02d",sekundaBudzik);
			LCD1602_Print(display);
		}
	}
	S2_press=0;
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; 		// START TIMERA
	
	///////////////////////////////////// KONIEC USTAWIANIA BUDZIKA ///////////////////////////////////////////

	while(1)		// Poczatek petli glównej
	{
		w=TSI_ReadSlider(); 	//wylaczanie budzika
		if(w!=0 && alarm==1)
		{
			LCD1602_ClearAll();
			TPM0->MOD = 0x0;
			TPM0->SC |= TPM_SC_CMOD(0);
			PTB->PSOR|= LED_R_MASK;
			PTB->PSOR|= LED_G_MASK;
			PTB->PSOR|= LED_B_MASK;
			alarm=0;
		}
		
		if(S3_press && alarm==1) //ustawianie drzemki
		{
			drzemka=1;
			LCD1602_ClearAll();
			TPM0->MOD = 0x0;
			TPM0->SC |= TPM_SC_CMOD(0);
			PTB->PSOR|= LED_R_MASK;
			PTB->PSOR|= LED_G_MASK;
			PTB->PSOR|= LED_B_MASK;
			alarm=0;
			S3_press=0;
		}
		
		if(drzemka==1)
		{
			if(sekundaZegar+czasDrzemka>59)
			{
				minutaBudzik=minutaZegar + 1;
				if(minutaBudzik>59)
				{
					godzinaBudzik=godzinaZegar + 1;
					minutaBudzik = 0;
					if(godzinaBudzik==24)
					{
						godzinaBudzik=0;
					}
				}
				kolizjaCzasu=60-sekundaZegar;
				sekundaBudzik=czasDrzemka-kolizjaCzasu;
			}
			else
			{
				sekundaBudzik=sekundaZegar+czasDrzemka;
				minutaBudzik=minutaZegar;
				godzinaBudzik=godzinaZegar;
			}
			LCD1602_SetCursor(0,0);
			sprintf(display,"%02d:%02d:%02d",godzinaZegar,minutaZegar,sekundaZegar);
			LCD1602_Print(display);
			LCD1602_SetCursor(0,1);
			sprintf(display,"%02d:%02d:%02d",godzinaBudzik,minutaBudzik,sekundaBudzik);
			LCD1602_Print(display);
			drzemka=0;
		}
			
		if(sekundaZegar==sekundaBudzik)	
			if(minutaZegar==minutaBudzik)		
				if(godzinaZegar==godzinaBudzik)
				{
					LCD1602_SetCursor(9,0);
					sprintf(display,"Drzemka");
					LCD1602_Print(display);
					LCD1602_SetCursor(11,1);
					sprintf(display,"%02d s",czasDrzemka);
					LCD1602_Print(display);
					alarm=1;
				}
					
		if(alarm==1)
		{			
			currentMOD++;
			TPM0->MOD = currentMOD;
			TPM0->CONTROLS[0].CnV = ((int)currentMOD*0.5);
			if(currentMOD==MOD_500Hz)
			{
				currentMOD=MOD_600Hz;
				PTB->PTOR|= LED_R_MASK;
				DELAY(5)
				PTB->PTOR|= LED_G_MASK;
				DELAY(5)
				PTB->PTOR|= LED_B_MASK;
				DELAY(5)
			}				
		}

		if(sekunda_OK) //wyswietlanie aktualnego czasu
		{
			sekundaZegar++;
			if(sekundaZegar>59)
			{
				sekundaZegar=0;
				minutaZegar++;
				if(minutaZegar>59)
				{
					minutaZegar=0;
					godzinaZegar++;
					if(godzinaZegar>=24)
					{
						godzinaZegar=0;
					}
				}
			}
			LCD1602_SetCursor(0,0);
			sprintf(display,"%02d:%02d:%02d",godzinaZegar,minutaZegar,sekundaZegar);
			LCD1602_Print(display);
			sekunda_OK=0;
		}
	}
}


