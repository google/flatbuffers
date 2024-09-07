import org.gradle.api.publish.maven.MavenPublication
import org.gradle.api.tasks.bundling.Jar
import org.gradle.kotlin.dsl.`maven-publish`
import org.gradle.kotlin.dsl.signing
import java.util.*

plugins {
  `maven-publish`
  signing
}

// Stub secrets to let the project sync and build without the publication values set up
ext["signing.keyId"] = null
ext["signing.password"] = null
ext["signing.secretKeyRingFile"] = null
ext["ossrhUsername"] = null
ext["ossrhPassword"] = null

// Grabbing secrets from local.properties file or from environment variables, which could be used on CI
val secretPropsFile = project.rootProject.file("local.properties")
if (secretPropsFile.exists()) {
  secretPropsFile.reader().use {
    Properties().apply {
      load(it)
    }
  }.onEach { (name, value) ->
    ext[name.toString()] = value
  }
} else {
  ext["signing.keyId"] = System.getenv("OSSRH_USERNAME")
  ext["signing.password"] = System.getenv("OSSRH_PASSWORD")
  ext["signing.secretKeyRingFile"] = System.getenv("INPUT_GPG_PRIVATE_KEY")
  ext["ossrhUsername"] = System.getenv("OSSRH_USERNAME")
  ext["ossrhPassword"] = System.getenv("OSSRH_PASSWORD")
}

val javadocJar by tasks.registering(Jar::class) {
  archiveClassifier.set("javadoc")
}

fun getExtraString(name: String) = ext[name]?.toString()

publishing {
  // Configure maven central repository
  repositories {
    maven {
      name = "sonatype"
      setUrl("https://s01.oss.sonatype.org/service/local/staging/deploy/maven2/")
      credentials {
        username = getExtraString("ossrhUsername")
        password = getExtraString("ossrhPassword")
      }
    }
  }

  // Configure all publications
  publications.withType<MavenPublication> {
    // Stub javadoc.jar artifact
    artifact(javadocJar.get())

    // Provide artifacts information requited by Maven Central
    pom {
      name.set("Flatbuffers Kotlin")
      description.set("Memory Efficient Serialization Library")
      url.set("https://github.com/google/flatbuffers")

      licenses {
        license {
          name.set("Apache License V2.0")
          url.set("https://raw.githubusercontent.com/google/flatbuffers/master/LICENSE")
        }
      }
      developers {
        developer {
          id.set("https://github.com/paulovap")
          name.set("Paulo Pinheiro")
          email.set("paulovictor.pinheiro@gmail.com")
        }
        developer {
          id.set("https://github.com/dbaileychess")
          name.set("Derek Bailey")
          email.set("dbaileychess@gmail.com")
        }
      }
      scm {
        url.set("https://github.com/google/flatbuffers")
      }
    }
  }
}

// Signing artifacts. Signing.* extra properties values will be used
signing {
  sign(publishing.publications)
}
