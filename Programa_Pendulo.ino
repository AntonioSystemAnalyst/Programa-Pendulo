// Bibliotecas
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Wire.h>

// Definições
#define endereco 0x27     // Endereço 
#define colunas  16       // Numero de colunas do Display LCD
#define linhas   2        // Numero de linhas do Display LCD      
#define echo     A1       // Entrada para o pino de echo do sensor ultrasonico
#define trig     A3       // Saída para o pino de trigger do sensor ultrasonico
#define leda     13       // Led Vermelho
#define ledb     12       // Led Verde
#define ledc     11       // Led Branco
#define buzzer   10       // Buzzer
#define infra    A0        // Módulo conectado no pino digial 2
#define btn      2        // Botão
#define qtd_M    10       // Quantidade de medidas para fazer a média

// Objetos
LiquidCrystal_I2C lcd(endereco, colunas, linhas);  // Objeto para controle do Display LCD
Servo servo;                                       // Objeto para controle do Servo

// Variaveis unsigned
unsigned long timer = 0;                   // Para receber o valor do tempo
unsigned long tempo_auxiliar = 0;          // Para reservar o tempo no meio da trajetória
unsigned long tempo_um = 0;                // Tempo de ida, do meio para esquerda, e para o meio de novo
unsigned long tempo_dois = 0;              // Tempo de ida, do meio para direita, e para o meio de novo
unsigned long tempo_um_total = 0;          // Tempo_um/2
unsigned long tempo_dois_total = 0;        // Tempo_dois/2
unsigned long tempo_Periodo_Total = 0;     // Tempo total de ciclo completo
unsigned long Periodo = 0;                 // Periodo calculado
unsigned long tempo_vetor[25];             // Vetor de periodos para se obter a média

// Variaveis
int controle1 = 0;        // Controle na programação, de botão ligado e desligado
int controle2 = 0;        // Controle na programação, de botão ligado e desligado
int controle_sensor = 0; // Para evitar erros que o sensor pode cometer em determinadas situações
int led_controle = 0;    // Determina o comportamento do led
int contador1 = 0;       // Determina a quantidade de passos do pêndulo para estabilizar, para então iniciar as medidas
int contador2 = 0;       // Determina a quantidade total de medidas feitas
int status_pendulo = 0;  // Define qual a posição em que o pendulo esta, na lógica
float dist_cm = 0;       // Recebe a distância medida no sensor ultrasonico

// Funções
void  free_all ();       // Limpa os dados para nova medição
void  servo_pendulo ();  // Retorna a posição de inicio
void  sensores();        // Aciona ambos os sensores
void  infravermelho();   // Função para nedida apenas com o sensor optico
void  ultrasonico();     // Função para medidas apenas com o ultrasonic
void  trigPulso();       // Gera o pulso de trigger para o sensor HC-SR04
float medirDistancia();  // Retorna a distância em centímetros com o ultrasonic

void setup() {
  Serial.begin(9600);                // Inicia o serial em 9600
  pinMode(leda, OUTPUT);             // Porta para controle do led1
  pinMode(ledb, OUTPUT);             // Porta para controle do led2
  pinMode(ledc, OUTPUT);             // Porta para controle do led3
  pinMode(buzzer, OUTPUT);           // Porta para controle do buzzer
  pinMode(btn, INPUT_PULLUP);        // Porta para controle do botão de acionamento
  pinMode(infra, INPUT);             //DEFINE O PINO COMO ENTRADA
  pinMode(trig, OUTPUT);             // Saída para o pulso de trigger
  pinMode(echo, INPUT);              // Entrada para o pulso de echo
  servo.attach(9);                   // Define a porta digital 9 para controle do servo
  lcd.init();                        // Inicia a comunicação com display
  lcd.backlight();                   // Liga a iluminação do display
  lcd.clear();                       // Realiza a limpeza do display
  lcd.print("Pendulo Simples!");     // Exibe o texto na tela do display
  lcd.setCursor(0, 1);               // Altera a posição para print no display
  lcd.print("  HackArduino");        // Exibe o texto na tela do display
}

