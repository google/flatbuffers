package tasks

import org.gradle.api.*
import org.gradle.api.file.*
import org.gradle.api.provider.*
import org.gradle.api.tasks.*
import javax.inject.*

abstract class GenerateFBTestClasses : DefaultTask() {
  @get:InputFiles
  abstract val inputFiles: ConfigurableFileCollection

  @get:Input
  abstract val includeFolder: Property<String>

  @get:Input
  abstract val outputFolder: Property<String>

  @get:Input
  abstract val variants: ListProperty<String>

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
    val flatcBinary = project.layout.projectDirectory.file("../../flatc").asFile.absolutePath
    val sources = inputFiles.asPath.split(":")
    val langs = variants.get().map { "--$it" }
    val args = mutableListOf(flatcBinary, "-o", outputFolder.get(), *langs.toTypedArray())
    if (includeFolder.get().isNotEmpty()) {
      args.add("-I")
      args.add(includeFolder.get())
    }
    args.addAll(sources)
    execAction.commandLine = args
  }
}
