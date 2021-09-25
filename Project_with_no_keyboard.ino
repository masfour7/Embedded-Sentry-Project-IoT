
// Made by: Mohammad Asfour
// Course: Embedded Systems (spring 2020)
// Professor Campisi
// Project: Create a hand-movement wearable techonology IoT device which unlocks a resource.
// Components used: Atmega328p & MPU6050
// assume keyboard is not present

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

int led = 13;

void setup(void) {
  Serial.begin(115200);

  pinMode(led, OUTPUT); // turn on LED when opened
  
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
  Serial.println("please move right to start recording");
}

int state = 0;
int arr[4];
int i = 0;

  //1, right
  //2, left
  //3, up
  //4, down
  //5, forward
  //6, backward
int Direction(double leftright, double updown, double forback){
    if(leftright > 4){
      Serial.println("left");
      delay(500);
      forback = 0;
      return 2;
    }

    else if(leftright < -4){
      Serial.println("right");
      delay(500);
      return 1;
    }
  
    else if(updown > 13){
      Serial.println("up");
      delay(500);
      return 3;
    }
  
    else if(updown < 7){

      Serial.println("down");
      delay(500);
      return 4;
    }
  
    else if(forback > 3 && leftright < 2 && leftright > -2){
      Serial.println("backward");
      delay(500);
      return 6;
    }
  
    else if(forback < -3 && leftright < 2 && leftright > -2){
      Serial.println("forward");
      delay(500);
      return 5;
    }

    else return 0;
  }

void loop() {

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
//  Serial.print("Acceleration X: ");
//  Serial.print(a.acceleration.x);
//  Serial.print(", Y: ");
//  Serial.print(a.acceleration.y);
//  Serial.print(", Z: ");
//  Serial.print(a.acceleration.z);
//  Serial.println(" m/s^2");

  double leftright = a.acceleration.x;
  double updown = a.acceleration.z;
  double forback = a.acceleration.y;

  int check = Direction(leftright, updown, forback);
  int change = 0;

  switch(state){
    case 0: // start
      if(check == 1){
        state = 1;
        Serial.println("recording started");
      }
    break;
    case 1: //rec1
      if(check == 1){
        state = 2;
        Serial.println("right recorded");
        arr[i] = 1;
        i++;
      }
      else if(check == 2){
        state = 2;
        Serial.println("left recorded");
        arr[i] = 2;
        i++;
      }
      else if(check == 3){
        state = 2;
        Serial.println("up recorded");
        arr[i] = 3;
        i++;
      }
      else if(check == 4){
        state = 2;
        Serial.println("down recorded");
        arr[i] = 4;
        i++;
      }
      else if(check == 5){
        state = 2;
        Serial.println("forward recorded");
        arr[i] = 5;
        i++;
      }
      else if(check == 6){
        state = 2;
        Serial.println("backward recorded");
        arr[i] = 6;
        i++;
      }
    break;
    case 2: //rec 2
      if(check == 1){
        state = 3;
        Serial.println("right recorded");
        arr[i] = 1;
        i++;
      }
      else if(check == 2){
        state = 3;
        Serial.println("left recorded");
        arr[i] = 2;
        i++;
      }
      else if(check == 3){
        state = 3;
        Serial.println("up recorded");
        arr[i] = 3;
        i++;
      }
      else if(check == 4){
        state = 3;
        Serial.println("down recorded");
        arr[i] = 4;
        i++;
      }
      else if(check == 5){
        state = 3;
        Serial.println("forward recorded");
        arr[i] = 5;
        i++;
      }
      else if(check == 6){
        state = 3;
        Serial.println("backward recorded");
        arr[i] = 6;
        i++;
      }
    break;
    case 3: // rec 3
      if(check == 1){
        state = 4;
        Serial.println("right recorded");
        arr[i] = 1;
        i++;
        Serial.println("Great, please enter your lock sequence to open");
      }
      else if(check == 2){
        state = 4;
        Serial.println("left recorded");
        arr[i] = 2;
        i++;
        Serial.println("Great, please enter your lock sequence to open");
      }
      else if(check == 3){
        state = 4;
        Serial.println("up recorded");
        arr[i] = 3;
        i++;
        Serial.println("Great, please enter your lock sequence to open");
      }
      else if(check == 4){
        state = 4;
        Serial.println("down recorded");
        arr[i] = 4;
        i++;
        Serial.println("Great, please enter your lock sequence to open");
      }
      else if(check == 5){
        state = 4;
        Serial.println("forward recorded");
        arr[i] = 5;
        i++;
        Serial.println("Great, please enter your lock sequence to open");
      }
      else if(check == 6){
        state = 4;
        Serial.println("backward recorded");
        arr[i] = 6;
        i++;
        Serial.println("");
        Serial.println("Great, please enter your lock sequence to open");
      }
      if(i == 3){
        i = 0;
      }
    break;
    case 4: //lock 1
      if(arr[0] == check){
        state = 5;
        Serial.println("that's 1, next..?");
      }
      else if(check > 0){
        Serial.println("");
        Serial.println("Wrong key, please start over");
        state = 4;
      }
    break;
    case 5:
      if(arr[1] == check ){
        state = 6;
        Serial.println("that's 2, next..?");
      }
      else if(check > 0){
        Serial.println("");
        Serial.println("Wrong key, please start over");
        state = 4;
      }
    break;
    case 6:
      if(arr[2] == check){
        state = 7;
        Serial.println("that's 3");
      }
      else if(check > 0){
        Serial.println("");
        Serial.println("Wrong key, please start over");
        state = 4;
      }
    break;
    case 7:
      Serial.println("Opened!, going to close after 3 seconds");
      state = 8;
    break;
    case 8:
      digitalWrite(led, HIGH);   // sets the LED on 
      delay(3000);
      digitalWrite(led, LOW);
      Serial.println(" ");
      Serial.println("closed again!");
      Serial.println("insert a lock sequence to open..?");
      state = 4;
    break;
  }

//  Serial.print("Rotation X: ");
//  Serial.print(g.gyro.x);
//  Serial.print(", Y: ");
//  Serial.print(g.gyro.y);
//  Serial.print(", Z: ");
//  Serial.print(g.gyro.z);
//  Serial.println(" rad/s");

  delay(20);
}
