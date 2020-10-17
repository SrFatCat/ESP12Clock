#ifndef TM1637EFFECTS_H
#define TM1637EFFECTS_H


inline void fillSegments(const uint8_t data[4]){
    for (int i=0; i<4; i++) dataClock[i] = data[i];
}

inline void showDelimit(bool isDelimit){
    //dataClock[1] = display.encodeDigit(hour() % 10);
    if (isDelimit) dataClock[1] |= SEG_DP; else dataClock[1] &= ~SEG_DP;
    display.setSegments(dataClock + 1, 1, 1);
}

inline void showNoWiFi(bool isShowNoWifi){
    //dataClock[1] = display.encodeDigit(hour() % 10);
    if (isShowNoWifi) dataClock[0] |= SEG_DP; else dataClock[0] &= ~SEG_DP;
    display.setSegments(dataClock, 1, 1);
}

void swapBytes(uint8_t* newByte, uint8_t oldByte, uint8_t newP, uint8_t oldP) {
  uint8_t newBit = 0;
  if (oldByte & (1 << oldP)) newBit = 1;
  else newBit = 0;
  *newByte = *newByte | (newBit << newP);
}

void scrollByte(int8_t DispData[], int delayms) {
	// uint8_t lastData[] = {display.encodeDigit(hour()/10), display.encodeDigit(hour()%10), display.encodeDigit(minute()/10), display.encodeDigit(minute()%10)};
    // display.setSegments(lastData);
    // delay(500);
  	uint8_t *lastData = dataClock;
  	lastData[0] &= ~SEG_DP;
	lastData[1] &= ~SEG_DP;
    uint8_t lastBytes[4];
	uint8_t step;
	uint8_t stepArray[4];
	boolean changeByte[4] = {0, 0, 0, 0};
	
	for (uint8_t i = 0; i < 4; i++) {
		if (DispData[i] != lastData[i] ) changeByte[i] = 1;	
		lastBytes[i] = 	lastData[i];	
	}
	
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t lastByte = lastData[i];
		stepArray[i] = lastByte;		
		
		if (changeByte[i]) {
			step = 0;
			swapBytes(&step, lastByte, 6, 0);
			swapBytes(&step, lastByte, 2, 1);
			swapBytes(&step, lastByte, 4, 5);
			swapBytes(&step, lastByte, 3, 6);
			stepArray[i] = step;			
		}		
	}	
	display.setSegments(stepArray);
	delay(delayms);
	
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t lastByte = lastBytes[i];
		stepArray[i] = lastByte;
		
		if (changeByte[i]) {
			step = 0;
			swapBytes(&step, lastByte, 3, 0);
			stepArray[i] = step;
		}
	}
	display.setSegments(stepArray);
	delay(delayms);
	
	for (uint8_t i = 0; i < 4; i++) {
		if (changeByte[i]) {
			stepArray[i] = 0;
		}
	}
	display.setSegments(stepArray);
	delay(delayms);
	
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t lastByte = lastBytes[i];
		uint8_t newByte = DispData[i];
		stepArray[i] = lastByte;
		
		if (changeByte[i]) {
			step = 0;
			swapBytes(&step, newByte, 0, 3);	
			stepArray[i] = step;
		}
	}
	display.setSegments(stepArray);
	delay(delayms);
	
	for (uint8_t i = 0; i < 4; i++) {
		uint8_t newByte = DispData[i];
		stepArray[i] = lastBytes[i];
		
		if (changeByte[i]) {	
			step = 0;
			swapBytes(&step, newByte, 0, 6);
			swapBytes(&step, newByte, 1, 2);
			swapBytes(&step, newByte, 5, 4);
			swapBytes(&step, newByte, 6, 3);	
			stepArray[i] = step;
		}
	}
	display.setSegments(stepArray);
	delay(delayms);
	
	display.setSegments((uint8_t*)DispData);
    delay(1000);
}

