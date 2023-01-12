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
       include(".*FlatbufferBenchmark.*")
    }
  }
  targets {
    register("jvm")
  }
}

kotlin {
  jvm()

  sourceSets {
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
      kotlin.srcDir("src/jvmMain/generated/kotlin/")
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

abstract class GenerateFBTestClasses : DefaultTask() {
  @get:InputFiles
  abstract val inputFiles: ConfigurableFileCollection

  @get:Input
  abstract val includeFolder: Property<String>

  @get:Input
  abstract val outputFolder: Property<String>

  @get:Input
  abstract val variant: Property<String>

  @Inject
  protected open fun getExecActionFactory(): org.gradle.process.internal.ExecActionFactory? {
    throw UnsupportedOperationException()
  }

  init {
    includeFolder.set("")
  }

  @TaskAction
  fun compile() {
    val execAction = getExecActionFactory()!!.newExecAction()
    val sources = inputFiles.asPath.split(":")
    val args = mutableListOf("/Users/ppinheiro/git_tree/flatbuffers/flatc","-o", outputFolder.get(), "--${variant.get()}")
    if (includeFolder.get().isNotEmpty()) {
      args.add("-I")
      args.add(includeFolder.get())
    }
    args.addAll(sources)
    println(args)
    execAction.commandLine = args
    print(execAction.execute())
  }
}

// Use the default greeting
tasks.register<GenerateFBTestClasses>("generateFBTestClassesKt") {
  inputFiles.setFrom("$rootDir/../tests/monster_test.fbs",
    "$rootDir/../tests/dictionary_lookup.fbs")
  includeFolder.set("$rootDir/../tests/include_test")
  outputFolder.set("${projectDir}/src/jvmMain/generated/kotlin/")
  variant.set("kotlin-kmp")
}

//../flatc --cpp --java --kotlin --csharp --ts --php  -o union_vector ./union_vector/union_vector.fbs
tasks.register<GenerateFBTestClasses>("generateFBTestClassesKtVec") {
  inputFiles.setFrom("$rootDir/../tests/union_vector/union_vector.fbs")
  outputFolder.set("${projectDir}/src/jvmMain/generated/kotlin/")
  variant.set("kotlin-kmp")
}

//../flatc --java --kotlin --lobster --ts optional_scalars.fbs
tasks.register<GenerateFBTestClasses>("generateFBTestClassesKtOptionalScalars") {
  inputFiles.setFrom("$rootDir/../tests/optional_scalars.fbs")
  outputFolder.set("${projectDir}/src/jvmMain/generated/kotlin/")
  variant.set("kotlin-kmp")
}

//../flatc --java --kotlin --lobster --ts optional_scalars.fbs
tasks.register<GenerateFBTestClasses>("generateFBTestClassesKtNameSpace") {
  inputFiles.setFrom("$rootDir/../tests/namespace_test/namespace_test1.fbs", "$rootDir/../tests/namespace_test/namespace_test2.fbs")
  outputFolder.set("${projectDir}/src/jvmMain/generated/kotlin/")
  variant.set("kotlin-kmp")
}

project.tasks.named("compileKotlinJvm") {
  dependsOn("generateFBTestClassesKt")
  dependsOn("generateFBTestClassesKtVec")
  dependsOn("generateFBTestClassesKtOptionalScalars")
  dependsOn("generateFBTestClassesKtNameSpace")
}
