#define TOOTH_COUNT 116
#define TOOTH_COUNT_OUT 24
#define TOOTH_MISSING_OUT 1

int toothCounter = 0;
int rpmCounter = 0;
int triggeringTeeth[TOOTH_COUNT_OUT];
int triggeringOffset[TOOTH_COUNT_OUT];
int currentTriggeringIndex = 0;
bool trigger;

bool firstRefToothDetected = false;
bool errorDetected = false;
long lastTrigger = 0;
long lastTeethTimestamp = 0;
long previousTimeBetweenTeeth = 0;

void smallToothDetected() {
  if (firstRefToothDetected) {
    toothCounter++;
    if (toothCounter == triggeringTeeth[currentTriggeringIndex]){
      trigger = true;
      lastTrigger = micros();
    } else {
      long aux = micros();
      if(aux > previousTimeBetweenTeeth){
        previousTimeBetweenTeeth = aux - lastTeethTimestamp;
        lastTeethTimestamp = aux;
      }
      lastTeethTimestamp = aux;
    }
  }
}

void referenceToothDetected() {
  if (firstRefToothDetected) {
    if (TOOTH_COUNT != toothCounter) {
      errorDetected = true;
    }
    rpmCounter++;
    toothCounter = 0;
    currentTriggeringIndex = TOOTH_MISSING_OUT;
  } else {
    firstRefToothDetected = true;
  }
}

void setup() {
  // put your setup code here, to run once:
  attachInterrupt(digitalPinToInterrupt(2), smallToothDetected, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), referenceToothDetected, RISING);
  DDRB = B11000000;
  DDRA = B11111111;
  PORTB = B10000000;
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(23, OUTPUT);
  Serial.begin(9600);

  for(int i = 0; i < TOOTH_COUNT_OUT; i++){
    triggeringTeeth[i] = (TOOTH_COUNT * i) / TOOTH_COUNT_OUT;
    triggeringOffset[i] = 100 * ((((float) TOOTH_COUNT * (float) i) / (float) TOOTH_COUNT_OUT) - (float) triggeringTeeth[i]);

    Serial.print(i);
    Serial.print("-");
    Serial.print(triggeringTeeth[i]);
    Serial.print("-");
    Serial.println(triggeringOffset[i]);
  }
  
  lastTrigger = micros();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(errorDetected){
    Serial.println("error detected");
    errorDetected = false;
    PORTB &= B01111111;
  }
//  Serial.println(rpmCounter * 60);
//  rpmCounter = 0;
  if(trigger){
    long currentTime = micros();
    long offsetFromLastTrigger = (previousTimeBetweenTeeth * triggeringOffset[currentTriggeringIndex]) / 100;
    if(currentTime > lastTrigger + offsetFromLastTrigger){
      PORTB = B11111111;
      currentTriggeringIndex++;
      trigger = false;
    }
    delayMicroseconds(previousTimeBetweenTeeth);
    PORTB = B00000000;
  }
}
