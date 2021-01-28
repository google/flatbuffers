import org.jetbrains.kotlin.ir.backend.js.compile

plugins {
  kotlin("multiplatform") version "1.4.20"
  id("org.jetbrains.kotlin.plugin.allopen") version "1.4.20"
  id("kotlinx.benchmark") version "0.2.0-dev-20"
  id("io.morethan.jmhreport") version "0.9.0"
}

// allOpen plugin is needed for the benchmark annotations.
// for more infomation, see https://github.com/Kotlin/kotlinx-benchmark#gradle-plugin
allOpen {
  annotation("org.openjdk.jmh.annotations.State")
}

group = "com.google.flatbuffers.jmh"
version = "1.12.0-SNAPSHOT"

// This plugin generates a static html page with the aggregation
// of all benchmarks ran. very useful visualization tool.
jmhReport {
  val baseFolder = project.file("build/reports/benchmarks/main").absolutePath
  val lastFolder = project.file(baseFolder).list()?.sortedArray()?.lastOrNull() ?: ""
  jmhResultPath = "$baseFolder/$lastFolder/jvm.json"
  jmhReportOutput = "$baseFolder/$lastFolder"
}

// For now we benchmark on JVM only
benchmark {
  configurations {
    this.getByName("main") {
      iterations = 5
      iterationTime = 300
      iterationTimeUnit = "ms"
    }
  }
  targets {
    register("jvm")
  }
}

kotlin {
  jvm {
    withJava()
    compilations.all {
      kotlinOptions {
        jvmTarget = JavaVersion.VERSION_1_8.toString()
      }
    }
  }

  sourceSets {

    all {
      languageSettings.enableLanguageFeature("InlineClasses")
      languageSettings.useExperimentalAnnotation("kotlin.ExperimentalUnsignedTypes")
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
      dependencies {
        implementation("org.jetbrains.kotlinx:kotlinx.benchmark.runtime:0.2.0-dev-20")
        implementation(kotlin("stdlib-common"))
        implementation(project(":flatbuffers-kotlin"))
        implementation("org.jetbrains.kotlinx:kotlinx.benchmark.runtime-jvm:0.2.0-dev-20")
        implementation("org.jetbrains.kotlin:kotlin-stdlib-jdk8")
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core-jvm:1.4.1")

      }
    }

    /* Targets configuration omitted.
     *  To find out how to configure the targets, please follow the link:
     *  https://kotlinlang.org/docs/reference/building-mpp-with-gradle.html#setting-up-targets
     */
    targets {
      targetFromPreset(presets.getAt("jvm"))
    }
  }
}
