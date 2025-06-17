project('funding-engine', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20'])

add_project_arguments('-w', language: 'cpp')  # disable warnings

cc = meson.get_compiler('cpp')

# Dependencies
boost_inc = include_directories('/opt/homebrew/opt/boost/include')
boost_dep = declare_dependency(
  include_directories: boost_inc,
  link_args: ['-L/opt/homebrew/opt/boost/lib', '-lboost_system', '-lboost_thread']
)

spdlog_dep = dependency('spdlog')
hiredis_dep = dependency('hiredis')

# Sources
srcs = files(
  'src/main.cpp',
  'src/funding_calculator.cpp',
  'src/funding_scheduler.cpp',
  'src/funding_publisher.cpp',
  'src/config_loader.cpp',
  'src/health_check.cpp',
  'src/metrics.cpp'
)

inc = include_directories('include', '.')

executable('funding-engine',
  sources: srcs,
  include_directories: inc,
  dependencies: [
    boost_dep,
    spdlog_dep,
    hiredis_dep
  ]
)
