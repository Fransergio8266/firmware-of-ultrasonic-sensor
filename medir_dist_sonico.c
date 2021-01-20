//bibliotecas
#include<p30f4011.h>
#include<uart.h>

//Fuses
_FOSC(XT);//ESCOLHA DO CLOCK 
_FWDT(WDT_OFF);//WATCH DOG DESLIGADO
_FGS(CODE_PROT_OFF); // PROTE��O DE C�DIGO DESLIGADO

///MCLR_DIS: DESABILITA RESET POR TENS�O DE 0 V NO PINO MCLR
//PBOR_ON (POWER-ON RESET): HABILITA O RESET DURANTE O TEMPO EM QUE A TENS�O E OSCILADORES N�O ESTABILIZARAM
//PWRT_64 (): HABILITA O RESET EM 64 ms AP�S O IN�CIO DO FUNCIONAMENTO 
//BORV42 (BROWNOUT):HABILITA O RESET QUANDO A TENS�O VDD FICA MENOR DO QUE 4,2 V
_FBORPOR(MCLR_DIS);

//CICLO DE TRABALHO DO PWM PARA RECIPIENTE N�O PRESSIONADO
int lim=24;

//DIST�NCIA DO SENSOR DE ULTRASOM EM CM
unsigned int dist=0;

//MENSAGEM DE VALIDA��O DA DIST�NCIA DO SENSOR
char p[6];

//VARI�VEL NECESS�RIA PARA CONTAR DURA��O DO PULSO
unsigned int cont_tempo_pulso=0;

//CONFIGURANDO
void main(void)
{

U2MODE=0b1000000000000000;
//HABILITANDO TRANSMISS�O DE DADOS: UTXEN=1. HABILITANDO INTERRUP��O APENAS QUANDO O BUFFER DE TRANSMISS�O EST� VAZIO:UTXISEL=1
U2STA=0b1000010000000001;
//TAXA DE BAUD=Fcy/(16*(U2BRG+1))=9600
U2BRG=12;


ADPCFG=0b111111110;

T1CON=0b1000000000000000;
_T1IE=1;
PR1=199;

T2CON=0b1000000000000000;
_T2IE=1;
PR2=19;


TRISB=0b1111111111111001;
PORTB=0b000000000;
//EXECUTANDO
while(1){
	//CASO O PULSO DO SENSOR TERMINE
	if(PORTBbits.RB3==0)
	{	//GARANTE QUE A DURA��O DO PULSO FOI CONTABILIZADA
		if(cont_tempo_pulso!=0)
		{	//C�LCULO DA DIST�NCIA EM CM
			dist=(cont_tempo_pulso)*0.17;
			//SE A DIST�NCIA SER MENOR DO QUE 4 CM ACIONA O BOT�O
			if(dist<5)
				{lim=6;}
			else
				{lim=24;}
sprintf(p,"%d\n",dist);
//ENVIA STRING POR UART PARA VALIDA��O
putsUART2 ((unsigned char *)p);
			}
		}
	}
}

void __attribute__((__interrupt__)) _T1Interrupt(void)
{
	//INTERRUP��O A CADA 0,1 MS PARA GERAR PWM POR SOFTWARE
	static int t=0;
	++t;
	//NIVEL ALTO
	if((t>=0) && (t<=lim))
		{LATBbits.LATB2=1;}

	//NIVEL BAIXO
	else if((t>lim) && (t<200))
		{LATBbits.LATB2=0;}

	else 
		{t=0;}
	
_T1IF=0;
}

void __attribute__((__interrupt__)) _T2Interrupt(void)
{
	//EXECUTA A CADA 10us
	static unsigned int delay_trigger=0;
	++delay_trigger;
	//ENVIA UM PULSO DE 10 us PARA O SENSOR
	if(delay_trigger==1)
	{
		LATBbits.LATB1=~LATBbits.LATB1;
	}

	else if(delay_trigger==2)
	{
	LATBbits.LATB1=~LATBbits.LATB1;
	}
//CONTA A DURA��O DO PULSO DO SENSOR EM TICKS DE 10 US
	if(PORTBbits.RB3==1)
	{//PORTBbits.RB8=1;
	++cont_tempo_pulso;
	}
//else{PORTBbits.RB8=0;}
//A CADA 100 MS ENVIA OUTRO PULSO
	if(delay_trigger==10000)
	{
	delay_trigger=0;
	cont_tempo_pulso=0;
	}

	_T2IF=0;
}

