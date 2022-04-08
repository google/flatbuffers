plugins {
  kotlin("multiplatform")
}

group = "com.google.flatbuffers.kotlin"
version = "2.0.0-SNAPSHOT"

kotlin {
  explicitApi()
  jvm()
  js {
    browser {
     testTask {
        useKarma {
          useChromeHeadless()
        }
      }
    }
    binaries.executable()
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
        implementation(kotlin("test"))
      }
    }
    val jvmTest by getting {
      dependencies {
        implementation(kotlin("test-junit"))
      }
    }
    val jvmMain by getting {
      kotlin.srcDir("java")
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
      languageSettings.optIn("kotlin.ExperimentalUnsignedTypes")
    }
  }
}

// Fixes JS issue: https://youtrack.jetbrains.com/issue/KT-49109
rootProject.plugins.withType<org.jetbrains.kotlin.gradle.targets.js.nodejs.NodeJsRootPlugin> {
  rootProject.the<org.jetbrains.kotlin.gradle.targets.js.nodejs.NodeJsRootExtension>().nodeVersion = "16.0.0"
}