void loop() {
  if (digitalRead(btn) == LOW)  // Se o botão estiver desligado
  {
    digitalWrite(ledb, HIGH);   // Mantenha o Led Verde Ligado
    if (controle1 == 0)
    {
      free_all ();                // Mantenha as variaveis nas consições iniciais
      controle1 = 1;
    }
  }
  else
  {
    digitalWrite(12, LOW);     // Mantenha o Led Verde Apagado
    if (controle2 == 0)
    {
      servo_pendulo();           // Libera o pêndulo
      //sensores();              // Liga ambos os sensores
      ultrasonico ();          // Liga apenas o sensor ultrasonic (visualizar plotter da IDE)
      //infravermelho ();                 // Liga apenas o sensor optico (visualizar plotter da IDE)
      controle2 = 1;
    }
  }
}
void sensores () // Função que controla os sensores
{
  int w     = 0;  // Controle do While
  int np    = 0;  // Contador para minimizar falhas

  do {
    if (analogRead(infra) > 800 && controle_sensor == 0) // Se o sensor estiver em 0 e a variavel de controle estiver em 0
    {
      contador1 =  contador1 + 1; // Incrementa o contador
      controle_sensor = 1;        // Altera a variavel de controle
      if (contador1 > 20)         // Para que se possa estabilizar
      {
        contador1 = 100;
        if (status_pendulo == 0)  // Se status_pendulo = 0, indica que é a primeira vez que o pendulo foi lançado
        {
          status_pendulo = 2;        // Altera-se o status para o valor 2, indicando que está indo do para a esquerda
          tempo_auxiliar = micros(); // Registra-se o tempo em microssegundos
        }
        else
        {
          if (status_pendulo == 2) // Se o valor do status for 2, é por que o pendulo acabou de retornar da esquerda para o centro
          {
            tempo_um = micros();   // Registra-se o tempo
            tempo_um_total = ((tempo_um - tempo_auxiliar) / 2); // Se faz a divisão por 3, já que ele foi e voltou
            status_pendulo = 3;  // Altera-se o status para o valor 3, indicando que está indo do para a direita
            /*Serial.print("-----------------");
              Serial.print("\n");
              Serial.print("1 parte");
              Serial.print("\n");
              Serial.print(tempo_auxiliar);
              Serial.print("\n");
              Serial.print(tempo_um);
              Serial.print("\n");
              Serial.print(tempo_um_total);
              Serial.print("\n"); */
            tempo_auxiliar = micros();  // registra-se o tempo
          }
          else
          {
            if (status_pendulo == 3)
            {
              tempo_dois = micros(); // Se o valor do status for 3, é por que o pendulo acabou de retornar da direita para o centro
              tempo_dois_total = ((tempo_dois - tempo_auxiliar) / 2); // Se faz a divisão por 3, já que ele foi e voltou
              if (tempo_dois_total > 10 && tempo_um_total > 10)  // Se ambos os valores estiverem corretos, com valores acima da faixa de erro
              {
                tempo_vetor[contador2] = ((tempo_um_total +  tempo_dois_total) * 2); // A soma indica o movimento do pendulo indo e voltando 1 vez, o periodo porem é doblo
                contador2 = contador2 + 1; // incrementa o contador, indicando a quantidade de medidas realizadas
                /*Serial.print(contador2);
                  Serial.print("\n");
                  Serial.print("2 parte: ");
                  Serial.print("\n");
                  Serial.print(tempo_auxiliar);
                  Serial.print("\n");
                  Serial.print(tempo_dois);
                  Serial.print("\n");
                  Serial.print(tempo_dois_total);
                  Serial.print("\n");
                  Serial.print(tempo_vetor[contador2]);
                  Serial.print("\n");*/
              }
              status_pendulo = 2;        // Altera-se o status para o valor 2, indicando que está indo do para a esquerda
              tempo_auxiliar = micros(); // registra-se o tempo
            }
          }
        }
      }

      if (led_controle == 0) // Para esquerda led branco acende e led vermelho apaga
      {
        digitalWrite(leda, HIGH);
        digitalWrite(ledc, LOW);
        tone(buzzer, 901); // Toca-se uma nota no buzzer
        led_controle = 1;
      }
      else
      {
        if (led_controle == 1) // Para esquerda led branco acende e led vermelho apaga
        {
          digitalWrite(ledc, HIGH);
          digitalWrite(leda, LOW);
          tone(buzzer, 201); // Toca-se uma nota no buzzer
          led_controle = 0;
        }
      }
      //delayMicroseconds(500);                    //Por 500µs ...
    }
    else
    {
      if ( analogRead(infra) <= 800 &&  controle_sensor == 1)
      {
         noTone(buzzer); // Desliga o buzzer
         controle_sensor = 0;
      }
    }
    if (digitalRead(btn) == LOW) // Se o botão for pressionado
    {
      noTone(buzzer);
      w = 1;
    }
    if (contador2 == qtd_M)
    {
      noTone(buzzer);
      w = 1;
    }

    //dist_cm  = measureDistance();
    // Serial.print(" ");
    // Serial.print("Distancia: ");
    // Serial.print(" ");
    // Serial.print(dist_cm);
    // Serial.println();
    //delay (50);

  } while (w == 0);
  Serial.print("-----------------");
  Serial.print("\n");
  for (int i = 0; i < qtd_M; i++)  // Percorre todas as medidas feitas
  {
    tempo_Periodo_Total = tempo_Periodo_Total + tempo_vetor[i]; // Soma total dos valores de periodo obtidos
    Serial.print(tempo_vetor[i]);                               // printa os valores no serial
    Serial.print("\n");
  }
  tempo_Periodo_Total = tempo_Periodo_Total / qtd_M;                // Periodo total medida na altura em que esta o sensor

  lcd.clear();                                                      // limpa o display
  lcd.print("Periodo[micro s]");
  lcd.setCursor(0, 1);
  lcd.print(tempo_Periodo_Total);

}

