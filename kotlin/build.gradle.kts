group = "com.google.flatbuffers"
version = "2.0.0-SNAPSHOT"

buildscript {
  repositories {
    gradlePluginPortal()
    google()
    mavenCentral()
  }
  dependencies {
    classpath(libs.plugin.kotlin.gradle)
    classpath(libs.plugin.kotlinx.benchmark)
    classpath(libs.plugin.jmhreport)
    classpath(libs.plugin.download)
  }
}

allprojects {
  repositories {
    google()
    mavenCentral()
  }
}
