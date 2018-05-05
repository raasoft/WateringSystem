#include <Arduino.h>

String head(String s) {

  //returns text after leading slash until next slash
  if (s.charAt(0) == '/')
    return head(s.substring(1));
  else
    return s.substring(0, s.indexOf('/'));
}

String tail(String s) {
  //returns text after second slash (if first char is a slash)

  if (s.charAt(s.length() - 1) != '/')
    s = s + '/';

  if (s.charAt(0) == '/')
    return tail(s.substring(1));
  else {
    int first = s.indexOf('/');
    return s.substring(first + 1, s.lastIndexOf('/'));
  }
}

void printdebug(String s) {
  if (DEBUG) {
    Serial.print(s);
    Serial.println(char(194));
  }
}




int headNumber(String s) {
  int number;
  String numberString = head(s);
  if (numberString.length() > 0) {
    number = numberString.toInt();
  } else {
    number = -1;
  }
  return number;
}


