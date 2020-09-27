{
    "targets": [
        {
            "target_name": "<(module_name)",
            "product_dir": "<(module_path)",
            "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],
            "sources": [
                "src/main.cc",
                "src/helpers.cc",
                "src/rtp_decoder.cc",
                "src/rtp_encoder.cc",
                "src/ogg_opus_to_rtp.cc",
                "src/rtp_to_ogg_opus.cc",
                "src/rtp_opus_to_pcm.cc",
            ],
            "dependencies": ["deps/libogg/libogg.gyp:libogg", "deps/libopus/libopus.gyp:libopus"],
            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],
            "conditions": [
                ['OS=="mac"', {
                    "xcode_settings": {
                        "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
                    }
                }]
            ],
            "cflags": [
                "-pthread",
                "-fno-strict-aliasing",
                "-Wall",
                "-Wno-unused-parameter",
                "-Wno-missing-field-initializers",
                "-Wextra",
                "-pipe",
                "-fno-ident",
                "-fdata-sections",
                "-ffunction-sections",
                "-fPIC",
            ],
            "defines": [
                "LARGEFILE_SOURCE",
                "_FILE_OFFSET_BITS=64",
                "WEBRTC_TARGET_PC",
                "WEBRTC_LINUX",
                "WEBRTC_THREAD_RR",
                "EXPAT_RELATIVE_PATH",
                "GTEST_RELATIVE_PATH",
                "JSONCPP_RELATIVE_PATH",
                "WEBRTC_RELATIVE_PATH",
                "POSIX," "__STDC_FORMAT_MACROS",
                "DYNAMIC_ANNOTATIONS_ENABLED=0",
                "NAPI_DISABLE_CPP_EXCEPTIONS",
                "NAPI_VERSION=<(napi_build_version)",
            ],
        },
        {
            "target_name": "action_after_build",
            "type": "none",
            "dependencies": ["<(module_name)"],
            "copies": [
                {
                    "files": ["<(module_path)/<(module_name).node"],
                    "destination": "addon-build/release/install-root/"
                }
            ]
        }
    ],
}
