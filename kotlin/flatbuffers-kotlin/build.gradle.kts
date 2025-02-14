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

  js {
    browser {
      testTask {
        enabled = false
      }
    }
    binaries.executable()
  }

  linuxX64()
  linuxArm64()

  macosX64()
  macosArm64()
  iosArm64()
  iosSimulatorArm64()

  sourceSets {
    all {
      languageSettings.optIn("kotlin.ExperimentalUnsignedTypes")
      languageSettings.optIn("kotlin.experimental.ExperimentalNativeApi")
    }

    commonMain {
      dependencies {
        implementation(kotlin("stdlib-common"))
      }
    }

    commonTest {
      kotlin.srcDir("src/commonTest/generated/kotlin/")

      dependencies {
        implementation(kotlin("test"))
      }
    }

    jvmTest {
      dependencies {
        implementation(kotlin("test-junit"))
        implementation("com.google.flatbuffers:flatbuffers-java:2.0.3")
      }
    }
  }
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
