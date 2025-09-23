plugins {
  `kotlin-dsl`
}

repositories {
  gradlePluginPortal()
}

dependencies {
  fun DependencyHandler.plugin(dependency: Provider<PluginDependency>): Dependency =
    dependency.get().run { create("$pluginId:$pluginId.gradle.plugin:$version") }

  implementation(plugin(libs.plugins.kotlin.multiplatform))
}
