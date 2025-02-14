import org.jetbrains.kotlin.gradle.dsl.JvmTarget
import org.jetbrains.kotlin.gradle.dsl.KotlinCompile
import org.jetbrains.kotlin.gradle.tasks.KotlinJvmCompile
import java.nio.charset.*

plugins {
  kotlin("multiplatform")
}

tasks.withType<KotlinCompile<*>> {
  kotlinOptions {
    // https://kotlinlang.org/docs/whatsnew13.html#progressive-mode
    freeCompilerArgs += "-progressive"
  }
}

tasks.withType<KotlinJvmCompile> {
  compilerOptions {
    jvmTarget.set(JvmTarget.JVM_1_8)
    freeCompilerArgs.add("-Xjvm-default=all")
  }
}

tasks.withType<JavaCompile> {
  options.encoding = StandardCharsets.UTF_8.toString()
  sourceCompatibility = JavaVersion.VERSION_1_8.toString()
  targetCompatibility = JavaVersion.VERSION_1_8.toString()
}
