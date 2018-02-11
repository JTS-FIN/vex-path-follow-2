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

task main()
{
	setColorMode(port3, colorTypeRGB_Hue_Reflected);
	int notSeenTapeCount = 0;
	float seenTapeDuration;
	int timeArrivedOnTape;
	int unsigned long runStartTime = 0;
	int unsigned long runEndTime = 0;
	int movementSpeed = 0;
	int turningSpeed = TURNING_SPEED_STAGE1;
	int currentState = STATE_RUN_NOT_STARTED;
	string currentStateString = "Not started";
	int rotationDirection = 1;
	int maxRotationDegrees = MAX_ROTATION_DEGREES_STAGE1;
	int recurringRotationDirectionChanges = 0;
	int i = 0;
	while (1) {
		int redValue = getColorRedChannel(port3);
		int blueValue = getColorBlueChannel(port3);
		int greenValue = getColorGreenChannel(port3);
		int combinedColorValue = redValue + blueValue + greenValue;
		int runTime = nPgmTime;
		if (currentState == STATE_RUN_ENDED) {
			runTime = runEndTime;

			setMotorSpeed(motor1, 0);
			setMotorSpeed(motor6, 0);
			displayTextLine(0, "Run endeded");
			displayTextLine(1, "Runtime: %d", (runTime - runStartTime) / 1000);
			displayTextLine(2, "\o/ \O/");
			displayTextLine(3, "woo WOO woo!!1");
			displayTextLine(4, "mage on ylbee xD");
			i++;
			if (i % 80 == 0) {
				setTouchLEDColor(port2, colorGreen);
				setTouchLEDColor(port5, colorRed);
			}
			else if (i % 40 == 0) {
				setTouchLEDColor(port2, colorRed);
				setTouchLEDColor(port5, colorGreen);
			}
			continue;
		}
		if (35 < redValue && redValue < 45 && 35 < blueValue && blueValue < 45) {
			currentState = STATE_ON_BASE;
			currentStateString = "On base";
		}
		else if (currentState == STATE_RUN_NOT_STARTED) {
			//woot
		}
		else if (combinedColorValue < 300 && currentState == STATE_OUT_OF_TRACK) {
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

		displayTextLine(0, "Runstate: %s", currentStateString);
		displayTextLine(1, "Current speed: %d", movementSpeed);
		displayTextLine(2, "Running time: %d", (runTime - runStartTime) / 1000);
		displayTextLine(3, "Gyro: %d", abs(getGyroDegrees(port4)));

		if (currentState == STATE_ON_BASE) {
			// Alustetaan aloitusaika
			if (runStartTime == 0) {
				runStartTime = nPgmTime;
				resetGyro(port4);
			}
			// Ollaan alussa, mennään pois lähtöpisteeltä
			// AIKAA KAKSI SEKUNTIAA!!!
			if (nPgmTime - runStartTime < 10000) {
				movementSpeed = BASE_MOVEMENT_SPEED;
				setMotorSpeed(motor1, movementSpeed);
				setMotorSpeed(motor6, movementSpeed);
			}
			else {
				if (runEndTime == 0) {
					runEndTime = nPgmTime;
				}
				currentState = STATE_RUN_ENDED;
				continue;
			}
		}
		else if (currentState == STATE_ARRIVED_ON_TAPE) {
			resetGyro(port4);
			timeArrivedOnTape = nPgmTime;
		}
		else if (currentState == STATE_ON_TAPE) {
			seenTapeDuration = nPgmTime - timeArrivedOnTape;
			movementSpeed = BASE_MOVEMENT_SPEED + (seenTapeDuration / 1000) * 20;
			if (movementSpeed > MAX_MOVEMENT_SPEED) {
				movementSpeed = MAX_MOVEMENT_SPEED;
			}
			setTouchLEDColor(port2, colorGreen);
			setMotorSpeed(motor1, movementSpeed);
			setMotorSpeed(motor6, movementSpeed);
			notSeenTapeCount = 0;
			recurringRotationDirectionChanges = 0;
			resetGyro(port4);
			maxRotationDegrees = MAX_ROTATION_DEGREES_STAGE1;
			turningSpeed = TURNING_SPEED_STAGE1;
		}
		else if (currentState == STATE_OUT_OF_TRACK) {
			seenTapeDuration = 0;
			setTouchLEDColor(port2, colorRed);

			setMotorSpeed(motor1, turningSpeed *  rotationDirection);
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
	}
}