void free_all ()  // Mantem o servo em 140, e as condições iniciais das variaveis
{
  lcd.clear();
  lcd.print("Pendulo Simples!");
  lcd.setCursor(0, 1);
  lcd.print("  HackArduino");
  servo.write(140);
  noTone(buzzer);
  digitalWrite(leda, LOW);
  digitalWrite(ledc, LOW);
  contador1 = 0;
  contador2 = 0;
  timer = 0;
  tempo_um = 0;
  tempo_dois = 0;
  tempo_um_total = 0;
  tempo_dois_total = 0;
  tempo_Periodo_Total = 0;
  controle_sensor = 0;
  tempo_auxiliar = 0;
  led_controle = 0;
  status_pendulo = 0;
  dist_cm = 0;
}
void servo_pendulo() // Função que movimenta o servo liberando o pêndulo
{
  servo.write(170);  // posiciona o servo em 170
}

void ultrasonico ()
{
  int w = 0;  // Controle While
  do {
    dist_cm  = medirDistancia();  // mede a distancia
    timer = micros();             // mede o tempo
    if (dist_cm <= 12.5 && dist_cm >= 4.5) // Condições para evitar falhar.. 12,5 cm para distância maior e 4,5 cm para a menor
    {
      //Serial.print ("Tempo: ");   // Printa os valores ou os exibe no plotter
      //Serial.print (timer);
      //Serial.print (" ");
      //Serial.print ("Distância: ");
      Serial.print(dist_cm);
      Serial.println();
    }
  } while (w == 0);
}

void infravermelho ()    // visualiza funcionamento do sensor optico
{
  int w   = 0;    // Controle While
  int np  = 0;    // Condição para próxima leitura do pêndulo
  int ax  = 0;    // Determina o numero de picos
  do {

    if (analogRead(infra) < 800)
    { //SE A LEITURA DO PINO FOR MENOR QUE 800 BITS, FAZ

      Serial.println(100); //IMPRIME O TEXTO NA SERIAL
      np = 1;
      
    } else { //SENÃO, FAZ
      
      if (np == 1)
      {
         Serial.println(500);
         np = 0;
      }
     
    
    }

    delayMicroseconds(10);                  //Por 10µs ...
  } while (w == 0);
}

float medirDistancia()                      //Função que retorna a distância em centímetros
{
  float pulso;                              //Armazena o valor de tempo em µs que o pino echo fica em nível alto
  trigPulso();                              //Envia pulso de 10µs para o pino de trigger do sensor
  pulso = pulseIn(echo, HIGH);              //Mede o tempo em que echo fica em nível alto e armazena na variável pulse
  return (pulso / 58.82);                   //Calcula distância em centímetros e retorna o valor
}

void trigPulso()                            //Função para gerar o pulso de trigger para o sensor HC-SR04
{
  digitalWrite(trig, HIGH);                 //Saída de trigger em nível alto
  delayMicroseconds(10);                    //Por 10µs ...
  digitalWrite(trig, LOW);                  //Saída de trigger volta a nível baixo
}
