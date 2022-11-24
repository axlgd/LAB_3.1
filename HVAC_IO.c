 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a trav�s de estados y objetos.
 // Authors:         Jos� Luis Chac�n M. y Jes�s Alejandro Navarro Acosta.
 // Updated:         11/2018

//EDITADO POR EL EQUIPO 6 DE ARQUITECTURA DE PROGRAMACI�N PARA CONTROL DE HARDWARE SEMESTRE AGO-DIC 2022
//ANAHI GONZALEZ HOLGU�N
//AXEL GAY D�AZ
//CARLOS ALBERTO GONZ�LEZ V�ZQUEZ
//CARLOS L�PEZ LARA
//UPDATED: 23/11/2022


#include "HVAC.h"

/* Definici�n de botones. */
#define BOTON_MENU  BSP_BUTTON1     //1.1 BOTON 1 MENU
#define BOTON_UPDW  BSP_BUTTON2     //1.4 BOT�N 2 UP/DOWN

#define BOTON_ONOFF      BSP_BUTTON3


/* Definici�n de leds. */
#define ONOFF_LED     BSP_LED1


char state[MAX_MSG_SIZE];      // Cadena a imprimir.

//VARIABLES PARA BANDERAS
bool event = FALSE;            // Evento I/O que fuerza impresi�n inmediata.
bool flag_P = FALSE;              //Si se presiona el bot�n 2 para P1 y P2
bool flag_LUM = FALSE;              //Si se presiona el bot�n 2 para LUM


//VARIABLES LUM, INDICA EL ESTADO DE LAS L�MPARAS
uint32_t LUM1_STATUS = Off;
uint32_t LUM2_STATUS = Off;
uint32_t LUM3_STATUS = Off;

//VARIABLES LUM, INDICA EL VALOR DE LAS L�MPARAS
uint32_t LUM1_VALUE = 0;
uint32_t LUM2_VALUE = 0;
uint32_t LUM3_VALUE = 0;

//VARIABLES P, INDICA EL ESTADO DE LAS PERSIANAS
uint32_t P1_STATUS= Close;
uint32_t P2_STATUS= Close;

//Contadores
uint32_t cont_menu = 0; //Veces que entra al menu

//Indice
uint32_t i;
/* Archivos sobre los cuales se escribe toda la informaci�n */
FILE _PTR_ input_port = NULL, _PTR_ output_port = NULL;                  // Entradas y salidas.
FILE _PTR_ fd_adc = NULL, _PTR_ fd_ch_LUM1 = NULL,_PTR_ fd_ch_LUM2 = NULL, _PTR_ fd_ch_LUM3 = NULL;
FILE _PTR_ fd_uart = NULL;                                               // Comunicaci�n serial as�ncrona.

// ESTRUCTURAS INICIALES PARA ADC

const ADC_INIT_STRUCT adc_init =
{
    ADC_RESOLUTION_DEFAULT,                                                     // Resoluci�n.
    ADC_CLKDiv8                                                                 // Divisi�n de reloj.
};


