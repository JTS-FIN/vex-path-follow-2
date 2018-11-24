//#pragma config(Sensor, port2,  distanceMM,     sensorVexIQ_Distance)
#pragma config(Sensor, port3,  colorDetector,  sensorVexIQ_ColorHue)
#pragma config(Sensor, port4,  gyroSensor,     sensorVexIQ_Gyro)
#pragma config(Sensor, port5,  touchLedMovementSpeed,       sensorVexIQ_LED)
#pragma config(Sensor, port2,  touchLedTurningSpeed,       sensorVexIQ_LED)
#pragma config(Motor,  motor1, leftMotor,     tmotorVexIQ, openLoop, driveLeft, encoder)
#pragma config(Motor,  motor6, rightMotor,    tmotorVexIQ, openLoop, reversed, driveRight, encoder)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#define BASE_MOVEMENT_SPEED 25
#define TURNING_SPEED_STAGE1 20
#define TURNING_SPEED_STAGE2 25
#define TURNING_SPEED_OVER_ALREADY_SCANNED_AREA 70
#define TURN_OVER_THE_TAPE_DEGREES 1
#define MAX_ROTATION_DEGREES_STAGE1 55
#define MAX_ROTATION_DEGREES_STAGE2 150
#define MAX_MOVEMENT_SPEED 60
#define MIN_MOVEMENT_SPEED 30
#define STATE_OUT_OF_TRACK 1
#define STATE_ON_TAPE 2
#define STATE_ON_BASE 3
#define STATE_RUN_ENDED 4
#define STATE_RUN_NOT_STARTED 5
#define STATE_ARRIVED_ON_TAPE 6
#define NUMBER_OF_RUNS 2

typedef struct {
  int startTimes[NUMBER_OF_RUNS];
  int endTimes[NUMBER_OF_RUNS];
  int currentLap;
} lapRecorder;

void startLap(int *movementSpeed, lapRecorder *lapTimes) {
	lapTimes->startTimes[lapTimes->currentLap] = nPgmTime;
	*movementSpeed = BASE_MOVEMENT_SPEED;
	setMotorSpeed(motor1, *movementSpeed);
	setMotorSpeed(motor6, *movementSpeed);
}

void endLap(lapRecorder *lapTimes) {
	lapTimes->endTimes[lapTimes->currentLap] = nPgmTime;
	lapTimes->currentLap++;
}

void ON_BASE(int *currentState, int *movementSpeed, lapRecorder *lapTimes) {
	resetGyro(port4);

	// If starttime for the lap hasnt been set, we start a new lap
	if (lapTimes->startTimes[lapTimes->currentLap] == 0) {
		startLap(movementSpeed, lapTimes);
	}
	// If it is more than 10sec since we started the lap, and we are still in base
	// then we have already made the lap
	else if (nPgmTime - lapTimes->startTimes[lapTimes->currentLap] > 10000) {
		endLap(lapTimes);
	}
}

void ARRIVED_ON_TAPE(int *timeArrivedOnTape) {
	resetGyro(port4);
	*timeArrivedOnTape = nPgmTime;
}

void ON_TAPE(int *timeArrivedOnTape, int *movementSpeed, int *recurringRotationDirectionChanges, int *turningSpeed, int *maxRotationDegrees) {
	float seenTapeDuration = nPgmTime - *timeArrivedOnTape;
	*movementSpeed = BASE_MOVEMENT_SPEED + (seenTapeDuration / 1000) * 20;
	if (*movementSpeed > MAX_MOVEMENT_SPEED) {
		*movementSpeed = MAX_MOVEMENT_SPEED;
	}
	setTouchLEDColor(port2, colorGreen);
	setMotorSpeed(motor1, *movementSpeed);
	setMotorSpeed(motor6, *movementSpeed);

	*recurringRotationDirectionChanges = 0;
	resetGyro(port4);
	*maxRotationDegrees = MAX_ROTATION_DEGREES_STAGE1;
	*turningSpeed = TURNING_SPEED_STAGE1;
}


