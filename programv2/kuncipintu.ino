void lock() {
  if (Firebase.getInt(firebaseData, lockFrontPath)) {
    Serial.print("Lock front: ");
    Serial.println((firebaseData.intData()));
    digitalWrite(lockfront, (firebaseData.intData()));
  }

  if (Firebase.getInt(firebaseData, lockBackPath)) {
    Serial.print("Lock back: ");
    Serial.println((firebaseData.intData()));
    digitalWrite(lockback, (firebaseData.intData()));
  }
}
