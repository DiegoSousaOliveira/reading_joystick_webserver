# Leitura da Posição de um Joystick via Servidor Web com RP2040

Este projeto demonstra como configurar e programar o microcontrolador RP2040 para ler a posição de um joystick analógico e disponibilizar essas informações através de um servidor web.

## 📌 Objetivo

Capturar as posições dos eixos X e Y de um joystick analógico e exibi-las em tempo real em uma página web hospedada no próprio RP2040.

## 🧰 Requisitos

### 📦 Software

- Raspberry Pi Pico SDK
- CMake
- Compilador arm-none-eabi
- Make

### 🧱 Hardware

- Raspberry Pi Pico (RP2040)
- Módulo de Joystick Analógico
- Resistores de pull-down de 10kΩ (opcional, se necessário)
- Cabos jumpers e protoboard

## ⚙️ Esquema de Ligação

| Componente            | Pino Pico |
|-----------------------|-----------|
| Eixo X do Joystick    | GP26 (ADC0) |
| Eixo Y do Joystick    | GP27 (ADC1) |

**Observações:**

- Os eixos X e Y do joystick fornecem saídas analógicas que variam tipicamente de 0V a 3.3V, correspondendo a valores digitais de 0 a 4095 (considerando uma leitura ADC de 12 bits).
- O botão do joystick é um interruptor que pode ser lido como uma entrada digital.

### 🔧 Compilando o Projeto
  Crie a pasta de build:
    ```
    mkdir build
    cd build
    ```
  Execute o CMake:
    ```
    cmake ..
    ```
  Compile:
    ```
    make
    ```
### Grave o .uf2 no RP2040:

  - Conecte o Pico ao PC segurando o botão BOOTSEL.
  - Solte o botão após conectar.
  - Copie o .uf2 gerado para o dispositivo montado.

### 💻 Código Principal (Resumo)

O código principal do projeto está no arquivo `reading_position_joystick.c`. Este programa configura o RP2040 para:

- Inicializar os pinos ADC para ler os eixos X e Y do joystick
- Configurar o pino digital para ler o estado do botão do joystick.
- Configurar uma interface de rede utilizando a biblioteca lwIP.
- Configurar um servidor HTTP que responde com as posições atuais dos eixos do joystick e o estado do botão.
- Ler continuamente os valores dos eixos e do botão, atualizando as informações no servidor web.

## 🌐 Testando o Servidor Web

Após gravar o firmware no RP2040:

- Conecte-se à mesma rede Wi-Fi que o RP2040 está configurado.
- Obtenha o endereço IP do RP2040 (pode ser exibido via `printf` no código).
- Acesse o endereço IP através de um navegador web.
- O servidor web exibirá as posições atuais dos eixos X e Y do joystick, bem como o estado do botão.

## ✅ Resultados Obtidos

Ao finalizar a implementação do projeto, obtivemos os seguintes resultados:

- O joystick analógico conectado aos pinos GP26 e GP27 pôde ser lido corretamente pelo RP2040, fornecendo valores correspondentes às posições dos eixos X e Y.
- O botão do joystick conectado ao pino GP22 foi lido corretamente, indicando quando estava pressionado ou não.
- Um servidor web foi iniciado diretamente no microcontrolador, permitindo acessar via navegador as posições dos eixos e o estado do botão em tempo real.
- As leituras foram atualizadas continuamente a cada 100ms, garantindo baixa latência e boa responsividade na visualização.
- O código funcionou de forma eficiente mesmo com recursos limitados do RP2040, graças ao uso de C bare metal e à leveza da lwIP.

## 🎯 Impacto de Cada Configuração

### 1. `adc_init() + adc_gpio_init()`

- **Função:** Inicializa o módulo ADC e configura os pinos para leitura analógica.
- **Impacto:** Permite que os pinos designados leiam valores analógicos dos eixos do joystick.

### 2. `adc_gpio_init(JOYSTICK_X_PIN)` e `adc_gpio_init(JOYSTICK_Y_PIN)`
- **Função**: Configura os pinos GP26 (ADC0) e GP27 (ADC1) para uso com o ADC.
- **Impacto**: Habilita a leitura correta dos sinais analógicos dos eixos X e Y. Sem isso, os pinos estariam em modo digital e retornariam leituras inválidas.

### 3. `adc_select_input(0)` e `adc_select_input(1)`
- **Função**: Define qual canal ADC será lido (canal 0 = GP26, canal 1 = GP27).
-  **Impacto**: Permite alternar entre as leituras dos dois eixos do joystick. A seleção correta é essencial para não confundir os valores de X e Y.

### 4. `adc_read()`
- **Função**: Lê o valor analógico atual do canal ADC selecionado (entre 0 e 4095).
- **Impacto**: Fornece a posição do joystick com precisão. Esses valores são usados para exibir as coordenadas no servidor web.

### 5. `gpio_init(JOYSTICK_BUTTON_PIN)` + `gpio_set_dir(..., GPIO_IN)`
- **Função**: Inicializa o pino como entrada para leitura digital (botão).
- **Impacto**: Permite verificar se o botão do joystick foi pressionado. É fundamental para dar mais controle ao usuário sobre a interface.

### 7. `gpio_get(JOYSTICK_BUTTON_PIN)`
- **Função**: Lê o estado do botão (pressionado ou não).
- **Impacto**: Fornece um valor binário (0 ou 1) que indica a interação do usuário com o botão. Pode ser usado para acionar comandos ou mudar estados na página web.
