// Projeto 2 - ENTREGA FINAL

// Pedro da Silva Panini nºusp:15483543
// Lucas Gomes Pacheco nºusp:15697253
// Carlos Eduardo Cintra Siqueira Rodrigues de Mattos nºusp:15445279

// Bibliotecas habilitadas no MikroC: LCD, Conversions e ADC

// Configuração dos pinos do Display LCD
sbit LCD_RS at RD4_bit;
sbit LCD_EN at RD5_bit;
sbit LCD_D4 at RD0_bit;
sbit LCD_D5 at RD1_bit;
sbit LCD_D6 at RD2_bit;
sbit LCD_D7 at RD3_bit;

sbit LCD_RS_Direction at TRISD4_bit;
sbit LCD_EN_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD0_bit;
sbit LCD_D5_Direction at TRISD1_bit;
sbit LCD_D6_Direction at TRISD2_bit;
sbit LCD_D7_Direction at TRISD3_bit;

// Variáveis Globais de Controle
char modo_longo = 1;       // 1 = Duração Longa (60s), 0 = Duração Curta (10s)
char rodando = 0;          // Flag de estado do processo (0 = Parado, 1 = Em aferição)
int tempo_restante = 60;   // Tempo atual no display
int tmr1_cont = 0;         // Contador auxiliar para TMR1 (4 x 250ms = 1s)

// tratamento de Bouncing dos botões
char bounce_flag0 = 0;
char bounce_flag1 = 0;

// Rotina de Interrupção
void Interrupt() {
    // Interrupção Externa 0 (Botão Superior INT0 / RB0) - ATIVA 60 SEGUNDOS
    if (INTCON.INT0IF) {
        if (bounce_flag0 == 0) {
            bounce_flag0 = 1;

            // Se já está rodando nos 60s, apertar de novo faz parar (STOP)
            if (rodando && modo_longo == 1) {
                rodando = 0;
            } else {
                // Inicia o processo de 60s imediatamente
                modo_longo = 1;
                tempo_restante = 60;
                rodando = 1;
                TMR0H = 0xE1; TMR0L = 0x7C; // Reseta Timer0
                TMR1H = 0x0B; TMR1L = 0xDC; // Reseta Timer1
                tmr1_cont = 0;
            }
        }
        INTCON.INT0IF = 0;
    }

    // Interrupção Externa 1 (Botão Inferior INT1 / RB1) - ATIVA 10 SEGUNDOS
    if (INTCON3.INT1IF) {
        if (bounce_flag1 == 0) {
            bounce_flag1 = 1;

            // Se já está rodando nos 10s, apertar de novo faz parar (STOP)
            if (rodando && modo_longo == 0) {
                rodando = 0;
            } else {
                // Inicia o processo de 10s imediatamente
                modo_longo = 0;
                tempo_restante = 10;
                rodando = 1;
                TMR0H = 0xE1; TMR0L = 0x7C; // Reseta Timer0
                TMR1H = 0x0B; TMR1L = 0xDC; // Reseta Timer1
                tmr1_cont = 0;
            }
        }
        INTCON3.INT1IF = 0;
    }

    // Interrupção Timer 0 (Base de 1 segundo - Modo Longo)
    if (INTCON.TMR0IF) {
        TMR0H = 0xE1;
        TMR0L = 0x7C;
        if (rodando && modo_longo) {
            if (tempo_restante > 0) tempo_restante--;
            if (tempo_restante == 0) rodando = 0; // Desliga tudo ao acabar
        }
        INTCON.TMR0IF = 0;
    }

    // Interrupção Timer 1 (Base de 250 milissegundos - Modo Curto)
    if (PIR1.TMR1IF) {
        TMR1H = 0x0B;
        TMR1L = 0xDC;
        if (rodando && !modo_longo) {
            tmr1_cont++;
            if (tmr1_cont >= 4) { // 4 x 250ms = 1 segundo
                tmr1_cont = 0;
                if (tempo_restante > 0) tempo_restante--;
                if (tempo_restante == 0) rodando = 0; // Desliga tudo ao acabar
            }
        }
        PIR1.TMR1IF = 0;
    }
}

