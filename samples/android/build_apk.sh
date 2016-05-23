#!/bin/bash -eu
#
# Copyright (c) 2013 Google, Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
# 1. The origin of this software must not be misrepresented; you must not
# claim that you wrote the original software. If you use this software
# in a product, an acknowledgment in the product documentation would be
# appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
# misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# Build, deploy, debug / execute a native Android package based upon
# NativeActivity.

declare -r script_directory=$(dirname $0)
declare -r android_root=${script_directory}/../../../../../../
declare -r script_name=$(basename $0)
declare -r android_manifest=AndroidManifest.xml
declare -r os_name=$(uname -s)

# Minimum Android target version supported by this project.
: ${BUILDAPK_ANDROID_TARGET_MINVERSION:=10}
# Directory containing the Android SDK
# (http://developer.android.com/sdk/index.html).
: ${ANDROID_SDK_HOME:=}
# Directory containing the Android NDK
# (http://developer.android.com/tools/sdk/ndk/index.html).
: ${NDK_HOME:=}

# Display script help and exit.
usage() {
  echo "
Build the Android package in the current directory and deploy it to a
connected device.

Usage: ${script_name} \\
         [ADB_DEVICE=serial_number] [BUILD=0] [DEPLOY=0] [RUN_DEBUGGER=1] \
         [LAUNCH=0] [SWIG_BIN=swig_binary_directory] [SWIG_LIB=swig_include_directory] [ndk-build arguments ...]

ADB_DEVICE=serial_number:
  serial_number specifies the device to deploy the built apk to if multiple
  Android devices are connected to the host.
BUILD=0:
  Disables the build of the package.
DEPLOY=0:
  Disables the deployment of the built apk to the Android device.
RUN_DEBUGGER=1:
  Launches the application in gdb after it has been deployed.  To debug in
  gdb, NDK_DEBUG=1 must also be specified on the command line to build a
  debug apk.
LAUNCH=0:
  Disable the launch of the apk on the Android device.
SWIG_BIN=swig_binary_directory:
  The directory where the SWIG binary lives. No need to set this if SWIG is
  installed and point to from your PATH variable.
SWIG_LIB=swig_include_directory:
  The directory where SWIG shared include files are, usually obtainable from
  commandline with \"swig -swiglib\". No need to set this if SWIG is installed
  and point to from your PATH variable.
ndk-build arguments...:
  Additional arguments for ndk-build.  See ndk-build -h for more information.
" >&2
  exit 1
}

# Get the number of CPU cores present on the host.
get_number_of_cores() {
  case ${os_name} in
    Darwin)
      sysctl hw.ncpu | awk '{ print $2 }'
      ;;
    CYGWIN*|Linux)
      awk '/^processor/ { n=$3 } END { print n + 1 }' /proc/cpuinfo
      ;;
    *)
      echo 1
      ;;
  esac
}

# Get the package name from an AndroidManifest.xml file.
get_package_name_from_manifest() {
  xmllint --xpath 'string(/manifest/@package)' "${1}"
}

