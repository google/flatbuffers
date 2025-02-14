package tasks

import org.gradle.api.*
import org.gradle.api.file.*
import org.gradle.api.provider.*
import org.gradle.api.tasks.*
import org.gradle.process.*
import javax.inject.*

abstract class GenerateFBTestClasses : DefaultTask() {
  @get:InputFiles
  abstract val inputFiles: ConfigurableFileCollection

  @get:Input
  @get:Optional
  abstract val includeFolder: Property<String>

  @get:Input
  abstract val outputFolder: Property<String>

  @get:Input
  abstract val variants: ListProperty<String>

  @get:Inject
  internal abstract val execOperations: ExecOperations

  @TaskAction
  fun compile() {
    val rootDirectory = project.layout.projectDirectory.dir("../..")
    val flatcBinary = rootDirectory.file("flatc").asFile.absolutePath

    execOperations.exec {
      executable = flatcBinary

      args("-o", outputFolder.get())
      args(variants.get().map { "--$it" })
      includeFolder.orNull?.let { args("-I", it) }
      args(inputFiles.asPath.split(":"))
    }
  }
}
