plugins {
  alias(libs.plugins.kotlin.multiplatform) apply false
  alias(libs.plugins.kotlinx.benchmark) apply false
  alias(libs.plugins.jmhreport) apply false
  alias(libs.plugins.download) apply false
}

allprojects {
  repositories {
    google()
    mavenCentral()
  }
}
