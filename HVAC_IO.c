 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a través de estados y objetos.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"

/* Definición de botones. */
#define BOTON_MENU  BSP_BUTTON1     /* TEMP_PLUSBotones de suma y resta al valor deseado, funcionan con interrupciones. */
#define BOTON_UPDW  BSP_BUTTON2     //TEMP_MINUS

#define BOTON_ONOFF      BSP_BUTTON3     /* P2.3 FAN _ON ---- Botones para identificación del estado del sistema. */
/*#define FAN_AUTO    BSP_BUTTON4
#define SYSTEM_COOL BSP_BUTTON5
#define SYSTEM_OFF  BSP_BUTTON6
#define SYSTEM_HEAT BSP_BUTTON7*/

/* Definición de leds. */
#define ONOFF_LED     BSP_LED1        /* FAN_LED Leds para denotar el estado de las salidas. */
/*#define HEAT_LED    BSP_LED2        //HEAT_LED
#define HBeat_LED   BSP_LED3        //HBeat_LED
#define COOL_LED    BSP_LED4        //COOL_LED*/

/* Variables sobre las cuales se maneja el sistema. */

/*float TemperaturaActual = 20;  // Temperatura.
float SetPoint = 25.0;         // V. Deseado.*/

char state[MAX_MSG_SIZE];      // Cadena a imprimir.

/*bool toggle = 0;               // Toggle para el heartbeat.
_mqx_int delay;                // Delay aplicado al heartbeat.
bool event = FALSE;            // Evento I/O que fuerza impresión inmediata.

bool FAN_LED_State = 0;                                     // Estado led_FAN.
const char* SysSTR[] = {"Cool","Off","Heat","Only Fan"};    // Control de los estados.
*/
//MIS VARIABLES

bool event = FALSE;            // Evento I/O que fuerza impresión inmediata.
bool flag_P = FALSE;              //Si se presiona el botón 2 para P1 y P2
bool flag_LUM = FALSE;              //Si se presiona el botón 2 para LUM
bool ENCENDIDO = FALSE;


//VARIABLES LUM, INDICA EL ESTADO DE LAS LÁMPARAS
uint32_t LUM1_STATUS = Off;
uint32_t LUM2_STATUS = Off;
uint32_t LUM3_STATUS = Off;

//VARIABLES LUM, INDICA EL VALOR DE LAS LÁMPARAS
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
/* Archivos sobre los cuales se escribe toda la información */
FILE _PTR_ input_port = NULL, _PTR_ output_port = NULL;                  // Entradas y salidas.
FILE _PTR_ fd_adc = NULL, _PTR_ fd_ch_LUM1 = NULL,_PTR_ fd_ch_LUM2 = NULL, _PTR_ fd_ch_LUM3 = NULL;    // ADC: ch_T -> Temperature, ch_H -> Pot.
FILE _PTR_ fd_uart = NULL;                                               // Comunicación serial asíncrona.

// Estructuras iniciales.

const ADC_INIT_STRUCT adc_init =
{
    ADC_RESOLUTION_DEFAULT,                                                     // Resolución.
    ADC_CLKDiv8                                                                 // División de reloj.
};

