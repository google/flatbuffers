import org.jetbrains.kotlin.ir.backend.js.compile

plugins {
  kotlin("multiplatform")
  id("org.jetbrains.kotlinx.benchmark")
  id("io.morethan.jmhreport")
  id("de.undercouch.download")
}

group = "com.google.flatbuffers.jmh"
version = "2.0.0-SNAPSHOT"

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
      // uncomment for benchmarking JSON op only
      include(".*JsonBenchmark.*")
    }
  }
  targets {
    register("jvm")
  }
}

kotlin {
  jvm()

  sourceSets {

    all {
      languageSettings.enableLanguageFeature("InlineClasses")
    }

    val jvmMain by getting {
      dependencies {
        implementation(kotlin("stdlib-common"))
        implementation(project(":flatbuffers-kotlin"))
        implementation(libs.kotlinx.benchmark.runtime)
        implementation("com.google.flatbuffers:flatbuffers-java:2.0.3")
        // json serializers
        implementation(libs.moshi.kotlin)
        implementation(libs.gson)
      }
    }
  }
}

// This task download all JSON files used for benchmarking
tasks.register<de.undercouch.gradle.tasks.download.Download>("downloadMultipleFiles") {
  // We are downloading json benchmark samples from serdes-rs project.
  // see: https://github.com/serde-rs/json-benchmark/blob/master/data
  val baseUrl = "https://github.com/serde-rs/json-benchmark/raw/master/data/"
  src(listOf("$baseUrl/canada.json", "$baseUrl/twitter.json", "$baseUrl/citm_catalog.json"))
  dest(File("${project.projectDir.absolutePath}/src/jvmMain/resources"))
  overwrite(false)
}