task main()
{
	setColorMode(port3, colorTypeRGB_Hue_Reflected);
	float seenTapeDuration;
	int timeArrivedOnTape;
	int movementSpeed = 0;
	int turningSpeed = TURNING_SPEED_STAGE1;
	int currentState = STATE_RUN_NOT_STARTED;
	string currentStateString = "Not started";
	int rotationDirection = 1;
	int maxRotationDegrees = MAX_ROTATION_DEGREES_STAGE1;
	int recurringRotationDirectionChanges = 0;
	int i = 0;
	lapRecorder lapTimes;
	for (int k = 0; k < NUMBER_OF_RUNS; k++) {
		lapTimes.startTimes[k] = 0;
		lapTimes.endTimes[k] = 0;
	}
	lapTimes.currentLap = 0;

	while (1) {
		int redValue = getColorRedChannel(port3);
		int blueValue = getColorBlueChannel(port3);
		int greenValue = getColorGreenChannel(port3);
		int combinedColorValue = redValue + blueValue + greenValue;

		// ==============================
		// Start of state switching logic
		if (lapTimes.currentLap == NUMBER_OF_RUNS) {
			currentState = STATE_RUN_ENDED;
		}
		else {
			if (35 < redValue && redValue < 45 && 35 < blueValue && blueValue < 45) {
				currentState = STATE_ON_BASE;
				currentStateString = "On base";
			}
			else if (currentState != STATE_RUN_NOT_STARTED) {
				if (combinedColorValue < 300 && currentState == STATE_OUT_OF_TRACK) {
					currentState = STATE_ARRIVED_ON_TAPE;
					currentStateString = "Arrived on tape";
				}
				else if (combinedColorValue < 300 && currentState == STATE_ARRIVED_ON_TAPE) {
					currentState = STATE_ON_TAPE;
					currentStateString = "On tape";
				}
				else if (combinedColorValue > 300) {
					currentState = STATE_OUT_OF_TRACK;
					currentStateString = "Out of track";
				}
			}
		}
		// End of state switching logic

		// ===========================
		// Start of calling state logic
		if (currentState == STATE_RUN_ENDED) {
			setMotorSpeed(motor1, 0);
			setMotorSpeed(motor6, 0);
			for (int k = 0; k < NUMBER_OF_RUNS; k++) {
				displayTextLine(k, "Lap %d: %d", k, (lapTimes.endTimes[k] - lapTimes.startTimes[k]) / 1000);
			}
			i++;
			if (i % 160 == 0) {
				setTouchLEDColor(port2, colorGreen);
				setTouchLEDColor(port5, colorRed);
			}
			else if (i % 80 == 0) {
				setTouchLEDColor(port2, colorRed);
				setTouchLEDColor(port5, colorGreen);
			}
			continue;
		}
		else if (currentState == STATE_ON_BASE) {
			ON_BASE(&currentState, &movementSpeed, &lapTimes);
		}
		else if (currentState == STATE_ARRIVED_ON_TAPE) {
			ARRIVED_ON_TAPE(&timeArrivedOnTape);
		}
		else if (currentState == STATE_ON_TAPE) {
			ON_TAPE(&timeArrivedOnTape, &movementSpeed, &recurringRotationDirectionChanges, &turningSpeed, &maxRotationDegrees);
		}
		else if (currentState == STATE_OUT_OF_TRACK) {
			seenTapeDuration = 0;
			setTouchLEDColor(port2, colorRed);

			setMotorSpeed(motor1, turningSpeed * rotationDirection);
			setMotorSpeed(motor6, turningSpeed * -1 * rotationDirection);
			displayTextLine(4, "Turning to tape %d", rotationDirection);
			setTouchLEDColor(port5, colorNone);

			if (abs(getGyroDegrees(port4)) > maxRotationDegrees) {
				rotationDirection = rotationDirection * -1;
				recurringRotationDirectionChanges += 1;
				repeatUntil(abs(getGyroDegrees(port4)) < 5) {
					// When returning towards 0 degrees, we know we have scanned the area
					// and we can speed up the turning speed to ridiculous speeds
					setMotorSpeed(motor1, TURNING_SPEED_OVER_ALREADY_SCANNED_AREA * rotationDirection);
					setMotorSpeed(motor6, TURNING_SPEED_OVER_ALREADY_SCANNED_AREA * -1 * rotationDirection);

					setTouchLEDColor(port5, colorOrange);
					displayTextLine(4, "Flippinging the direction!");
				}
			}
			if (recurringRotationDirectionChanges > 0) {
				// Here we enter stage 2 of the scan
				maxRotationDegrees = MAX_ROTATION_DEGREES_STAGE2;
				turningSpeed = TURNING_SPEED_STAGE2;
			}
		}
		// End of calling state logic

		// =====================
		// Start of screen update
		displayTextLine(0, "Runstate: %s", currentStateString);
		displayTextLine(1, "Lap number: %d", lapTimes.currentLap);
		displayTextLine(3, "Gyro: %d", abs(getGyroDegrees(port4)));
		// End of screen update
	}


}



