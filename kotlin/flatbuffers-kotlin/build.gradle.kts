import tasks.*

plugins {
  id("convention.multiplatform")
  id("convention.publication")
}

val libName = "Flatbuffers"
group = "com.google.flatbuffers.kotlin"
version = "2.0.0-SNAPSHOT"

kotlin {
  explicitApi()
  jvm()
  js(IR) {
    browser {
      testTask {
        enabled = false
      }
    }
    binaries.executable()
  }
  macosX64()
  macosArm64()
  iosArm64()
  iosSimulatorArm64()

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

      kotlin.srcDir("src/commonTest/generated/kotlin/")
    }
    val jvmTest by getting {
      dependencies {
        implementation(kotlin("test-junit"))
        implementation("com.google.flatbuffers:flatbuffers-java:2.0.3")
      }
    }
    val jvmMain by getting {
    }

    val macosX64Main by getting
    val macosArm64Main by getting
    val iosArm64Main by getting
    val iosSimulatorArm64Main by getting

    val nativeMain by creating {
      // this sourceSet will hold common cold for all iOS targets
      dependsOn(commonMain)
      macosArm64Main.dependsOn(this)
      macosX64Main.dependsOn(this)
      iosArm64Main.dependsOn(this)
      iosSimulatorArm64Main.dependsOn(this)
    }

    all {
      languageSettings.optIn("kotlin.ExperimentalUnsignedTypes")
    }
  }
}

// Fixes JS issue: https://youtrack.jetbrains.com/issue/KT-49109
rootProject.plugins.withType<org.jetbrains.kotlin.gradle.targets.js.nodejs.NodeJsRootPlugin> {
  rootProject.the<org.jetbrains.kotlin.gradle.targets.js.nodejs.NodeJsRootExtension>().nodeVersion = "16.0.0"

}

// Use the default greeting
tasks.register<GenerateFBTestClasses>("generateFBTestClassesKt") {
  inputFiles.setFrom("$rootDir/../tests/monster_test.fbs",
    "$rootDir/../tests/dictionary_lookup.fbs",
// @todo Seems like nesting code generation is broken for all generators.
// disabling test for now.
//    "$rootDir/../tests/namespace_test/namespace_test1.fbs",
//    "$rootDir/../tests/namespace_test/namespace_test2.fbs",
    "$rootDir/../tests/union_vector/union_vector.fbs",
    "$rootDir/../tests/optional_scalars.fbs")
  includeFolder.set("$rootDir/../tests/include_test")
  outputFolder.set("${projectDir}/src/commonTest/generated/kotlin/")
  variants.add("kotlin-kmp")
}

project.tasks.forEach {
  if (it.name.contains("compileKotlin"))
    it.dependsOn("generateFBTestClassesKt")
}

fun String.intProperty() = findProperty(this).toString().toInt()
