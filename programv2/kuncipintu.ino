void lock() {
  if (Firebase.getInt(firebaseData, devicePath + "/action/frontDoor")) {
    // Serial.print("Lock front: ");
    // Serial.println((firebaseData.intData()));
    digitalWrite(lockfront, (firebaseData.intData()));
  }

  if (Firebase.getInt(firebaseData, devicePath + "/action/backDoor")) {
    // Serial.print("Lock back: ");
    // Serial.println((firebaseData.intData()));
    digitalWrite(lockback, (firebaseData.intData()));
  }
}
