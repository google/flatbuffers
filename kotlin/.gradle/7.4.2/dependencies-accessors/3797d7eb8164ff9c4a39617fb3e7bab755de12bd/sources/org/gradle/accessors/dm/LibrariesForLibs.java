package org.gradle.accessors.dm;

import org.gradle.api.NonNullApi;
import org.gradle.api.artifacts.MinimalExternalModuleDependency;
import org.gradle.plugin.use.PluginDependency;
import org.gradle.api.artifacts.ExternalModuleDependencyBundle;
import org.gradle.api.artifacts.MutableVersionConstraint;
import org.gradle.api.provider.Provider;
import org.gradle.api.provider.ProviderFactory;
import org.gradle.api.internal.catalog.AbstractExternalDependencyFactory;
import org.gradle.api.internal.catalog.DefaultVersionCatalog;
import java.util.Map;
import javax.inject.Inject;

/**
 * A catalog of dependencies accessible via the `libs` extension.
*/
@NonNullApi
public class LibrariesForLibs extends AbstractExternalDependencyFactory {

    private final AbstractExternalDependencyFactory owner = this;
    private final KotlinxLibraryAccessors laccForKotlinxLibraryAccessors = new KotlinxLibraryAccessors(owner);
    private final MoshiLibraryAccessors laccForMoshiLibraryAccessors = new MoshiLibraryAccessors(owner);
    private final PluginLibraryAccessors laccForPluginLibraryAccessors = new PluginLibraryAccessors(owner);
    private final VersionAccessors vaccForVersionAccessors = new VersionAccessors(providers, config);
    private final BundleAccessors baccForBundleAccessors = new BundleAccessors(providers, config);
    private final PluginAccessors paccForPluginAccessors = new PluginAccessors(providers, config);

    @Inject
    public LibrariesForLibs(DefaultVersionCatalog config, ProviderFactory providers) {
        super(config, providers);
    }

        /**
         * Creates a dependency provider for gson (com.google.code.gson:gson)
         * This dependency was declared in catalog libs.versions.toml
         */
        public Provider<MinimalExternalModuleDependency> getGson() { return create("gson"); }

        /**
         * Creates a dependency provider for junit (junit:junit)
         * This dependency was declared in catalog libs.versions.toml
         */
        public Provider<MinimalExternalModuleDependency> getJunit() { return create("junit"); }

    /**
     * Returns the group of libraries at kotlinx
     */
    public KotlinxLibraryAccessors getKotlinx() { return laccForKotlinxLibraryAccessors; }

    /**
     * Returns the group of libraries at moshi
     */
    public MoshiLibraryAccessors getMoshi() { return laccForMoshiLibraryAccessors; }

    /**
     * Returns the group of libraries at plugin
     */
    public PluginLibraryAccessors getPlugin() { return laccForPluginLibraryAccessors; }

    /**
     * Returns the group of versions at versions
     */
    public VersionAccessors getVersions() { return vaccForVersionAccessors; }

    /**
     * Returns the group of bundles at bundles
     */
    public BundleAccessors getBundles() { return baccForBundleAccessors; }

    /**
     * Returns the group of plugins at plugins
     */
    public PluginAccessors getPlugins() { return paccForPluginAccessors; }

    public static class KotlinxLibraryAccessors extends SubDependencyFactory {
        private final KotlinxBenchmarkLibraryAccessors laccForKotlinxBenchmarkLibraryAccessors = new KotlinxBenchmarkLibraryAccessors(owner);

        public KotlinxLibraryAccessors(AbstractExternalDependencyFactory owner) { super(owner); }

        /**
         * Returns the group of libraries at kotlinx.benchmark
         */
        public KotlinxBenchmarkLibraryAccessors getBenchmark() { return laccForKotlinxBenchmarkLibraryAccessors; }

    }

    public static class KotlinxBenchmarkLibraryAccessors extends SubDependencyFactory {

        public KotlinxBenchmarkLibraryAccessors(AbstractExternalDependencyFactory owner) { super(owner); }

            /**
             * Creates a dependency provider for runtime (org.jetbrains.kotlinx:kotlinx-benchmark-runtime)
             * This dependency was declared in catalog libs.versions.toml
             */
            public Provider<MinimalExternalModuleDependency> getRuntime() { return create("kotlinx.benchmark.runtime"); }

    }

    public static class MoshiLibraryAccessors extends SubDependencyFactory {

        public MoshiLibraryAccessors(AbstractExternalDependencyFactory owner) { super(owner); }

            /**
             * Creates a dependency provider for kotlin (com.squareup.moshi:moshi-kotlin)
             * This dependency was declared in catalog libs.versions.toml
             */
            public Provider<MinimalExternalModuleDependency> getKotlin() { return create("moshi.kotlin"); }

    }

    public static class PluginLibraryAccessors extends SubDependencyFactory {
        private final PluginKotlinLibraryAccessors laccForPluginKotlinLibraryAccessors = new PluginKotlinLibraryAccessors(owner);

        public PluginLibraryAccessors(AbstractExternalDependencyFactory owner) { super(owner); }

            /**
             * Creates a dependency provider for gver (com.github.ben-manes:gradle-versions-plugin)
             * This dependency was declared in catalog libs.versions.toml
             */
            public Provider<MinimalExternalModuleDependency> getGver() { return create("plugin.gver"); }

        /**
         * Returns the group of libraries at plugin.kotlin
         */
        public PluginKotlinLibraryAccessors getKotlin() { return laccForPluginKotlinLibraryAccessors; }

    }

    public static class PluginKotlinLibraryAccessors extends SubDependencyFactory implements DependencyNotationSupplier {

