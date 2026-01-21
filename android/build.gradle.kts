// Top-level build file for Otto Matic Android port
buildscript {
    repositories {
        google()  // Required for Android Gradle Plugin
        mavenCentral()
    }
    dependencies {
        classpath("com.android.tools.build:gradle:8.2.2")
    }
}

allprojects {
    repositories {
        google()
        mavenCentral()
    }
}
