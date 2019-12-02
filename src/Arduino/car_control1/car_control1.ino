int i =0;
   
const int EnableL = 5;
const int HighL = 6;       // LEFT SIDE MOTOR
const int LowL =7;

const int EnableR = 10;
const int HighR = 8;       //RIGHT SIDE MOTOR
const int LowR =9;

const int D0 = 0;       //Raspberry pin 21    LSB
const int D1 = 1;       //Raspberry pin 22
const int D2 = 2;       //Raspberry pin 23
const int D3 = 3;       //Raspberry pin 24    MSB

const int LeftSignal = 12;
const int RightSignal = 13;

int a,b,c,d,data;
int object_time = -3000;




void setup() 
{
  pinMode(EnableL, OUTPUT);
  pinMode(HighL, OUTPUT);
  pinMode(LowL, OUTPUT);
  
  
  pinMode(EnableR, OUTPUT);
  pinMode(HighR, OUTPUT);
  pinMode(LowR, OUTPUT);

  pinMode(LeftSignal, OUTPUT);
  pinMode(RightSignal, OUTPUT);
  
  pinMode(D0, INPUT_PULLUP);
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
}

void Data() // input data from Ras Pi
{
   a = digitalRead(D0);
   b = digitalRead(D1);
   c = digitalRead(D2);
   d = digitalRead(D3);

   data = 8*d+4*c+2*b+a;
}

void Forward()
{
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL,255);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR,255);
}


void Backward()
{
  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  analogWrite(EnableL,255);

  digitalWrite(HighR, HIGH);
  digitalWrite(LowR, LOW);
  analogWrite(EnableR,255);
}

void Stop()
{
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL,0);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR,0); 
}

void Left1()
{
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL,160);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR,255);
}

void Left2()
{
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL,90);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR,255); 
}


void Left3()
{
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL,50);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR,255);  
}

void Right1()
{
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL,255);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR,160);  //200
}

void Right2()
{
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL,255);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR,90);   //160
}

void Right3()
{
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL,255);

  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableR,50);   //100
}

void UTurn()
{
  analogWrite(EnableL, 0);      // stop
  analogWrite(EnableR, 0);
  BrakeLight();
  delay(400);

  analogWrite(EnableL, 250);
  analogWrite(EnableR, 250);    // forward
  OffLight();
  delay(1000);

  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);      // stop
  BrakeLight();
  delay(400);

  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  digitalWrite(HighR, LOW);     // left
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  LeftLight();
  delay(700);

  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);   // stop
  BrakeLight();
  delay(400);

  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, LOW);    // forward
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  OffLight();
  delay(900);

  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);     // stop
  BrakeLight();
  delay(400);

  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  digitalWrite(HighR, LOW);    // left
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  LeftLight();
  delay(700);


  analogWrite(EnableL, 0);     // stop 
  analogWrite(EnableR, 0);
  BrakeLight();
  delay(1000);



  digitalWrite(HighL, LOW);    
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, LOW);    // forward
  digitalWrite(LowL, HIGH);  
  analogWrite(EnableL, 150);
  analogWrite(EnableR, 150);
  OffLight();
  delay(300);
}


void Object()
{
  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);            // stop
  BrakeLight();
  delay(1000);

  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);          // left
  analogWrite(EnableL, 250);
  analogWrite(EnableR, 250);
  LeftLight();
  delay(500);

  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);           // stop
  BrakeLight();
  delay(200);

  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);          // forward
  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  OffLight();
  delay(1000);

  analogWrite(EnableL, 0);           // stop
  analogWrite(EnableR, 0);
  BrakeLight();
  delay(200);

  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, HIGH);         // right
  digitalWrite(LowR, LOW);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  RightLight();
  delay(500);

  analogWrite(EnableL, 0);           // stop
  analogWrite(EnableR, 0);
  BrakeLight();
  delay(1000);

  
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, LOW);         // forward
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 150);
  analogWrite(EnableR, 150);
  OffLight();
  delay(500);

   i  = i+1;
}

void Lane_Change()
{
  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);            //stop
  BrakeLight();
  delay(1000);

  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, HIGH);
  digitalWrite(LowR, LOW);           //Right
  analogWrite(EnableL, 250);
  analogWrite(EnableR, 250);
  RightLight();
  delay(500);

  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);            //stop
  BrakeLight();  
  delay(200);

  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);           //forward
  digitalWrite(HighR, LOW);
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  OffLight();
  delay(800);

  analogWrite(EnableL, 0);           // stop
  analogWrite(EnableR, 0);
  BrakeLight();
  delay(200);

  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  digitalWrite(HighR, LOW);          // left
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  LeftLight();
  delay(500);

  analogWrite(EnableL, 0);           // stop
  analogWrite(EnableR, 0);
  BrakeLight();
  delay(1000);

  
  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, LOW);         // forward
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 150);
  analogWrite(EnableR, 150);
  OffLight();
  delay(500);
}

void LeftLight()
{
  digitalWrite(LeftSignal, HIGH);
  digitalWrite(RightSignal, LOW);
}

void RightLight()
{
  digitalWrite(LeftSignal, LOW);
  digitalWrite(RightSignal, HIGH);
}

void BrakeLight()
{
  digitalWrite(LeftSignal, HIGH);
  digitalWrite(RightSignal, HIGH);
}

void OffLight()
{
  digitalWrite(LeftSignal, LOW);
  digitalWrite(RightSignal, LOW);
}


void loop() 
{
   if (i != 0 && millis() - object_time > 3000)  // [][][] 3seconds
   {
     Lane_Change();
     i = 0;
   }
  
  Data(); // input from Raspi Pi
  if(data==0) // move forward
  {
     Forward();
     OffLight();
  }
   
  else if(data==1) // turn right
  {
     Right1();
     RightLight();
  }
     
  else if(data==2)
  {
     Right2();
     RightLight();
  }
     
  else if(data==3)
  {
     Right3();
     RightLight();
  }
     
  else if(data==4) // turn left
  {
     Left1();
     LeftLight();
  }
    
  else if(data==5)
  {
     Left2();
     LeftLight();
  }
    
  else if(data==6)
  {
     Left3();
     LeftLight();
  }
     
  else if(data==7) // lane end
  {
     UTurn();
  }
  
  else if (data==8) // stop sign
  {
      analogWrite(EnableL, 0);
      analogWrite(EnableR, 0);
      BrakeLight();
      delay(4000);

      analogWrite(EnableL, 150);
      analogWrite(EnableR, 150);
      OffLight();
      delay(1000);//开过范围
  }

  else if(data==9) // object detection
  {
     Object();
     object_time = millis();
  }

  else if(data==10) // traffic light detection
  {
      analogWrite(EnableL, 0);
      analogWrite(EnableR, 0);
      BrakeLight();
      delay(2000);
  }

  else if(data>10)
  {
     Stop();
     BrakeLight();
  }
  
}