        public PluginKotlinLibraryAccessors(AbstractExternalDependencyFactory owner) { super(owner); }

            /**
             * Creates a dependency provider for kotlin (org.jetbrains.kotlin:kotlin-gradle-plugin)
             * This dependency was declared in catalog libs.versions.toml
             */
            public Provider<MinimalExternalModuleDependency> asProvider() { return create("plugin.kotlin"); }

            /**
             * Creates a dependency provider for serialization (org.jetbrains.kotlin:kotlin-serialization)
             * This dependency was declared in catalog libs.versions.toml
             */
            public Provider<MinimalExternalModuleDependency> getSerialization() { return create("plugin.kotlin.serialization"); }

    }

    public static class VersionAccessors extends VersionFactory  {

        private final KotlinxVersionAccessors vaccForKotlinxVersionAccessors = new KotlinxVersionAccessors(providers, config);
        private final MoshiVersionAccessors vaccForMoshiVersionAccessors = new MoshiVersionAccessors(providers, config);
        private final PluginVersionAccessors vaccForPluginVersionAccessors = new PluginVersionAccessors(providers, config);
        public VersionAccessors(ProviderFactory providers, DefaultVersionCatalog config) { super(providers, config); }

            /**
             * Returns the version associated to this alias: gson (2.8.5)
             * If the version is a rich version and that its not expressible as a
             * single version string, then an empty string is returned.
             * This version was declared in catalog libs.versions.toml
             */
            public Provider<String> getGson() { return getVersion("gson"); }

            /**
             * Returns the version associated to this alias: junit (4.12)
             * If the version is a rich version and that its not expressible as a
             * single version string, then an empty string is returned.
             * This version was declared in catalog libs.versions.toml
             */
            public Provider<String> getJunit() { return getVersion("junit"); }

        /**
         * Returns the group of versions at versions.kotlinx
         */
        public KotlinxVersionAccessors getKotlinx() { return vaccForKotlinxVersionAccessors; }

        /**
         * Returns the group of versions at versions.moshi
         */
        public MoshiVersionAccessors getMoshi() { return vaccForMoshiVersionAccessors; }

        /**
         * Returns the group of versions at versions.plugin
         */
        public PluginVersionAccessors getPlugin() { return vaccForPluginVersionAccessors; }

    }

    public static class KotlinxVersionAccessors extends VersionFactory  {

        private final KotlinxBenchmarkVersionAccessors vaccForKotlinxBenchmarkVersionAccessors = new KotlinxBenchmarkVersionAccessors(providers, config);
        public KotlinxVersionAccessors(ProviderFactory providers, DefaultVersionCatalog config) { super(providers, config); }

        /**
         * Returns the group of versions at versions.kotlinx.benchmark
         */
        public KotlinxBenchmarkVersionAccessors getBenchmark() { return vaccForKotlinxBenchmarkVersionAccessors; }

    }

    public static class KotlinxBenchmarkVersionAccessors extends VersionFactory  {

        public KotlinxBenchmarkVersionAccessors(ProviderFactory providers, DefaultVersionCatalog config) { super(providers, config); }

            /**
             * Returns the version associated to this alias: kotlinx.benchmark.runtime (0.4.2)
             * If the version is a rich version and that its not expressible as a
             * single version string, then an empty string is returned.
             * This version was declared in catalog libs.versions.toml
             */
            public Provider<String> getRuntime() { return getVersion("kotlinx.benchmark.runtime"); }

    }

    public static class MoshiVersionAccessors extends VersionFactory  {

        public MoshiVersionAccessors(ProviderFactory providers, DefaultVersionCatalog config) { super(providers, config); }

            /**
             * Returns the version associated to this alias: moshi.kotlin (1.11.0)
             * If the version is a rich version and that its not expressible as a
             * single version string, then an empty string is returned.
             * This version was declared in catalog libs.versions.toml
             */
            public Provider<String> getKotlin() { return getVersion("moshi.kotlin"); }

    }

    public static class PluginVersionAccessors extends VersionFactory  {

        public PluginVersionAccessors(ProviderFactory providers, DefaultVersionCatalog config) { super(providers, config); }

            /**
             * Returns the version associated to this alias: plugin.gver (0.42.0)
             * If the version is a rich version and that its not expressible as a
             * single version string, then an empty string is returned.
             * This version was declared in catalog libs.versions.toml
             */
            public Provider<String> getGver() { return getVersion("plugin.gver"); }

            /**
             * Returns the version associated to this alias: plugin.kotlin (1.6.10)
             * If the version is a rich version and that its not expressible as a
             * single version string, then an empty string is returned.
             * This version was declared in catalog libs.versions.toml
             */
            public Provider<String> getKotlin() { return getVersion("plugin.kotlin"); }

    }

    public static class BundleAccessors extends BundleFactory {

        public BundleAccessors(ProviderFactory providers, DefaultVersionCatalog config) { super(providers, config); }

            /**
             * Creates a dependency bundle provider for plugins which is an aggregate for the following dependencies:
             * <ul>
             *    <li>org.jetbrains.kotlin:kotlin-gradle-plugin</li>
             *    <li>org.jetbrains.kotlin:kotlin-serialization</li>
             *    <li>com.github.ben-manes:gradle-versions-plugin</li>
             * </ul>
             * This bundle was declared in catalog libs.versions.toml
             */
            public Provider<ExternalModuleDependencyBundle> getPlugins() { return createBundle("plugins"); }

    }

    public static class PluginAccessors extends PluginFactory {

        public PluginAccessors(ProviderFactory providers, DefaultVersionCatalog config) { super(providers, config); }

    }

}