# Get the library name from an AndroidManifest.xml file.
get_library_name_from_manifest() {
  echo "\
setns android=http://schemas.android.com/apk/res/android
xpath string(/manifest/application/activity\
[@android:name=\"android.app.NativeActivity\"]/meta-data\
[@android:name=\"android.app.lib_name\"]/@android:value)" |
  xmllint --shell "${1}" | awk '/Object is a string/ { print $NF }'
}

# Get the number of Android devices connected to the system.
get_number_of_devices_connected() {
  adb devices -l | \
    awk '/^..*$/ { if (p) { print $0 } }
         /List of devices attached/ { p = 1 }' | \
    wc -l
  return ${PIPESTATUS[0]}
}

# Kill a process and its' children.  This is provided for cygwin which
# doesn't ship with pkill.
kill_process_group() {
  local parent_pid="${1}"
  local child_pid=
  for child_pid in $(ps -f | \
                     awk '{ if ($3 == '"${parent_pid}"') { print $2 } }'); do
    kill_process_group "${child_pid}"
  done
  kill "${parent_pid}" 2>/dev/null
}

# Find and run "adb".
adb() {
  local adb_path=
  for path in "$(which adb 2>/dev/null)" \
              "${ANDROID_SDK_HOME}/sdk/platform-tools/adb" \
              "${android_root}/prebuilts/sdk/platform-tools/adb"; do
    if [[ -e "${path}" ]]; then
      adb_path="${path}"
      break
    fi
  done
  if [[ "${adb_path}" == "" ]]; then
    echo -e "Unable to find adb." \
           "\nAdd the Android ADT sdk/platform-tools directory to the" \
           "PATH." >&2
    exit 1
  fi
  "${adb_path}" "$@"
}

# Find and run "android".
android() {
  local android_executable=android
  if echo "${os_name}" | grep -q CYGWIN; then
    android_executable=android.bat
  fi
  local android_path=
  for path in "$(which ${android_executable})" \
              "${ANDROID_SDK_HOME}/sdk/tools/${android_executable}" \
              "${android_root}/prebuilts/sdk/tools/${android_executable}"; do
    if [[ -e "${path}" ]]; then
      android_path="${path}"
      break
    fi
  done
  if [[ "${android_path}" == "" ]]; then
    echo -e "Unable to find android tool." \
           "\nAdd the Android ADT sdk/tools directory to the PATH." >&2
    exit 1
  fi
  # Make sure ant is installed.
  if [[ "$(which ant)" == "" ]]; then
    echo -e "Unable to find ant." \
            "\nPlease install ant and add to the PATH." >&2
    exit 1
  fi

  "${android_path}" "$@"
}

# Find and run "ndk-build"
ndkbuild() {
  local ndkbuild_path=
  for path in "$(which ndk-build 2>/dev/null)" \
              "${NDK_HOME}/ndk-build" \
              "${android_root}/prebuilts/ndk/current/ndk-build"; do
    if [[ -e "${path}" ]]; then
      ndkbuild_path="${path}"
      break
    fi
  done
  if [[ "${ndkbuild_path}" == "" ]]; then
    echo -e "Unable to find ndk-build." \
            "\nAdd the Android NDK directory to the PATH." >&2
    exit 1
  fi
  "${ndkbuild_path}" "$@"
}

# Get file modification time of $1 in seconds since the epoch.
stat_mtime() {
  local filename="${1}"
  case ${os_name} in
    Darwin) stat -f%m "${filename}" 2>/dev/null || echo 0 ;;
    *) stat -c%Y "${filename}" 2>/dev/null || echo 0 ;;
  esac
}

# Build the native (C/C++) build targets in the current directory.
build_native_targets() {
  # Save the list of output modules in the install directory so that it's
  # possible to restore their timestamps after the build is complete.  This
  # works around a bug in ndk/build/core/setup-app.mk which results in the
  # unconditional execution of the clean-installed-binaries rule.
  restore_libraries="$(find libs -type f 2>/dev/null | \
                       sed -E 's@^libs/(.*)@\1@')"

  # Build native code.
  ndkbuild -j$(get_number_of_cores) "$@"

  # Restore installed libraries.
  # Obviously this is a nasty hack (along with ${restore_libraries} above) as
  # it assumes it knows where the NDK will be placing output files.
  (
    IFS=$'\n'
    for libpath in ${restore_libraries}; do
      source_library="obj/local/${libpath}"
      target_library="libs/${libpath}"
      if [[ -e "${source_library}" ]]; then
        cp -a "${source_library}" "${target_library}"
      fi
    done
  )
}

# Select the oldest installed android build target that is at least as new as
# BUILDAPK_ANDROID_TARGET_MINVERSION.  If a suitable build target isn't found,
# this function prints an error message and exits with an error.
select_android_build_target() {
  local -r android_targets_installed=$( \
    android list targets | \
    awk -F'"' '/^id:.*android/ { print $2 }')
  local android_build_target=
  for android_target in $(echo "${android_targets_installed}" | \
                          awk -F- '{ print $2 }' | sort -n); do
    local isNumber='^[0-9]+$'
    # skip preview API releases e.g. 'android-L'
    if [[ $android_target =~ $isNumber ]]; then
      if [[ $((android_target)) -ge \
          $((BUILDAPK_ANDROID_TARGET_MINVERSION)) ]]; then
        android_build_target="android-${android_target}"
        break
      fi
    # else
      # The API version is a letter, so skip it.
    fi
  done
  if [[ "${android_build_target}" == "" ]]; then
    echo -e \
      "Found installed Android targets:" \
      "$(echo ${android_targets_installed} | sed 's/ /\n  /g;s/^/\n  /;')" \
      "\nAndroid SDK platform" \
      "android-$((BUILDAPK_ANDROID_TARGET_MINVERSION))" \
      "must be installed to build this project." \
      "\nUse the \"android\" application to install API" \
      "$((BUILDAPK_ANDROID_TARGET_MINVERSION)) or newer." >&2
    exit 1
  fi
  echo "${android_build_target}"
}

