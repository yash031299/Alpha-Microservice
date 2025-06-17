project('stream_service', 'cpp',
  default_options : ['cpp_std=c++20', 'warning_level=3', 'buildtype=debug']
)

add_project_arguments('-w', language: 'cpp')  # Disable all warnings

# Dependencies (installed via Conan and exposed via PkgConfig)
dependency_spdlog = dependency('spdlog')
dependency_fmt = dependency('fmt')
dependency_boost = dependency('boost')
dependency_hiredis = dependency('hiredis')
dependency_rapidjson = dependency('rapidjson')

# Include directory
inc = include_directories('include')

# Build the executable
executable('stream_service',
  [
    'src/main.cpp',
    'src/redis_stream_listener.cpp',
    'src/utils.cpp'
  ],
  include_directories : inc,
  dependencies : [
    dependency_spdlog,
    dependency_fmt,
    dependency_boost,
    dependency_hiredis,
    dependency_rapidjson
  ]
)