//LUM 1 -> CANAL 1. ADC AN0.
const ADC_INIT_CHANNEL_STRUCT adc_ch_param =
{
    AN0,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicializaci�n (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_1                                                                // Trigger l�gico que puede activar este canal.
};

//LUM 2 -> CANAL 2. ADC AN1.
const ADC_INIT_CHANNEL_STRUCT adc_ch_param2 =
{
    AN1,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicializaci�n (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_2                                                                // Trigger l�gico que puede activar este canal.
};

//LUM 3 -> CANAL 3. ADC AN5.
const ADC_INIT_CHANNEL_STRUCT adc_ch_param3 =
{
    AN5,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicializaci�n (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_3                                                                // Trigger l�gico que puede activar este canal.
};

// ARREGLO PARA ALMACENAR EL VALOR DE LA ENTRADA DE CADA BOT�N

static uint_32 data[] =                                                          // Formato de las entradas.
{                                                                                // Se prefiri� un solo formato.
     BOTON_MENU,
     BOTON_UPDW,
     BOTON_ONOFF,


     GPIO_LIST_END
};

/*FUNCTION******************************************************************************
*
* Function Name    : INT_SWI
* Returned Value   :
* Comments         :
*    CREA LA INTERRUPCI�N CUANDO SE PRESIONA EL BOT�N DE MEN� Y MANDA HABLAR A LA FUNCI�N HVAC_BotonMenu();
*
*END***********************************************************************************/

void INT_SWI(void)
{
    Int_clear_gpio_flags(input_port);

    ioctl(input_port, GPIO_IOCTL_READ, &data);

    if((data[0] & GPIO_PIN_STATUS) == 0)        // Lectura de los pines, �ndice cero es BOTON_MENU.
        HVAC_BotonMenu();
    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : boolean; inicializaci�n correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas de entrada y salida GPIO.
*
*END***********************************************************************************/

boolean HVAC_InicialiceIO(void)
{
    // Estructuras iniciales de entradas y salidas.

    const uint_32 output_set[] =
    {
         ONOFF_LED   | GPIO_PIN_STATUS_0,

         GPIO_LIST_END
    };

    const uint_32 input_set[] =
    {
        BOTON_MENU,
        BOTON_UPDW,
        BOTON_ONOFF,


        GPIO_LIST_END
    };

    // Iniciando GPIO.
    ////////////////////////////////////////////////////////////////////

    output_port =  fopen_f("gpio:write", (char_ptr) &output_set);
    input_port =   fopen_f("gpio:read", (char_ptr) &input_set);

    if (output_port) { ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, NULL); }   // Inicialmente salidas apagadas.
    ioctl (input_port, GPIO_IOCTL_SET_IRQ_FUNCTION, INT_SWI);               // Declarando interrupci�n.

    return (input_port != NULL) && (output_port != NULL);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : boolean; inicializaci�n correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas para
*    el m�dulo general ADC y tres de sus canales, uno para cada POT.
*
*END***********************************************************************************/
boolean HVAC_InicialiceADC (void)
{

    // Iniciando ADC y canales.
    ////////////////////////////////////////////////////////////////////

    fd_adc   = fopen_f("adc:",  (const char*) &adc_init);               // M�dulo.
    fd_ch_LUM1 =  fopen_f("adc:1", (const char*) &adc_ch_param);           // Canal uno. POT1
    fd_ch_LUM2 =  fopen_f("adc:2", (const char*) &adc_ch_param2);           // Canal dos. POT2
    fd_ch_LUM3 =  fopen_f("adc:3", (const char*) &adc_ch_param3);           // Canal tres. POT3

    return (fd_adc != NULL) && (fd_ch_LUM1 != NULL) && (fd_ch_LUM2 != NULL) && (fd_ch_LUM3 != NULL);  // Valida que se crearon los archivos.
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceUART
* Returned Value   : boolean; inicializaci�n correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas para
*    configurar el modulo UART (comunicaci�n as�ncrona).
*
*END***********************************************************************************/
boolean HVAC_InicialiceUART (void)
{
    // Estructura inicial de comunicaci�n serie.
    const UART_INIT_STRUCT uart_init =
    {
        /* Selected port */        1,
        /* Selected pins */        {1,2},
        /* Clk source */           SM_CLK,
        /* Baud rate */            BR_115200,

        /* Usa paridad */          NO_PARITY,
        /* Bits protocolo  */      EIGHT_BITS,
        /* Sobremuestreo */        OVERSAMPLE,
        /* Bits de stop */         ONE_STOP_BIT,
        /* Direcci�n TX */         LSB_FIRST,

        /* Int char's \b */        NO_INTERRUPTION,
        /* Int char's err�neos */  NO_INTERRUPTION
    };

    // Inicializaci�n de archivo.
    fd_uart = fopen_f("uart:", (const char*) &uart_init);

    return (fd_uart != NULL); // Valida que se crearon los archivos.
}

void HVAC_ActualizarSalidas(void)
{

    if(EstadoEntradas.SystemState == Constante) //ESTADO NORMAL
    {
        PRINT_SYSTEM_STATUS(); //IMPRIME VALORES ACTUALES
        sprintf(state,"\n\rPRESS BOTON_MENU\n\r");
        print(state);
        //Delay_ms(100);
        usleep(10000);
    }

    if(EstadoEntradas.SystemState == Menu) // ENTRA AL MEN�
    {
        cont_menu = cont_menu + 1; //ACTUALIZA CONTADOR

        sprintf(state,"\n\r#OPCION %i:\n\r",cont_menu); //INDICA LA OPCI�N DEL MEN� ACTUAL
        print(state);
        //Delay_ms(2000);
        usleep(2000000);

        if(cont_menu == 1)
        {
            PRINT_SYSTEM_STATUS(); //IMPRIME VALORES ACTUALES
            sprintf(state,"\n\rP1_STATUS_SELECTED.\n\r");//INDICA LA OPCI�N DEL MEN� ACTUAL
            print(state);
            //Delay_ms(4000);
            usleep(4000000);
            Desplegar_Opcion(); //REVISA EL BOT�N UP/DOWN

        }

        if(cont_menu == 2)
        {
            PRINT_SYSTEM_STATUS();//IMPRIME VALORES ACTUALES
            sprintf(state,"\n\rP2_STATUS_SELECTED.\n\r");//INDICA LA OPCI�N DEL MEN� ACTUAL
            print(state);
            //Delay_ms(4000);
            usleep(4000000);
            Desplegar_Opcion();//REVISA EL BOT�N UP/DOWN
        }

        if(cont_menu == 3)
        {
            PRINT_SYSTEM_STATUS();//IMPRIME VALORES ACTUALES
            sprintf(state,"\n\rLUZ_SELECTED.\n\r");//INDICA LA OPCI�N DEL MEN� ACTUAL
            print(state);
            //Delay_ms(4000);
            usleep(4000000);
            Desplegar_Opcion();//REVISA EL BOT�N UP/DOWN
        }

        if(cont_menu == 4)
        {
            EstadoEntradas.SystemState = Constante; //REGRESA A ESTADO NORMAL
            cont_menu = 0; //REINICIA CONTADOR DEL MEN�
        }
    }

    if(EstadoEntradas.SystemState == UpDw) //CUANDO SE PRESIONA EL BOT�N UP/DW EN CADA OPCI�N DEL MEN�
    {
        //Reinicia banderas que indican si se cambi� el estado de una persiana o luces
        flag_P = FALSE;
        flag_LUM = FALSE;

        if(cont_menu == 1) //PARA LA OPCI�N 1 DEL MEN�, CAMBIA EL ESTADO DE P1
        {
            if(P1_STATUS == Close) //SI EST� CERRADA
            {
                P1_STATUS = Open; //SE ABRE
                sprintf(state,"P1_UP.\n\r");// IMPRIME QUE SE EST� ABRIENDO
                print(state);
                //Delay_ms(5000);
                sleep(5);
                EstadoEntradas.SystemState = Constante; //VUELVE A ESTADO NORMAL
            }else
            {                       //SI EST� ABIERTA
                P1_STATUS = Close;//SE CIERRA
                sprintf(state,"P1_DOWN.\n\r");//IMPRIME QUE SE EST� CERRANDO
                print(state);
                //Delay_ms(5000);
                sleep(5);
                EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
            }
        }

        if(cont_menu == 2)//PARA LA OPCI�N 2 DEL MEN�, CAMBIA EL ESTADO DE P2
                {
                    if(P2_STATUS == Close)
                    {
                        P2_STATUS = Open;
                        sprintf(state,"P2_UP.\n\r");// IMPRIME QUE SE EST� ABRIENDO
                        print(state);
                        //Delay_ms(5000);
                        sleep(5);
                        EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
                    }else
                    {
                        P2_STATUS = Close;
                        sprintf(state,"P2_DOWN.\n\r");//IMPRIME QUE SE EST� CERRANDO
                        print(state);
                        //Delay_ms(5000);
                        sleep(5);
                        EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
                    }
                }

        if(cont_menu == 3)//PARA LA OPCI�N 3 DEL MEN�, CAMBIA EL ESTADO DE LAS LUCES
                {
                    if(LUM1_STATUS == Off) //SI EST�N APAGADAS SE PRENDEN
                    {
                        LUM1_STATUS = On;
                        LUM2_STATUS = On;
                        LUM3_STATUS = On;
                        //Delay_ms(1000);
                        sleep(1);
                        EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
                    }else
                    {
                        LUM1_STATUS = Off; //SI EST�N PRENDIDAS SE APAGAN
                        LUM2_STATUS = Off;
                        LUM3_STATUS = Off;
                        //Delay_ms(1000);
                        sleep(1);
                        EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
                    }
                }
    }

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Revisa si se gener� un evento con el bot�n men�.
*END***********************************************************************************/
void HVAC_PrintState(void)
{

    if(event == TRUE)
    {

          event = FALSE;
          sprintf(state,"\n\r HAY UN EVENTO\n\r"); //IMPRIME QUE HAY UN EVENTO
          print(state);
          //Delay_ms(3000);
          usleep(3000000);
          EstadoEntradas.SystemState = Menu; //CAMBIA EL ESTADO DEL SISTEMA A MEN�


    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_BOTON_MENU
* Returned Value   : None.
* Comments         :
*    Genera un evento a causa del SW1 cuando se presiona el bot�n Menu.
*
*END***********************************************************************************/

void HVAC_BotonMenu(void)
{
    event = TRUE;
}

/*FUNCTION******************************************************************************
*
* Function Name    : Desplegar_Opcion
* Returned Value   : None.
* Comments         :
*    MUESTRA LO QUE PUEDES REALIZAR ESTANDO EN ALGUNA DE LAS OPCIONES DEL MEN�.
*    VUELVE AL ESTADO NORMAL SI NO REALIZAS NADA Y SE DIRIGE AL ESTADO UP/DOWN
*    SI PRESIONAS EL BOT�N UP/DW.
*
*END***********************************************************************************/

void Desplegar_Opcion(void)
{
    ioctl(input_port, GPIO_IOCTL_READ, &data);

    if(cont_menu == 1|| cont_menu == 2) //OPCION PARA PERSIANAS
    {
        sprintf(state," PRESIONA EL BOT�N 2 PARA ABRIR/CERRAR PERSIANA. TIENES 5 SEGUNDOS.\n\r");
        print(state);

        for(i=0;i<10;i++)//REVISA SI SE PRESIONA EL BOT�N POR 8 SEGUNDOS
        {
            ioctl(input_port, GPIO_IOCTL_READ, &data);
            if((data[1] & GPIO_PIN_STATUS) == NORMAL_STATE_EXTRA_BUTTONS)
            {
                EstadoEntradas.SystemState = UpDw; //SI S�, EL ESTADO DEL SISTEMA CAMBIA A UPDW
                flag_P=TRUE;// SE INDICA QUE S� SE PRESION� EL BOT�N

            }

            //Delay_ms(100);
        }

        if(flag_P==FALSE) //SI NO SE PRESIONA EL BOT�N
        {
            EstadoEntradas.SystemState = Constante; // CAMBIA EL ESTADO DEL SISTEMA A NORMAL.
        }
    }

    if(cont_menu == 3) //OPCION PARA LUCES
    {
        sprintf(state," PRESIONA EL BOT�N 2 PARA PRENDER/APAGAR LUCES. TIENES 5 SEGUNDOS.\n\r");
        print(state);

        for(i=0;i<10;i++)//REVISA SI SE PRESIONA EL BOT�N POR 8 SEGUNDOS
        {
            ioctl(input_port, GPIO_IOCTL_READ, &data);

            if((data[1] & GPIO_PIN_STATUS) == NORMAL_STATE_EXTRA_BUTTONS)
            {

                EstadoEntradas.SystemState = UpDw;//SI S�, EL ESTADO DEL SISTEMA CAMBIA A UPDW

                flag_LUM = TRUE; // SE INDICA QUE S� SE PRESION� EL BOT�N
            }

            //Delay_ms(100)
        }

        if(flag_LUM==FALSE) //SI NO SE PRESIONA EL BOT�N
        {
            EstadoEntradas.SystemState = Constante; // CAMBIA EL ESTADO DEL SISTEMA A NORMAL.
        }
    }

    if(cont_menu == 4) //OPCION PARA CERRAR MENU
    {
        EstadoEntradas.SystemState = Constante; // CAMBIA EL ESTADO DEL SISTEMA A NORMAL.
        cont_menu = 0; //REINICIA CONTADOR DEL MEN�
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    :  GET_LUM1_VALUE
* Returned Value   : value.
* Comments         :
*    FUNCI�N QUE RETORNA EL VALOR DE LOS POTS ENTRE 1-10 CON LA CONVERSI�N ADC 14 BITS DE RESOLUCI�N. LAMPARA 1.
*
*END***********************************************************************************/

uint32_t    GET_LUM1_VALUE(void)
{
        uint32_t value;
        _mqx_int lectura = 0;
       boolean flag = TRUE;
       static boolean bandera_inicial = 0;

       if(bandera_inicial == 0)
       {
           // Entrando por primera vez, empieza a correr el canal de heartbeat.
           ioctl (fd_ch_LUM1, IOCTL_ADC_RUN_CHANNEL, NULL);
           bandera_inicial = 1;
       }

       //Valor se guarda en val, flag nos dice si fue exitoso.
       flag =  (fd_adc && fread_f(fd_ch_LUM1, &lectura, sizeof(lectura))) ? 1 : 0;

       if(flag != TRUE)
       {
           printf("Error al leer archivo. Cierre del programa\r\n");
           exit(1);
       }

        value = 0 + (10* lectura / 16300);         // Lectura del ADC por medio de la funci�n.
        return value;
}

/*FUNCTION******************************************************************************
*
* Function Name    :  GET_LUM2_VALUE
* Returned Value   : value.
* Comments         :
*    FUNCI�N QUE RETORNA EL VALOR DE LOS POTS ENTRE 1-10 CON LA CONVERSI�N ADC 14 BITS DE RESOLUCI�N. LAMPARA 2.
*
*END***********************************************************************************/
uint32_t    GET_LUM2_VALUE(void)
{
        uint32_t value;
        _mqx_int lectura = 0;
       boolean flag = TRUE;
       static boolean bandera_inicial = 0;

       if(bandera_inicial == 0)
       {
           // Entrando por primera vez, empieza a correr el canal de heartbeat.
           ioctl (fd_ch_LUM2, IOCTL_ADC_RUN_CHANNEL, NULL);
           bandera_inicial = 1;
       }

       //Valor se guarda en val, flag nos dice si fue exitoso.
       flag =  (fd_adc && fread_f(fd_ch_LUM2, &lectura, sizeof(lectura))) ? 1 : 0;

       if(flag != TRUE)
       {
           printf("Error al leer archivo. Cierre del programa\r\n");
           exit(1);
       }

        value = 0 + (10* lectura / 16300);         // Lectura del ADC por medio de la funci�n.
        return value;
}

/*FUNCTION******************************************************************************
*
* Function Name    :  GET_LUM3_VALUE
* Returned Value   : value.
* Comments         :
*    FUNCI�N QUE RETORNA EL VALOR DE LOS POTS ENTRE 1-10 CON LA CONVERSI�N ADC 14 BITS DE RESOLUCI�N. LAMPARA 3.
*
*END***********************************************************************************/
uint32_t    GET_LUM3_VALUE(void)
{
        uint32_t value;
        _mqx_int lectura = 0;
       boolean flag = TRUE;
       static boolean bandera_inicial = 0;

       if(bandera_inicial == 0)
       {
           // Entrando por primera vez, empieza a correr el canal de heartbeat.
           ioctl (fd_ch_LUM3, IOCTL_ADC_RUN_CHANNEL, NULL);
           bandera_inicial = 1;
       }

       //Valor se guarda en val, flag nos dice si fue exitoso.
       flag =  (fd_adc && fread_f(fd_ch_LUM3, &lectura, sizeof(lectura))) ? 1 : 0;

       if(flag != TRUE)
       {
           printf("Error al leer archivo. Cierre del programa\r\n");
           exit(1);
       }

        value = 0 + (10* lectura / 16300);         // Lectura del ADC por medio de la funci�n.
        return value;
}

/*FUNCTION******************************************************************************
*
* Function Name    :  PRINT_SYSTEM_STATUS
* Returned Value   : None.
* Comments         :
*    FUNCI�N IMPRIME EL VALOR DE CADA ELEMENTO DEL SISTEMA.
*    LUM1-3 Y P1 Y P2.
*
*END***********************************************************************************/

void PRINT_SYSTEM_STATUS(void)
{
    //PERSIANA 1
    sprintf(state, "P1= ");
    print(state);
    if(P1_STATUS== Open)
    {
        sprintf(state, "OPEN");
        print(state);
    }else
    {
        sprintf(state, "CLOSE");
        print(state);
    }

    //Delay_ms(100);
    usleep(100000);

    //PERSIANA 2
    sprintf(state, " P2= ");
        print(state);
        if(P2_STATUS== Open)
        {
            sprintf(state, "OPEN");
            print(state);
        }else
        {
            sprintf(state, "CLOSE");
            print(state);
        }
    //Delay_ms(100);
    usleep(100000);

    //LUM1
    sprintf(state,"  LUM1= ");
    print(state);
    if(LUM1_STATUS==Off)
    {
        sprintf(state,"OFF");
        print(state);
    }else
    {
       sprintf(state,"%i",GET_LUM1_VALUE());
       print(state);
    }
    //Delay_ms(100);
    usleep(100000);
    //LUM2
    sprintf(state,"  LUM2= ");
        print(state);
        if(LUM2_STATUS==Off)
        {
            sprintf(state,"OFF");
            print(state);
        }else
        {
           sprintf(state,"%i",GET_LUM2_VALUE());
           print(state);
        }

    //Delay_ms(100);
        usleep(100000);
    //LUM3
    sprintf(state,"  LUM3= ");
    print(state);
    if(LUM3_STATUS==Off)
    {
        sprintf(state,"OFF");
        print(state);
    }else
    {
       sprintf(state,"%i",GET_LUM3_VALUE());
       print(state);
    }
    //Delay_ms(100);
    usleep(100000);
}


