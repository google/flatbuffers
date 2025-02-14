rootProject.name = "flatbuffers-kotlin"

pluginManagement {
  repositories {
    gradlePluginPortal()
    google()
    mavenCentral()
  }
}

dependencyResolutionManagement {
  @Suppress("UnstableApiUsage")
  repositories {
    mavenCentral()
  }
}

includeBuild("convention-plugins")
include("flatbuffers-kotlin")
include("benchmark")
