# Projeto 2: Aferidor de Temperatura de Forno Industrial
**Disciplina:** SEL0433 - Aplicação de Microprocessadores  
**Autores:** Pedro da Silva Panini nºusp:15483543
             Lucas Gomes Pacheco nºusp:15697253
             Carlos Eduardo Cintra Siqueira Rodrigues de Mattos nºusp:15445279

## Introdução
Este projeto consiste no desenvolvimento de um sistema embarcado para aferição de temperatura e tempo de um forno industrial, utilizando o microcontrolador PIC18F4550. O objetivo é monitorar a temperatura interna simulada por um sensor (potenciômetro) e exibir os dados em um display LCD durante um período predeterminado pelo usuário, implementando timers, interrupções e conversão analógico-digital (ADC).

## Funcionalidades Implementadas
- **Seleção de Tempo por Interrupção:** - Botão 1 (RB0): Inicia imediatamente uma contagem regressiva longa (60 segundos).
  - Botão 2 (RB1): Inicia imediatamente uma contagem regressiva curta (10 segundos).
  - Ambos os botões funcionam como *STOP* caso pressionados durante suas respectivas contagens.
- **Leitura de Temperatura (ADC):** Conversão de sinal analógico com Vref externa (1V) nos pinos A2/A3, garantindo maior precisão para a escala do sensor LM35 (0 a 100 °C).
- **Lógica de Histerese (Controle da Resistência):** Um LED no pino RC0 simula a resistência do forno, ligando para temperaturas abaixo de 60 °C e desligando ao ultrapassar 80 °C.
- **Tratamento de Bouncing:** Implementado via software (flags não-bloqueantes) dentro da rotina de interrupção.
- **Otimização de Memória:** O cálculo e a formatação da temperatura para o display (`XX.X °C`) foram feitos utilizando matemática inteira, sem o uso de variáveis do tipo `float`.

## Esquemático e Hardware
As conexões foram baseadas na estrutura do Kit EasyPIC v7 e validadas no SimulIDE:
- **Display LCD:** Barramento de dados 4-bits e controle centralizados no PORTD (RD0 a RD5).
- **Botões:** Pinos RB0 (INT0) e RB1 (INT1) com resistores de pull-down.
- **LED (Forno):** Pino RC0.
- **Potenciômetro:** Pino RA0 (AN0).

## Resultados e Discussão
O sistema comportou-se conforme os requisitos especificados. Os **Timer0** e **Timer1** foram configurados com sucesso para gerar bases de tempo de 1 segundo e 250 milissegundos, respectivamente, garantindo uma contagem regressiva precisa no display. 

Um desafio superado durante o desenvolvimento foi a configuração da Tensão de Referência Externa para o ADC. A biblioteca padrão do compilador MikroC sobrescrevia os bits de referência ao chamar `ADC_Init()`. A solução adotada foi forçar a injeção do valor `0x3B` no registrador `ADCON1` logo após a inicialização da biblioteca, alocando corretamente Vref+ no pino RA3 e Vref- no pino RA2.

## Imagens do Projeto

### 1. Compilação no MikroC
Abaixo está o log de compilação confirmando o sucesso da build. Nota-se o baixo consumo de memória (apenas 3% de RAM e 8% de ROM), evidenciando o cumprimento do requisito de não utilização de variáveis do tipo `float` para a formatação da temperatura.
<img width="1470" height="270" alt="image" src="https://github.com/user-attachments/assets/4f616b79-ef4c-4a6e-8d7b-94fe5b24d2e6" />

### 2. Circuito de Simulação (SimulIDE)
Captura do circuito em pleno funcionamento no modo de aferição "Longo" (60s). O display exibe corretamente a contagem regressiva e a medição em tempo real da temperatura no formato "XX.X °C".
<img width="951" height="929" alt="image" src="https://github.com/user-attachments/assets/ae9bc46f-9480-4eba-9fa5-4b171e3302dc" />
