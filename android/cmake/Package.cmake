function(emcicalc_android_package target)
 set_target_properties(${target} PROPERTIES
  QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
  QT_ANDROID_PACKAGE_NAME "io.github.davoda1.emcicalc"
  QT_ANDROID_APP_NAME "EMCICalc"
  QT_ANDROID_VERSION_NAME "1.0.0"
  QT_ANDROID_VERSION_CODE "1"
  QT_ANDROID_MIN_SDK_VERSION "36"
  QT_ANDROID_TARGET_SDK_VERSION "36"
  QT_ANDROID_COMPILE_SDK_VERSION "36"
  QT_ANDROID_SDK_BUILD_TOOLS_REVISION "36.0.0"
  QT_ANDROID_ABIS "arm64-v8a"
 )
 foreach(ssl IN ITEMS crypto ssl)
  if(EXISTS "${EMCICALC_ANDROID_PREFIX}/lib/lib${ssl}_3.so")
   set_property(TARGET ${target} APPEND PROPERTY QT_ANDROID_EXTRA_LIBS "${EMCICALC_ANDROID_PREFIX}/lib/lib${ssl}_3.so")
  else()
   message(FATAL_ERROR "Missing OpenSSL runtime lib${ssl}_3.so; run scripts/android-dependencies.sh")
  endif()
 endforeach()
endfunction()

# qt_finalize_executable must run before this function creates dependencies.
function(emcicalc_android_runtime target)
 find_program(EMCICALC_PYTHON NAMES python3 REQUIRED NO_CMAKE_FIND_ROOT_PATH)
 get_target_property(deployment_settings ${target} QT_ANDROID_DEPLOYMENT_SETTINGS_FILE)
 add_custom_target(${target}_runtime_deployment
  COMMAND "${EMCICALC_PYTHON}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/android-deployment-runtime.py"
   "${deployment_settings}" "${EMCICALC_ANDROID_PREFIX}/lib/libc++_shared.so"
   "${CMAKE_CURRENT_BINARY_DIR}/android-stdlib"
  COMMENT "Selecting the ABI-verified 16 KiB C++ runtime for ${target}"
  VERBATIM)
 add_dependencies(${target}_prepare_apk_dir ${target}_runtime_deployment)
endfunction()