/*const ADC_INIT_CHANNEL_STRUCT adc_ch_param =
{
    TEMPERATURE_ANALOG_PIN,                                                      // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP | ADC_CHANNEL_START_NOW | ADC_INTERNAL_TEMPERATURE, // Banderas de inicialización (temperatura)
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_1                                                                // Trigger lógico que puede activar este canal.
};

const ADC_INIT_CHANNEL_STRUCT adc_ch_param2 =
{
    AN1,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicialización (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_2                                                                // Trigger lógico que puede activar este canal.
};
*/
//POT_1.............LUM_1
const ADC_INIT_CHANNEL_STRUCT adc_ch_param =
{
    AN0,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicialización (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_1                                                                // Trigger lógico que puede activar este canal.
};

const ADC_INIT_CHANNEL_STRUCT adc_ch_param2 =
{
    AN1,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicialización (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_2                                                                // Trigger lógico que puede activar este canal.
};

const ADC_INIT_CHANNEL_STRUCT adc_ch_param3 =
{
    AN5,                                                                         // Fuente de lectura, 'ANx'.
    ADC_CHANNEL_MEASURE_LOOP,                                                    // Banderas de inicialización (pot).
    50000,                                                                       // Periodo en uS, base 1000.
    ADC_TRIGGER_3                                                                // Trigger lógico que puede activar este canal.
};

static uint_32 data[] =                                                          // Formato de las entradas.
{                                                                                // Se prefirió un solo formato.
     BOTON_MENU, //CAMBIAMOS TEMP_PLUS POR BOTON MENU
     BOTON_UPDW,
     BOTON_ONOFF,
     /*FAN_AUTO,
     SYSTEM_COOL,
     SYSTEM_OFF,
     SYSTEM_HEAT,*/

     GPIO_LIST_END
};

static const uint_32 onoff[] =     //fan                                               // Formato de los leds, uno por uno.
{
     ONOFF_LED,
     GPIO_LIST_END
};

/*static const uint_32 heat[] =                                                   // Formato de los leds, uno por uno.
{
     HEAT_LED,
     GPIO_LIST_END
};

const uint_32 hbeat[] =                                                         // Formato de los leds, uno por uno.
{
     HBeat_LED,
     GPIO_LIST_END
};

static const uint_32 cool[] =                                                   // Formato de los leds, uno por uno.
{
     COOL_LED,
     GPIO_LIST_END
};

*/
/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón SW1 o SW2.
 * Input: None.
 * Output: None.
 **********************************************************************************/
void INT_SWI(void)
{
    Int_clear_gpio_flags(input_port);

    ioctl(input_port, GPIO_IOCTL_READ, &data);

    if((data[0] & GPIO_PIN_STATUS) == 0)        // Lectura de los pines, índice cero es BOTON_MENU.
        HVAC_BotonMenu();

    /*else if((data[1] & GPIO_PIN_STATUS) == 0)   // Lectura de los pines, índice uno es TEMP_MINUS.
        HVAC_SetPointDown();*/

    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : boolean; inicialización correcta.
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
         /*HEAT_LED  | GPIO_PIN_STATUS_0,
         HBeat_LED | GPIO_PIN_STATUS_0,
         COOL_LED  | GPIO_PIN_STATUS_0,*/
         GPIO_LIST_END
    };

    const uint_32 input_set[] =
    {
        BOTON_MENU,
        BOTON_UPDW,
        BOTON_ONOFF,
        /*FAN_AUTO,
        SYSTEM_COOL,
        SYSTEM_OFF,
        SYSTEM_HEAT,*/

        GPIO_LIST_END
    };

    // Iniciando GPIO.
    ////////////////////////////////////////////////////////////////////

    output_port =  fopen_f("gpio:write", (char_ptr) &output_set);
    input_port =   fopen_f("gpio:read", (char_ptr) &input_set);

    if (output_port) { ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, NULL); }   // Inicialmente salidas apagadas.
    ioctl (input_port, GPIO_IOCTL_SET_IRQ_FUNCTION, INT_SWI);               // Declarando interrupción.

    return (input_port != NULL) && (output_port != NULL);
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : boolean; inicialización correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas para
*    el módulo general ADC y dos de sus canales; uno para la temperatura, otro para
*    el heartbeat.
*
*END***********************************************************************************/
boolean HVAC_InicialiceADC (void)
{

    // Iniciando ADC y canales.
    ////////////////////////////////////////////////////////////////////

    fd_adc   = fopen_f("adc:",  (const char*) &adc_init);               // Módulo.
    fd_ch_LUM1 =  fopen_f("adc:1", (const char*) &adc_ch_param);           // Canal uno, arranca al instante.
    fd_ch_LUM2 =  fopen_f("adc:2", (const char*) &adc_ch_param2);           // Canal uno, arranca al instante.
    fd_ch_LUM3 =  fopen_f("adc:3", (const char*) &adc_ch_param3);           // Canal uno, arranca al instante.

    return (fd_adc != NULL) && (fd_ch_LUM1 != NULL) && (fd_ch_LUM2 != NULL) && (fd_ch_LUM3 != NULL);  // Valida que se crearon los archivos.
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceUART
* Returned Value   : boolean; inicialización correcta.
* Comments         :
*    Abre los archivos e inicializa las configuraciones deseadas para
*    configurar el modulo UART (comunicación asíncrona).
*
*END***********************************************************************************/
boolean HVAC_InicialiceUART (void)
{
    // Estructura inicial de comunicación serie.
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
        /* Dirección TX */         LSB_FIRST,

        /* Int char's \b */        NO_INTERRUPTION,
        /* Int char's erróneos */  NO_INTERRUPTION
    };

    // Inicialización de archivo.
    fd_uart = fopen_f("uart:", (const char*) &uart_init);

    return (fd_uart != NULL); // Valida que se crearon los archivos.
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarEntradas
* Returned Value   : None.
* Comments         :
*    Actualiza los variables indicadores de las entradas sobre las cuales surgirán
*    las salidas.
*
*END***********************************************************************************/
void HVAC_ActualizarEntradas(void)
{
    /*static bool ultimos_estados[] = {FALSE, FALSE, FALSE, FALSE, FALSE};

    ioctl(fd_ch_LUM1, IOCTL_ADC_READ_TEMPERATURE, (pointer) &TemperaturaActual);   // Actualiza valor de temperatura.
    ioctl(input_port, GPIO_IOCTL_READ, &data);

    if((data[2] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)               // Cambia el valor de las entradas FAN.
    {
        EstadoEntradas.FanState = On;
        EstadoEntradas.SystemState = FanOnly;

        if(ultimos_estados[0] == FALSE)
            event = TRUE;

        ultimos_estados[0] = TRUE;
        ultimos_estados[1] = FALSE;
    }

    else if((data[3] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)          // Cambia el valor de las entradas SYSTEM.
    {
        EstadoEntradas.FanState = Auto;
        if(ultimos_estados[1] == FALSE)
            event = TRUE;

        ultimos_estados[1] = TRUE;
        ultimos_estados[0] = FALSE;

        if((data[4] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)           // Y así para el resto de pines.
        {
            EstadoEntradas.SystemState = Cool;
            if(ultimos_estados[2] == FALSE)
                event = TRUE;
            ultimos_estados[2] = TRUE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = FALSE;
        }
        else if((data[5] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)
        {
            EstadoEntradas.SystemState = Off;
            if(ultimos_estados[3] == FALSE)
                event = TRUE;
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = TRUE;
            ultimos_estados[4] = FALSE;
        }

        else if((data[6] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)
        {
            EstadoEntradas.SystemState = Heat;
            if(ultimos_estados[4] == FALSE)
                event = TRUE;
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = TRUE;
        }
        else
        {
            EstadoEntradas.SystemState = Off;
            ultimos_estados[2] = FALSE;
            ultimos_estados[3] = FALSE;
            ultimos_estados[4] = FALSE;
        }
    }*/
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarSalidas
* Returned Value   : None.
* Comments         :
*    Decide a partir de las entradas actualizadas las salidas principales,
*    y en ciertos casos, en base a una cuestión de temperatura, la salida del 'fan'.
*
*END***********************************************************************************/
/*void HVAC_ActualizarSalidas(void)
{
    // Cambia el valor de las salidas de acuerdo a entradas.

    if(EstadoEntradas.FanState == On)                               // Para FAN on.
    {
        FAN_LED_State = 1;
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);
    }

    else if(EstadoEntradas.FanState == Auto)                        // Para FAN automático.
    {
        switch(EstadoEntradas.SystemState)
        {
        case Off:   ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);
                    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
                    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);
                    FAN_LED_State = 0;
                    break;
        case Heat:  HVAC_Heat(); break;
        case Cool:  HVAC_Cool(); break;
        }
    }
}*/
void HVAC_ActualizarSalidas(void)
{
    //ENCENDIDO = ChecarBoton(); //Revisa el botón ON/OFF

    if(EstadoEntradas.SystemState == Constante) //ESTADO NORMAL
    {
        PRINT_SYSTEM_STATUS(); //IMPRIME VALORES ACTUALES
        sprintf(state,"\n\rPRESS BOTON_MENU\n\r");
        print(state);
        //Delay_ms(100);
        usleep(100000);
    }

    if(EstadoEntradas.SystemState == Menu) // ENTRA AL MENÚ
    {
        //ENCENDIDO = ChecarBoton(); //REVISA BOTÓN ON/OFF
        cont_menu = cont_menu + 1; //ACTUALIZA CONTADOR

        sprintf(state,"\n\r#OPCION %i:\n\r",cont_menu); //INDICA LA OPCIÓN DEL MENÚ ACTUAL
        print(state);
        //Delay_ms(2000);
        usleep(2000000);

        if(cont_menu == 1)
        {
            PRINT_SYSTEM_STATUS(); //IMPRIME VALORES ACTUALES
            sprintf(state,"\n\rP1_STATUS_SELECTED.\n\r");//INDICA LA OPCIÓN DEL MENÚ ACTUAL
            print(state);
            //Delay_ms(4000);
            usleep(4000000);
            Desplegar_Opcion(); //REVISA EL BOTÓN UP/DOWN

        }

        if(cont_menu == 2)
        {
            PRINT_SYSTEM_STATUS();//IMPRIME VALORES ACTUALES
            sprintf(state,"\n\rP2_STATUS_SELECTED.\n\r");//INDICA LA OPCIÓN DEL MENÚ ACTUAL
            print(state);
            //Delay_ms(4000);
            usleep(4000000);
            Desplegar_Opcion();//REVISA EL BOTÓN UP/DOWN
        }

        if(cont_menu == 3)
        {
            PRINT_SYSTEM_STATUS();//IMPRIME VALORES ACTUALES
            sprintf(state,"\n\rLUZ_SELECTED.\n\r");//INDICA LA OPCIÓN DEL MENÚ ACTUAL
            print(state);
            //Delay_ms(4000);
            usleep(4000000);
            Desplegar_Opcion();//REVISA EL BOTÓN UP/DOWN
        }

        if(cont_menu == 4)
        {
            EstadoEntradas.SystemState = Constante; //REGRESA A ESTADO NORMAL
            cont_menu = 0; //REINICIA CONTADOR DEL MENÚ
        }
    }

    if(EstadoEntradas.SystemState == UpDw) //CUANDO SE PRESIONA EL BOTÓN UP/DW EN CADA OPCIÓN DEL MENÚ
    {
        //Reinicia banderas que indican si se cambió el estado de una persiana o luces
        flag_P = FALSE;
        flag_LUM = FALSE;

        if(cont_menu == 1) //PARA LA OPCIÓN 1 DEL MENÚ, CAMBIA EL ESTADO DE P1
        {
            if(P1_STATUS == Close) //SI ESTÁ CERRADA
            {
                P1_STATUS = Open; //SE ABRE
                sprintf(state,"P1_UP.\n\r");// IMPRIME QUE SE ESTÁ ABRIENDO
                print(state);
                //Delay_ms(5000);
                usleep(5000000);
                EstadoEntradas.SystemState = Constante; //VUELVE A ESTADO NORMAL
            }else
            {                       //SI ESTÁ ABIERTA
                P1_STATUS = Close;//SE CIERRA
                sprintf(state,"P1_DOWN.\n\r");//IMPRIME QUE SE ESTÁ CERRANDO
                print(state);
                //Delay_ms(5000);
                usleep(5000000);
                EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
            }
        }

        if(cont_menu == 2)//PARA LA OPCIÓN 2 DEL MENÚ, CAMBIA EL ESTADO DE P2
                {
                    if(P2_STATUS == Close)
                    {
                        P2_STATUS = Open;
                        sprintf(state,"P2_UP.\n\r");// IMPRIME QUE SE ESTÁ ABRIENDO
                        print(state);
                        //Delay_ms(5000);
                        usleep(5000000);
                        EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
                    }else
                    {
                        P2_STATUS = Close;
                        sprintf(state,"P2_DOWN.\n\r");//IMPRIME QUE SE ESTÁ CERRANDO
                        print(state);
                        //Delay_ms(5000);
                        usleep(5000000);
                        EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
                    }
                }

        if(cont_menu == 3)//PARA LA OPCIÓN 3 DEL MENÚ, CAMBIA EL ESTADO DE LAS LUCES
                {
                    if(LUM1_STATUS == Off) //SI ESTÁN APAGADAS SE PRENDEN
                    {
                        LUM1_STATUS = On;
                        LUM2_STATUS = On;
                        LUM3_STATUS = On;
                        //Delay_ms(1000);
                        usleep(1000000);
                        EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
                    }else
                    {
                        LUM1_STATUS = Off; //SI ESTÁN PRENDIDAS SE APAGAN
                        LUM2_STATUS = Off;
                        LUM3_STATUS = Off;
                        //Delay_ms(1000);
                        usleep(1000000);
                        EstadoEntradas.SystemState = Constante;//VUELVE A ESTADO NORMAL
                    }
                }
    }

    //return (ENCENDIDO); //REGRESA EL VALOR DE ENCENDIDO PARA REVISAR EL BOTÓN ON/OFF
}
/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heat
* Returned Value   : None.
* Comments         :
*    Decide a partir de la temperatura actual y la deseada, si se debe activar el fan.
*    (La temperatura deseada debe ser mayor a la actual). El estado del fan debe estar
*    en 'auto' y este modo debe estar activado para entrar a la función.
*
*END***********************************************************************************/
/*void HVAC_Heat(void)
{
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &heat);
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &cool);

    if(TemperaturaActual < SetPoint)                        // El fan se debe encender si se quiere una temp. más alta.
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);
        FAN_LED_State = 1;
    }
    else
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);
        FAN_LED_State = 0;
    }
}
*/
/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Cool
* Returned Value   : None.
* Comments         :
*    Decide a partir de la temperatura actual y la deseada, si se debe activar el fan.
*    (La temperatura deseada debe ser menor a la actual). El estado del fan debe estar
*    en 'auto' y este modo debe estar activado para entrar a la función.
*
*END***********************************************************************************/
/*void HVAC_Cool(void)
{
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &heat);
    ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &cool);

    if(TemperaturaActual > SetPoint)                        // El fan se debe encender si se quiere una temp. más baja.
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &fan);
        FAN_LED_State = 1;
    }
    else
    {
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &fan);
        FAN_LED_State = 0;
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_Heartbeat
* Returned Value   : None.
* Comments         :
*    Función que prende y apaga una salida para notificar que el sistema está activo.
*    El periodo en que se hace esto depende de una entrada del ADC en esta función.
*
*END***********************************************************************************/
/*void HVAC_Heartbeat(void)               // Función de 'alive' del sistema.
{
   _mqx_int val = 0;
   boolean flag = TRUE;
   static _mqx_int delay_en_curso = 0;
   static boolean bandera_inicial = 0;

   if(bandera_inicial == 0)
   {
       // Entrando por primera vez, empieza a correr el canal de heartbeat.
       ioctl (fd_ch_H, IOCTL_ADC_RUN_CHANNEL, NULL);
       bandera_inicial = 1;
   }

   //Valor se guarda en val, flag nos dice si fue exitoso.
   flag =  (fd_adc && fread_f(fd_ch_H, &val, sizeof(val))) ? 1 : 0;

   if(flag != TRUE)
   {
       printf("Error al leer archivo. Cierre del programa\r\n");
       exit(1);
   }

    delay = 15000 + (100* val / 4);         // Lectura del ADC por medio de la función.
    //Nota: delay no puede ser mayor a 1,000,000 ya que luego se generan problemas en usleep.

    delay_en_curso -= DELAY;
    if(delay_en_curso <= 0)
    {
        delay_en_curso = delay;
        toggle ^= 1;                          // Toggle.
    }

    if(toggle)
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &hbeat);
    else
        ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &hbeat);

    if(delay_en_curso < DELAY)
        usleep(delay_en_curso);
    else
        usleep(DELAY);

    return;
}*/



/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situación actual del sistema en términos de temperaturas
*    actual y deseada, estado del abanico, del sistema y estado de las entradas.
*    Imprime cada cierto número de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciéndose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    /*static char iterations = 0;

    iterations++;       */                                    // A base de iteraciones para imprimir cada cierto tiempo.
    if(event == TRUE)
    {

          event = FALSE;
          sprintf(state," HAY UN EVENTO"); //IMPRIME QUE HAY UN EVENTO
          print(state);
          //Delay_ms(3000);
          usleep(3000000);
          EstadoEntradas.SystemState = Menu; //CAMBIA EL ESTADO DEL SISTEMA A MENÚ

        /*sprintf(state,"Fan: %s, System: %s, SetPoint: %0.2f\n\r",
                    EstadoEntradas.FanState == On? "On":"Auto",
                    SysSTR[EstadoEntradas.SystemState],
                    SetPoint);
        print(state);

        sprintf(state,"Temperatura Actual: %0.2f°C %0.2f°F  Fan: %s\n\r\n\r",
                    TemperaturaActual,
                    ((TemperaturaActual*9.0/5.0) + 32),
                    FAN_LED_State?"On":"Off");
        print(state);*/
    }
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_SetPointUp
* Returned Value   : None.
* Comments         :
*    Sube el valor deseado (set point). Llamado por interrupción a causa del SW1.
*
*END***********************************************************************************/
void HVAC_BotonMenu(void)
{
    event = TRUE;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_SetPointDown
* Returned Value   : None.
* Comments         :
*    Baja el valor deseado (set point). Llamado por interrupción a causa del SW2.
*
*END***********************************************************************************/
/*void HVAC_SetPointDown(void)
{
    SetPoint -= 0.5;
    event = TRUE;
}*/
void Desplegar_Opcion(void)
{

    if(cont_menu == 1|| cont_menu == 2) //OPCION PARA PERSIANAS
    {
        sprintf(state," PRESIONA EL BOTÓN 2 PARA ABRIR/CERRAR PERSIANA. TIENES 8 SEGUNDOS.\n\r");
        print(state);

        for(i=0;i<80;i++)//REVISA SI SE PRESIONA EL BOTÓN POR 8 SEGUNDOS
        {
            if((data[1] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)
            {
                EstadoEntradas.SystemState = UpDw; //SI SÍ, EL ESTADO DEL SISTEMA CAMBIA A UPDW
                flag_P=TRUE;// SE INDICA QUE SÍ SE PRESIONÓ EL BOTÓN
            }

            //Delay_ms(100);
            usleep(100000);
        }

        if(flag_P==FALSE) //SI NO SE PRESIONA EL BOTÓN
        {
            EstadoEntradas.SystemState = Constante; // CAMBIA EL ESTADO DEL SISTEMA A NORMAL.
        }
    }

    if(cont_menu == 3) //OPCION PARA LUCES
    {
        sprintf(state," PRESIONA EL BOTÓN 2 PARA PRENDER/APAGAR LUCES. TIENES 8 SEGUNDOS.\n\r");
        print(state);

        for(i=0;i<80;i++)//REVISA SI SE PRESIONA EL BOTÓN POR 8 SEGUNDOS
        {
            if((data[1] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS)
            {

                EstadoEntradas.SystemState = UpDw;//SI SÍ, EL ESTADO DEL SISTEMA CAMBIA A UPDW

                flag_LUM = TRUE; // SE INDICA QUE SÍ SE PRESIONÓ EL BOTÓN
            }

            //Delay_ms(100)
            usleep(100000);
        }

        if(flag_LUM==FALSE) //SI NO SE PRESIONA EL BOTÓN
        {
            EstadoEntradas.SystemState = Constante; // CAMBIA EL ESTADO DEL SISTEMA A NORMAL.
        }
    }

    if(cont_menu == 4) //OPCION PARA CERRAR MENU
    {
        EstadoEntradas.SystemState = Constante; // CAMBIA EL ESTADO DEL SISTEMA A NORMAL.
        cont_menu = 0; //REINICIA CONTADOR DEL MENÚ
    }
}

//FUNCIÓN QUE RETORNA EL VALOR DE LOS POTS ENTRE 1-10 CON LA CONVERSIÓN ADC 14 BITS DE RESOLUCIÓN. LAMPARA 1.
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

        value = 0 + (10* lectura / 16300);         // Lectura del ADC por medio de la función.
        return value;
}

//FUNCIÓN QUE RETORNA EL VALOR DE LOS POTS ENTRE 1-10 CON LA CONVERSIÓN ADC 14 BITS DE RESOLUCIÓN. LAMPARA 2.
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

        value = 0 + (10* lectura / 16300);         // Lectura del ADC por medio de la función.
        return value;
}

//FUNCIÓN QUE RETORNA EL VALOR DE LOS POTS ENTRE 1-10 CON LA CONVERSIÓN ADC 14 BITS DE RESOLUCIÓN. LAMPARA 3.
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

        value = 0 + (10* lectura / 16300);         // Lectura del ADC por medio de la función.
        return value;
}

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

//FUNCIÓN PARA REVISAR EL BOTÓN ON/OFF.
//ON: ENCIENDE EL LED Y REGRESA LA VARIABLE ENCENDIDO == TRUE
//OFF: APAGA EL LED Y REGRESA LA VARIABLE ENCENDIDO == FALSE
bool ChecarBoton(void)
{

        if((data[2] & GPIO_PIN_STATUS) != NORMAL_STATE_EXTRA_BUTTONS) //REVISA EL BOTÓN ON/OFF
       {
            //Delay_ms(2000);

            //SI ESTÁ ENCENDIDO EL SISTEMA, LO APAGA Y VICEVERSA
           if(ENCENDIDO)
           {
               sprintf(state,"\n\rSISTEMA APAGADO\n\r");
               print(state);
               ENCENDIDO = FALSE;
               ioctl(output_port, GPIO_IOCTL_WRITE_LOG0, &onoff);
           }else
           {
               sprintf(state,"\n\rSISTEMA ENCENDIDO\n\r");
               print(state);
               ioctl(output_port, GPIO_IOCTL_WRITE_LOG1, &onoff);
               ENCENDIDO = TRUE;
           }
       }

    return(ENCENDIDO); //REGRESA LA VARIABLE ENCENDIDO
}