void twistByte(int8_t DispData[], int delayms) {
    uint8_t *lastData = dataClock;
	lastData[0] &= ~SEG_DP;
	lastData[1] &= ~SEG_DP;
	uint8_t step;
	uint8_t stepArray[4];
	boolean changeByte[4] = {0, 0, 0, 0};
	
	for (uint8_t i = 0; i < 4; i++) {
		if (DispData[i] != lastData[i]) changeByte[i] = 1;
		stepArray[i] = DispData[i];
	}	
	
	step = step & 0b00111111;			// выкл центральную
	for (uint8_t i = 0; i < 5; i++) {
		step |= (1 << i);				// зажигаем по одной
		for (uint8_t k = 0; k < 4; k++) {
			if (changeByte[k])
				stepArray[k] = step;
		}		
	display.setSegments(stepArray);
	delay(delayms);
	}
	//for (uint8_t r = 0; r < 1; r++) {
		for (uint8_t i = 0; i < 6; i++) {
			step = 0b11000000;
			step = ~(step | (1 << i) | (1 << i + 1));	// бегает дырка
				for (uint8_t k = 0; k < 4; k++) {	
					if (changeByte[k])
						stepArray[k] = step;
					//Serial.println(stepArray[k], BIN);
				}
			display.setSegments(stepArray);
			delay(delayms);
		}
	//}
	step = 0b11000000;
	for (uint8_t i = 0; i < 6; i++) {
		step |= (1 << i);		
		for (uint8_t k = 0; k < 4; k++) {
			if (changeByte[k])
				stepArray[k] = ~step;
		}
		display.setSegments(stepArray);		
		delay(delayms);
	}
	for (uint8_t k = 0; k < 4; k++) {
		if (changeByte[k])
			stepArray[k] = 0b0000000;
	}
	for (uint8_t i = 0; i < 7; i++) {				
		for (uint8_t k = 0; k < 4; k++) {
				if (changeByte[k]) {
				uint8_t newBit = 0;
				if (DispData[k] & (1 << i)) newBit = 1;
				else newBit = 0;
				stepArray[k] |= (newBit << i);
			}			
		}
		display.setSegments(stepArray);
		delay(delayms);
	}
}

void runningString(int8_t DispData[], uint8_t amount, int delayMs) {
  for (uint8_t i = 0; i <= amount-4; i++) {   // выводим
    display.setSegments((uint8_t *)(DispData+i), 4);
    delay(delayMs);
  }
}

void setDigits(int8_t *newDigits, int delayMs){
    uint32_t oldTime = now() - 1;
    char oldDigits[5];
	uint8_t seg;
	sprintf(oldDigits, "%02i%02i",hour(oldTime), minute(oldTime) );
    for(int i = 0; i< 4; i++){
        if (newDigits[i]!=oldDigits[i]){
            for(int j = 0; j<=newDigits[i]-'0'; j++){
				seg = display.encodeDigit(j-'0');
				display.setSegments(&seg, 1, i);
				delay(delayMs);
			} 
        }
    }
}

bool goAnimation(int8_t animation, const uint8_t *data){
    static uint8_t pattern = 0;
	uint8_t newPattern = (animation == -1) ? pattern : animation; 
	DEBUG_PRINT("animation = %i\n", newPattern);
    switch (newPattern){
        case 0:
            twistByte((int8_t *)data, 100);
            break;
        case 1:
            scrollByte((int8_t *)data, 100);
            break;    
        case 2:
            int8_t dataRuning[8];
            memcpy(dataRuning, dataClock, 4);
            memcpy(dataRuning+4, data, 4);
            runningString(dataRuning, 8, 350);
            break;
        case 3:
            int8_t digits[5];
            sprintf((char *)digits, "%02i%02i",hour()+1, 0 );
            setDigits(digits, 200);
            break;

    }
    if (animation == -1 && ++pattern == 2) pattern = 0; //специально последнюю не трогаем, предпоследняя на часы
    return false;
}

void showClockData(bool isShowDelimit, bool isShowNoWifi){
    const uint8_t data[] = {display.encodeDigit(hour()/10), display.encodeDigit(hour()%10), display.encodeDigit(minute()/10), display.encodeDigit(minute()%10)};
	goAnimation((minute() == 0) ? 2 : -1, data);
    fillSegments( data );
    if (isShowDelimit) dataClock[1] |= SEG_DP; else dataClock[1] &= ~SEG_DP;
    if (hour() == 0) dataClock[0] = 0;
	if (isShowNoWifi) dataClock[0] |= SEG_DP; else dataClock[0] &= ~SEG_DP;
    display.setSegments(dataClock);
}

#endif