void lock() {
  if (Firebase.getInt(firebaseData, "device/lockfront")) {
    //Serial.print("lock front: ");
    //Serial.print((firebaseData.intData()));
    digitalWrite(lockfront, (firebaseData.intData()));
  }

  if (Firebase.getInt(firebaseData, "device/lockback")) {
    //Serial.print("lock back: ");
    //Serial.println((firebaseData.intData()));
    digitalWrite(lockback, (firebaseData.intData()));
  }
}