# Sign unsigned apk $1 and write the result to $2 with key store file $3 and
# password $4.
# If a key store file $3 and password $4 aren't specified, a temporary
# (60 day) key is generated and used to sign the package.
sign_apk() {
  local unsigned_apk="${1}"
  local signed_apk="${2}"
  if [[ $(stat_mtime "${unsigned_apk}") -gt \
          $(stat_mtime "${signed_apk}") ]]; then
    local -r key_alias=$(basename ${signed_apk} .apk)
    local keystore="${3}"
    local key_password="${4}"
    [[ "${keystore}" == "" ]] && keystore="${unsigned_apk}.keystore"
    [[ "${key_password}" == "" ]] && \
      key_password="${key_alias}123456"
    if [[ ! -e ${keystore} ]]; then
      keytool -genkey -v -dname "cn=, ou=${key_alias}, o=fpl" \
        -storepass ${key_password} \
        -keypass ${key_password} -keystore ${keystore} \
        -alias ${key_alias} -keyalg RSA -keysize 2048 -validity 60
    fi
    cp "${unsigned_apk}" "${signed_apk}"
    jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 \
      -keystore ${keystore} -storepass ${key_password} \
      -keypass ${key_password} "${signed_apk}" ${key_alias}
  fi
}

# Build the apk $1 for package filename $2 in the current directory using the
# ant build target $3.
build_apk() {
  local -r output_apk="${1}"
  local -r package_filename="${2}"
  local -r ant_target="${3}"
  # Get the list of installed android targets and select the oldest target
  # that is at least as new as BUILDAPK_ANDROID_TARGET_MINVERSION.
  local -r android_build_target=$(select_android_build_target)
  [[ "${android_build_target}" == "" ]] && exit 1
  echo "Building ${output_apk} for target ${android_build_target}" >&2

  # Create / update build.xml and local.properties files.
  if [[ $(stat_mtime "${android_manifest}") -gt \
          $(stat_mtime build.xml) ]]; then
    android update project --target "${android_build_target}" \
                           -n ${package_filename} --path .
  fi

  # Use ant to build the apk.
  ant -quiet ${ant_target}

  # Sign release apks with a temporary key as these packages will not be
  # redistributed.
  local unsigned_apk="bin/${package_filename}-${ant_target}-unsigned.apk"
  if [[ "${ant_target}" == "release" ]]; then
    sign_apk "${unsigned_apk}" "${output_apk}" "" ""
  fi
}

# Uninstall package $1 and install apk $2 on device $3 where $3 is "-s device"
# or an empty string.  If $3 is an empty string adb will fail when multiple
# devices are connected to the host system.
install_apk() {
  local -r uninstall_package_name="${1}"
  local -r install_apk="${2}"
  local -r adb_device="${3}"
  # Uninstall the package if it's already installed.
  adb ${adb_device} uninstall "${uninstall_package_name}" 1>&2 > /dev/null || \
    true # no error check

  # Install the apk.
  # NOTE: The following works around adb not returning an error code when
  # it fails to install an apk.
  echo "Install ${install_apk}" >&2
  local -r adb_install_result=$(adb ${adb_device} install "${install_apk}")
  echo "${adb_install_result}"
  if echo "${adb_install_result}" | grep -qF 'Failure ['; then
    exit 1
  fi
}

# Launch previously installed package $1 on device $2.
# If $2 is an empty string adb will fail when multiple devices are connected
# to the host system.
launch_package() {
  (
    # Determine the SDK version of Android on the device.
    local -r android_sdk_version=$(
      adb ${adb_device} shell cat system/build.prop | \
      awk -F= '/ro.build.version.sdk/ {
                 v=$2; sub(/[ \r\n]/, "", v); print v
               }')

    # Clear logs from previous runs.
    # Note that logcat does not just 'tail' the logs, it dumps the entire log
    # history.
    adb ${adb_device} logcat -c

    local finished_msg='Displayed '"${package_name}"
    local timeout_msg='Activity destroy timeout.*'"${package_name}"
    # Maximum time to wait before stopping log monitoring.  0 = infinity.
    local launch_timeout=0
    # If this is a Gingerbread device, kill log monitoring after 10 seconds.
    if [[ $((android_sdk_version)) -le 10 ]]; then
      launch_timeout=10
    fi
    # Display logcat in the background.
    # Stop displaying the log when the app launch / execution completes or the
    # logcat
    (
      adb ${adb_device} logcat | \
        awk "
          {
            print \$0
          }

          /ActivityManager.*: ${finished_msg}/ {
            exit 0
          }

          /ActivityManager.*: ${timeout_msg}/ {
            exit 0
          }" &
      adb_logcat_pid=$!;
      if [[ $((launch_timeout)) -gt 0 ]]; then
        sleep $((launch_timeout));
        kill ${adb_logcat_pid};
      else
        wait ${adb_logcat_pid};
      fi
    ) &
    logcat_pid=$!
    # Kill adb logcat if this shell exits.
    trap "kill_process_group ${logcat_pid}" SIGINT SIGTERM EXIT

    # If the SDK is newer than 10, "am" supports stopping an activity.
    adb_stop_activity=
    if [[ $((android_sdk_version)) -gt 10 ]]; then
      adb_stop_activity=-S
    fi

    # Launch the activity and wait for it to complete.
    adb ${adb_device} shell am start ${adb_stop_activity} -n \
      ${package_name}/android.app.NativeActivity

    wait "${logcat_pid}"
  )
}

