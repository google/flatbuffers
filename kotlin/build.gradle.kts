plugins {
  id("com.diffplug.spotless") version "5.8.2"
}

group = "com.google.flatbuffers"
version = "2.0.0-SNAPSHOT"

subprojects {

  repositories {
    maven { setUrl("https://plugins.gradle.org/m2/") }
    mavenCentral()
  }
}

buildscript {
  repositories {
    maven { setUrl("https://plugins.gradle.org/m2/") }
    gradlePluginPortal()
    mavenCentral()
  }
}

// plugin used to enforce code style
spotless {
  val klintConfig = mapOf("indent_size" to "2", "continuation_indent_size" to "2")
  kotlin {
    target("**/*.kt")
    ktlint("0.40.0").userData(klintConfig)
    trimTrailingWhitespace()
    indentWithSpaces()
    endWithNewline()
    licenseHeaderFile("$rootDir/spotless/spotless.kt").updateYearWithLatest(false)
    targetExclude("**/spotless.kt", "**/build/**")
  }
  kotlinGradle {
    target("*.gradle.kts")
    ktlint().userData(klintConfig)
  }
}