void main() {
    unsigned int adc_raw;
    unsigned int temp_x10;
    char str_tempo[17] = "Tempo:          ";
    char str_temp[17]  = " Temp:          ";

    // Atraso de proteção: evita cliques falsos ao dar "Play" no simulador
    Delay_ms(300);

    ADC_Init();
    // Configuração de Vref Externa (1V no A3) 
    ADCON1 = 0x3B;

    // Configuração de Direção dos Pinos
    TRISB.B0 = 1;
    TRISB.B1 = 1;
    TRISA.B0 = 1;
    TRISC.B0 = 0;
    PORTC.B0 = 0; // Garante que a resistência inicie desligada

    // Inicialização do Display LCD
    Lcd_Init();
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Lcd_Cmd(_LCD_CLEAR);

    // Configuração de Interrupções por Borda de Subida
    INTCON2.INTEDG0 = 1;
    INTCON2.INTEDG1 = 1;

    // Limpeza de flags de inicialização
    INTCON.INT0IF = 0;
    INTCON3.INT1IF = 0;

    // Habilita as interrupções externas dos botões
    INTCON.INT0IE = 1;
    INTCON3.INT1IE = 1;

    // Configuração do Timer0 (16-bits, Prescaler 1:256)
    T0CON = 0b10000111;
    TMR0H = 0xE1; TMR0L = 0x7C;
    INTCON.TMR0IF = 0;
    INTCON.TMR0IE = 1;

    // Configuração do Timer1 (16-bits, Prescaler 1:8)
    T1CON = 0b10110001;
    TMR1H = 0x0B; TMR1L = 0xDC;
    PIR1.TMR1IF = 0;
    PIE1.TMR1IE = 1;

    INTCON.PEIE = 1;
    INTCON.GIE = 1;

    while(1) {
        // Lógica para resetar as flags de bouncing quando soltar os botões
        if (PORTB.B0 == 0) bounce_flag0 = 0;
        if (PORTB.B1 == 0) bounce_flag1 = 0;

        if (rodando) {
            // Leitura de Temperatura
            adc_raw = ADC_Get_Sample(0);
            temp_x10 = ((unsigned long)adc_raw * 1000) / 1023;

            // Histerese da Resistência (LED em RC0) 
            if (temp_x10 < 600) PORTC.B0 = 1;
            else if (temp_x10 > 800) PORTC.B0 = 0;

            // Formatação do Display para "XX.X °C"
            str_temp[6] = (temp_x10 / 1000) > 0 ? (temp_x10 / 1000) + '0' : ' ';
            str_temp[7] = ((temp_x10 / 100) % 10) + '0';
            str_temp[8] = ((temp_x10 / 10) % 10) + '0';
            str_temp[9] = '.';
            str_temp[10] = (temp_x10 % 10) + '0';
            str_temp[11] = 223; // Caractere de "°" no LCD
            str_temp[12] = 'C';
            str_temp[13] = '\0';

        } else {
            // Desliga a resistência e oculta temperatura se estiver parado
            PORTC.B0 = 0;

            str_temp[6] = '-'; str_temp[7] = '-'; str_temp[8] = '-';
            str_temp[9] = ' '; str_temp[10] = ' '; str_temp[11] = ' ';
            str_temp[12] = ' '; str_temp[13] = '\0';
        }

        // Formatação do Tempo Restante
        str_tempo[7] = (tempo_restante / 10) + '0';
        str_tempo[8] = (tempo_restante % 10) + '0';
        str_tempo[9] = 's';
        str_tempo[10] = ' ';

        if (modo_longo) {
            str_tempo[11] = 'L'; str_tempo[12] = 'o'; str_tempo[13] = 'n'; str_tempo[14] = 'g'; str_tempo[15] = 'o';
        } else {
            str_tempo[11] = 'C'; str_tempo[12] = 'u'; str_tempo[13] = 'r'; str_tempo[14] = 't'; str_tempo[15] = 'o';
        }

        Lcd_Out(1, 1, str_tempo);
        Lcd_Out(2, 1, str_temp);

        Delay_ms(100);
    }
}
