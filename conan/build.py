from bincrafters import build_template_default
import os

def set_appveyor_environment():
    if os.getenv("APPVEYOR") is not None:
        compiler_version = os.getenv("CMAKE_VS_VERSION").split(" ")[0].replace('"', '')
        os.environ["CONAN_VISUAL_VERSIONS"] = compiler_version
        os.environ["CONAN_STABLE_BRANCH_PATTERN"] = "master"
        ci_platform = os.getenv("Platform").replace('"', '')
        ci_platform = "x86" if ci_platform == "x86" else "x86_64"
        os.environ["CONAN_ARCHS"] = ci_platform
        os.environ["CONAN_BUILD_TYPES"] = os.getenv("Configuration").replace('"', '')

if __name__ == "__main__":
    os.environ["CONAN_UPLOAD"] = os.getenv("CONAN_UPLOAD", "https://api.bintray.com/conan/aardappel/flatbuffers")
    os.environ["CONAN_STABLE_BRANCH_PATTERN"] = r"v\d+\.\d+\.\d+"
    test_folder = "-tf %s" % os.path.join("conan", "test_package")
    set_appveyor_environment()

    builder = build_template_default.get_builder(args=test_folder, pure_c=False)
    builder.run()
