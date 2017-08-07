//just about done!


long int Sensor = 15;
int binArr[100];
int binOpenSpot = 0;
void setup()
{
  Serial.begin(250000);
  build1Byte(Sensor);
}

void loop() {
  Sensor += 1;
  int a;

  while (a < binOpenSpot) {

    Serial.print(binArr[a]);
    a++;
  }
  Serial.println();
}



void build3Byte(long int numb) {
  //range +- 8,388,607

  if (numb >= 0) { //if positive first bit = 1
    binArr[binOpenSpot] = 1;
    binOpenSpot ++;
  } else {
    binArr[binOpenSpot] = 0;
    binOpenSpot ++; //if negative first bit = 0
    numb = abs(numb);
  }

  if (numb > -8388607 && numb < 8388607) {


    if (numb < 2) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 4) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 8) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 16) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 32) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 64) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 128) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 256) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++; // 1 byte
    }
    if (numb < 512) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 1024) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 2048) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 4096) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 8192) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 16384) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 32768) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 65536) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++; //2byte
    }
    if (numb < 131072) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 262144) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 524288) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 1048576) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 2097152) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 4194304) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++; //23 bits of data
    }

  } else {
    numb = 8388607;
  }
  String h = "";
  int a = 0;
  h += String(numb, BIN);
  int msgLen = h.length();

  while (msgLen != 0) {

    String x = h.substring(a, a + 1);
    char d[10];
    x.toCharArray(d, 10);

    if (int(d[0]) == 49) {
      binArr[binOpenSpot] = 1;
    } else {
      binArr[binOpenSpot] = 0;
    }

    binOpenSpot++;
    a++;
    msgLen--;
  }
}

void build2Byte(long int numb) {
  //range +- 32,767

  if (numb >= 0) { //if positive first bit = 1
    binArr[binOpenSpot] = 1;
    binOpenSpot ++;
  } else {
    binArr[binOpenSpot] = 0;
    binOpenSpot ++; //if negative first bit = 0
    numb = abs(numb);
  }

  if (numb > -32767 && numb < 32767) {


    if (numb < 2) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 4) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 8) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 16) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 32) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 64) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 128) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 256) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++; // 1 byte
    }
    if (numb < 512) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 1024) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 2048) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 4096) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 8192) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 16384) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }

  } else {
    numb = 32767;
  }
  String h = "";
  int a = 0;
  h += String(numb, BIN);
  int msgLen = h.length();

  while (msgLen != 0) {

    String x = h.substring(a, a + 1);
    char d[10];
    x.toCharArray(d, 10);

    if (int(d[0]) == 49) {
      binArr[binOpenSpot] = 1;
    } else {
      binArr[binOpenSpot] = 0;
    }

    binOpenSpot++;
    a++;
    msgLen--;
  }
}

void build1Byte(long int numb) {
  //range +- 127

  if (numb >= 0) { //if positive first bit = 1
    binArr[binOpenSpot] = 1;
    binOpenSpot ++;
  } else {
    binArr[binOpenSpot] = 0;
    binOpenSpot ++; //if negative first bit = 0
    numb = abs(numb);
  }

  if (numb > -127 && numb < 127) {


    if (numb < 2) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 4) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 8) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 16) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 32) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }
    if (numb < 64) {
      binArr[binOpenSpot] = 0;
      binOpenSpot ++;
    }

  } else {
    numb = 127;
  }
  String h = "";
  int a = 0;
  h += String(numb, BIN);
  int msgLen = h.length();

  while (msgLen != 0) {

    String x = h.substring(a, a + 1);
    char d[10];
    x.toCharArray(d, 10);

    if (int(d[0]) == 49) {
      binArr[binOpenSpot] = 1;
    } else {
      binArr[binOpenSpot] = 0;
    }

    binOpenSpot++;
    a++;
    msgLen--;
  }
}

