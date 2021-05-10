plugins {
  kotlin("multiplatform") version "1.4.20"
}

group = "com.google.flatbuffers.kotlin"
version = "2.0.0-SNAPSHOT"

kotlin {
  explicitApi()
  jvm()
  js {
    browser {
      binaries.executable()
      testTask {
        useKarma {
          useChromeHeadless()
        }
      }
    }
  }
  macosX64()
  iosArm32()
  iosArm64()
  iosX64()

  sourceSets {
    val commonMain by getting {
      dependencies {
        implementation(kotlin("stdlib-common"))
      }
    }

    val commonTest by getting {
      dependencies {
        implementation(kotlin("test-common"))
        implementation(kotlin("test-annotations-common"))
      }
    }
    val jvmTest by getting {
      dependencies {
        implementation(kotlin("test-junit"))
      }
    }
    val jvmMain by getting {
      kotlin.srcDir("java")
      dependencies {
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core-jvm:1.4.1")
      }
    }

    val jsMain by getting {
      dependsOn(commonMain)
    }
    val jsTest by getting {
      dependsOn(commonTest)
      dependencies {
        implementation(kotlin("test-js"))
      }
    }
    val nativeMain by creating {
        dependsOn(commonMain)
    }
    val nativeTest by creating {
      dependsOn(commonMain)
    }
    val macosX64Main by getting {
      dependsOn(nativeMain)
    }

    val iosArm32Main by getting {
      dependsOn(nativeMain)
    }
    val iosArm64Main by getting {
      dependsOn(nativeMain)
    }
    val iosX64Main by getting {
      dependsOn(nativeMain)
    }

    all {
      languageSettings.enableLanguageFeature("InlineClasses")
      languageSettings.useExperimentalAnnotation("kotlin.ExperimentalUnsignedTypes")
    }
  }

  /* Targets configuration omitted.
   *  To find out how to configure the targets, please follow the link:
   *  https://kotlinlang.org/docs/reference/building-mpp-with-gradle.html#setting-up-targets */
  targets {
    targetFromPreset(presets.getAt("jvm"))
    targetFromPreset(presets.getAt("js"))
    targetFromPreset(presets.getAt("macosX64"))
    targetFromPreset(presets.getAt("iosArm32"))
    targetFromPreset(presets.getAt("iosArm64"))
    targetFromPreset(presets.getAt("iosX64"))
  }
}
