# Controle Customizado para Kingdom Come: Deliverence

## jogo
**Kingdom Come: Deliverence** - Um jogo primeira pessoa de RPG medieval realista baseado em simular duelos, tramas e convivencias da epoca.

## Ideia do Controle
O controle sera uma espada de brinquedo com sensores que vai simular o combate dentro do jogo, fazendo movimentos de investidas, estocadas e defesas baseados nos movimentos da espada, tambem havera leds mostrando a conexao correta da espada com o computador e imas para gerar vibracao ao acertar um golpe.

## Inputs e OutPuts
### Entradas(inputs)
- **Joystick analogico**: Para movimentacao do personagem
- **IMU**: detectar movimentos da espada
- **botao liga/desliga**: ligar controle e iniciar deteccao
### Entradas Digitais
- **botao de abrir menu(pause)**: abre o menu de settings principal do jogo
- **botao de defender**: apersonagem faz uma defesa rapida
- **botao de atacar**: botao para realizar o ataque
- **botao de esquivar**: realizar desvio rapido

### **Saidas(outputs)**
-**LED indicando que o controle esta conectado**
-**buzzer para vibracao e som(golpe da espada e acerto)**

### Protocolo Utilizado
-**UART** comunicao controle e computador
-**GPIO Interrupts** botoes e entradas digitais(interrupcoes)
-**IMU** para detectar os movimentos do controle

## Diagrama de Blocos Firmware
### *Estrutura Geral*
---
+---------------------+
|   Task Captura     |
| Joystick, IMU, Btns|
+---------------------+
        ↓
+---------------------+
|  Filas de Eventos  |
+---------------------+
        ↓
+---------------------+
| Task Processamento |
|    de Eventos      |
+---------------------+
        ↓
+---------------------+
| Task Comunicação   |
|       com PC       |
+---------------------+
        ↓
+---------------------+
|  Task Feedback     |
| (LED/Buzzer)       |
+---------------------+

---

### **Principais Componester do RTOS**
-**Tasks**
Tasks:

    Task de leitura de entradas: le os valores do joystick, IMU e botões, 

    Task de processamento de eventos: recebe eventos da fila de entradas, interpreta os comandos e envia para a fila do jogo.

    Task de comunicação com o jogo: envia os comandos processados via UART/Bluetooth/USB para o computador.

    Task de feedback sonoro: controla o buzzer.

    Task de controle do LED indicador: atualiza o LED.

Filas:

    Fila de eventos de entrada: armazena eventos vindos do joystick, IMU e botoes para processamento.

    Fila de comandos para o jogo: contem os comandos que serao enviados para o computador.

    Fila de feedback sonoro: mantem eventos que ativam sons no buzzer.

Semáforos:

    Semáforo de estado de conexão: Garante que o LED e outros componentes respondam corretamente ao estado da conexao com o jogo.

Interrupts:

    Callbacks para botões: os botoes (ataque, defesa, esquiva, menu) geram interrupoces para reduzir latência na resposta.

    Interrupção do botão liga/desliga: para o funcionamento do controle

## Imagens do Controle
### ideia inicial(espada com sensores e botoes no handle)
--
![Diagrama do Projeto](espada.jpg)
--

