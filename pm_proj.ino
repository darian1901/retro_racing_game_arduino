const int latchPinCol = 10;
const int clockPinCol = 11;
const int dataPinCol = 12;

const int latchPinLine = 7;
const int clockPinLine = 8;
const int dataPinLine = 9;

const int buttonPinLeft = 2;
const int buttonPinRight = 3;

const int playerPos = 192;

volatile int playerLine = 0;
const int lineNum = 4;

int timer1_cmp_val;

const int hz_const = 2;

volatile int enemyCurrentCol[3];
volatile int randCol[3];

volatile int gameReady = 1;

ISR(TIMER1_COMPA_vect)
{
  TCNT1 = timer1_cmp_val;
  
  for (int i = 0; i < 3; ++i) {
    if(enemyCurrentCol[i] == 6) {
        enemyCurrentCol[i] = 0;
        switch(i) {
          case 0:
            randCol[i] = random(0, 2);
            break;
          case 1:
            if (randCol[i - 1] == 1)
              randCol[i] = 2;
            else
              randCol[i] = random(1, 3);
            break;
          case 2:
            if (randCol[i - 1] == 2)
              randCol[i] = 3;
            else
              randCol[i] = random(2, 4);
            break;
        }
      } else {
        if (enemyCurrentCol[i] == 0) {
          int coinFlip = random(0, 2);
            if (coinFlip)
              ++enemyCurrentCol[i];
        } else {
          ++enemyCurrentCol[i];
        }
      }
  }
}

void left()
{
    int readd = digitalRead(buttonPinLeft);

    if (readd == LOW) {
      if (playerLine > 0) {
        playerLine--;
      }
    }
}

void right()
{
    int readd = digitalRead(buttonPinRight);

    if (readd == LOW) {
      if (playerLine < lineNum - 1) {
        ++playerLine;
      }
    }
}

void setup()
{
  noInterrupts();

  randCol[0] = 0;
  randCol[1] = 1;
  randCol[2] = 2;

  TCCR1A = 0;
  TCCR1B = 0;

  timer1_cmp_val = 16000000/(256 * hz_const) - 1;

  TCNT1 = timer1_cmp_val;

  TCCR1B |= (1 << CS12);

  TIMSK1 |= (1 << OCIE1A);
  
  pinMode(buttonPinLeft, INPUT_PULLUP);
  pinMode(buttonPinRight, INPUT_PULLUP);
  
  pinMode(latchPinCol, OUTPUT);
  pinMode(clockPinCol, OUTPUT);
  pinMode(dataPinCol, OUTPUT);

  pinMode(latchPinLine, OUTPUT);
  pinMode(clockPinLine, OUTPUT);
  pinMode(dataPinLine, OUTPUT);

  digitalWrite(latchPinLine, LOW);
  shiftOut(dataPinLine, clockPinLine, MSBFIRST, 1L << playerLine);
  digitalWrite(latchPinLine, HIGH);

  attachInterrupt(digitalPinToInterrupt(buttonPinLeft), left, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPinRight), right, FALLING);
  
  delay(1000);

  interrupts();
}

void resetGame()
{
  noInterrupts();
      
  gameReady = 0;
  TCCR1A = 0;
  TCCR1B = 0;
          
  timer1_cmp_val = 16000000/(256 * hz_const) - 1;
          
  TCNT1 = timer1_cmp_val;
          
  TCCR1B |= (1 << CS12);
          
  TIMSK1 |= (1 << OCIE1A);

  enemyCurrentCol[0] = 0;
  enemyCurrentCol[1] = 1;
  enemyCurrentCol[2] = 2;

  randCol[0] = 0;
  randCol[1] = 1;
  randCol[2] = 2;
  
  playerLine = lineNum - 1;

  digitalWrite(latchPinLine, LOW);
  shiftOut(dataPinLine, clockPinLine, MSBFIRST, 15);
  digitalWrite(latchPinLine, HIGH);

  digitalWrite(latchPinCol, LOW);
  shiftOut(dataPinCol, clockPinCol, MSBFIRST, 0);
  digitalWrite(latchPinCol, HIGH);

  delay(900000);
  gameReady = 1;

  interrupts();
}

void loop()
{
  while (gameReady == 1) {
    for (int i = lineNum - 2; i >= 0; --i) {
      digitalWrite(latchPinLine, LOW);
      shiftOut(dataPinLine, clockPinLine, MSBFIRST, 1L << randCol[i]);
      digitalWrite(latchPinLine, HIGH);

      digitalWrite(latchPinCol, LOW);
      shiftOut(dataPinCol, clockPinCol, MSBFIRST, ~((1 << enemyCurrentCol[i]) + (1 << enemyCurrentCol[i] + 1)));
      digitalWrite(latchPinCol, HIGH);
      delay(7);
    }

    delay(4);

    digitalWrite(latchPinLine, LOW);
    shiftOut(dataPinLine, clockPinLine, MSBFIRST, 1L << playerLine);
    digitalWrite(latchPinLine, HIGH);
          
    digitalWrite(latchPinCol, LOW);
    shiftOut(dataPinCol, clockPinCol, MSBFIRST, ~playerPos);
    digitalWrite(latchPinCol, HIGH);

    for (int i = 0; i < lineNum - 1; ++i)
      if (randCol[i] == playerLine && enemyCurrentCol[i] >= 5) {
        noInterrupts();
        delay(900000);
        resetGame();
      }
        
    delay(4);
  }
}
