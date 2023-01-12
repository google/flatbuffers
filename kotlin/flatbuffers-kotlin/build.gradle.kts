import org.gradle.internal.impldep.org.fusesource.jansi.AnsiRenderer.test
import org.jetbrains.kotlin.gradle.plugin.mpp.apple.XCFramework
import org.jetbrains.kotlin.cli.common.toBooleanLenient
import org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget
import org.jetbrains.kotlin.gradle.plugin.mpp.NativeBuildType
import org.jetbrains.kotlin.gradle.plugin.mpp.apple.XCFrameworkConfig

plugins {
  kotlin("multiplatform")
}


val libName = "Flatbuffers"
group = "com.google.flatbuffers.kotlin"
version = "2.0.0-SNAPSHOT"

kotlin {
  explicitApi()
  jvm()
  macosX64()
  val xcf = XCFramework(libName)

  iosArm64("iosArm64") { configureTarget(xcf) }
  iosArm32("iosArm32") { configureTarget(xcf) }
  iosSimulatorArm64("iosSimulatorArm64") { configureTarget(xcf) }


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
    val iosArm32Main by getting
    val iosArm64Main by getting
    val iosSimulatorArm64Main by getting
    val nativeMain by creating {
      // this sourceSet will hold common cold for all iOS targets
      dependsOn(commonMain)

      macosX64Main.dependsOn(this)
      iosArm32Main.dependsOn(this)
      iosArm64Main.dependsOn(this)
      iosSimulatorArm64Main.dependsOn(this)
    }
    val nativeTest by creating {
      dependsOn(nativeMain)
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
    "$rootDir/../tests/dictionary_lookup.fbs")
  includeFolder.set("$rootDir/../tests/include_test")
  outputFolder.set("${projectDir}/src/commonTest/generated/kotlin/")
  variant.set("kotlin-kmp")
}

//../flatc --cpp --java --kotlin --csharp --ts --php  -o union_vector ./union_vector/union_vector.fbs
tasks.register<GenerateFBTestClasses>("generateFBTestClassesKtVec") {
  inputFiles.setFrom("$rootDir/../tests/union_vector/union_vector.fbs")
  outputFolder.set("${projectDir}/src/commonTest/generated/kotlin/")
  variant.set("kotlin-kmp")
}

//../flatc --java --kotlin --lobster --ts optional_scalars.fbs
tasks.register<GenerateFBTestClasses>("generateFBTestClassesKtOptionalScalars") {
  inputFiles.setFrom("$rootDir/../tests/optional_scalars.fbs")
  outputFolder.set("${projectDir}/src/commonTest/generated/kotlin/")
  variant.set("kotlin-kmp")
}

//../flatc --java --kotlin --lobster --ts optional_scalars.fbs
tasks.register<GenerateFBTestClasses>("generateFBTestClassesKtNameSpace") {
  inputFiles.setFrom("$rootDir/../tests/namespace_test/namespace_test1.fbs", "$rootDir/../tests/namespace_test/namespace_test2.fbs")
  outputFolder.set("${projectDir}/src/commonTest/generated/kotlin/")
  variant.set("kotlin-kmp")
}

project.tasks.named("compileKotlinJvm") {
  dependsOn("generateFBTestClassesKt")
  dependsOn("generateFBTestClassesKtVec")
  dependsOn("generateFBTestClassesKtOptionalScalars")
  dependsOn("generateFBTestClassesKtNameSpace")
}

fun String.intProperty() = findProperty(this).toString().toInt()

fun KotlinNativeTarget.configureTarget(xcf: XCFrameworkConfig) {
  binaries.framework {
    baseName = libName
    xcf.add(this)
  }
  // [Experimental] Enables export of KDoc comments to generated Objective-C headers
  compilations["main"].kotlinOptions.freeCompilerArgs += "-Xexport-kdoc"
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