# See usage().
main() {
  # Parse arguments for this script.
  local adb_device=
  local ant_target=release
  local disable_deploy=0
  local disable_build=0
  local run_debugger=0
  local launch=1
  local build_package=1
  for opt; do
    case ${opt} in
      # NDK_DEBUG=0 tells ndk-build to build this as debuggable but to not
      # modify the underlying code whereas NDK_DEBUG=1 also builds as debuggable
      # but does modify the code
      NDK_DEBUG=1) ant_target=debug ;;
      NDK_DEBUG=0) ant_target=debug ;;
      ADB_DEVICE*) adb_device="$(\
        echo "${opt}" | sed -E 's/^ADB_DEVICE=([^ ]+)$/-s \1/;t;s/.*//')" ;;
      BUILD=0) disable_build=1 ;;
      DEPLOY=0) disable_deploy=1 ;;
      RUN_DEBUGGER=1) run_debugger=1 ;;
      LAUNCH=0) launch=0 ;;
      clean) build_package=0 disable_deploy=1 launch=0 ;;
      -h|--help|help) usage ;;
    esac
  done

  # If a target device hasn't been specified and multiple devices are connected
  # to the host machine, display an error.
  local -r devices_connected=$(get_number_of_devices_connected)
  if [[ "${adb_device}" == "" && $((devices_connected)) -gt 1 && \
        ($((disable_deploy)) -eq 0 || $((launch)) -ne 0 || \
         $((run_debugger)) -ne 0) ]]; then
    if [[ $((disable_deploy)) -ne 0 ]]; then
      echo "Deployment enabled, disable using DEPLOY=0" >&2
    fi
    if [[ $((launch)) -ne 0 ]]; then
     echo "Launch enabled." >&2
    fi
    if [[ $((disable_deploy)) -eq 0 ]]; then
      echo "Deployment enabled." >&2
    fi
    if [[ $((run_debugger)) -ne 0 ]]; then
      echo "Debugger launch enabled." >&2
    fi
    echo "
Multiple Android devices are connected to this host.  Either disable deployment
and execution of the built .apk using:
  \"${script_name} DEPLOY=0 LAUNCH=0\"

or specify a device to deploy to using:
  \"${script_name} ADB_DEVICE=\${device_serial}\".

The Android devices connected to this machine are:
$(adb devices -l)
" >&2
    exit 1
  fi

  if [[ $((disable_build)) -eq 0 ]]; then
    # Build the native target.
    build_native_targets "$@"
  fi

  # Get the package name from the manifest.
  local -r package_name=$(get_package_name_from_manifest "${android_manifest}")
  if [[ "${package_name}" == "" ]]; then
    echo -e "No package name specified in ${android_manifest},"\
            "skipping apk build, deploy"
            "\nand launch steps." >&2
    exit 0
  fi
  local -r package_basename=${package_name/*./}
  local package_filename=$(get_library_name_from_manifest ${android_manifest})
  [[ "${package_filename}" == "" ]] && package_filename="${package_basename}"

  # Output apk name.
  local -r output_apk="bin/${package_filename}-${ant_target}.apk"

  if [[ $((disable_build)) -eq 0 && $((build_package)) -eq 1 ]]; then
    # Build the apk.
    build_apk "${output_apk}" "${package_filename}" "${ant_target}"
  fi

  # Deploy to the device.
  if [[ $((disable_deploy)) -eq 0 ]]; then
    install_apk "${package_name}" "${output_apk}" "${adb_device}"
  fi

  if [[ "${ant_target}" == "debug" && $((run_debugger)) -eq 1 ]]; then
    # Start debugging.
    ndk-gdb ${adb_device} --start
  elif [[ $((launch)) -eq 1 ]]; then
    launch_package "${package_name}" "${adb_device}"
  fi
}

main "$@"
