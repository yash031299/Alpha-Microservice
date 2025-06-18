project('risk-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20']
)

add_project_arguments('-w', language: 'cpp')  # Disable all warnings

cc = meson.get_compiler('cpp')

# ───── Dependencies ─────
boost_inc = include_directories('/opt/homebrew/opt/boost/include')
boost_dep = declare_dependency(
  include_directories: boost_inc,
  link_args: ['-L/opt/homebrew/opt/boost/lib', '-lboost_system', '-lboost_thread']
)

redis_dep = declare_dependency(
  include_directories: include_directories('/opt/homebrew/include'),
  link_args: ['-L/opt/homebrew/lib', '-lhiredis']
)

spdlog_dep     = dependency('spdlog')
json_dep       = dependency('nlohmann_json')
doctest_dep    = dependency('doctest', required: true, fallback: ['doctest', 'doctest_dep'])

# Prometheus deps are optional
prometheus_core_dep = dependency('prometheus-cpp', modules: ['prometheus-cpp::core'], required: false)
prometheus_pull_dep = dependency('prometheus-cpp', modules: ['prometheus-cpp::pull'], required: false)

all_deps = [
  boost_dep,
  redis_dep,
  spdlog_dep,
  json_dep,
  prometheus_core_dep,
  prometheus_pull_dep
]

inc = include_directories('include', 'include/rms')

# ───── Source Files ─────
sources = files(
  'src/main.cpp',
  'src/health_checker.cpp',
  'src/margin_listener.cpp',
  'src/redis_connection.cpp',
  'src/position_tracker.cpp',
  'src/rms_engine.cpp',
  'src/config_loader.cpp'
)

# ───── Executable ─────
executable('risk-service',
  sources,
  include_directories: inc,
  dependencies: all_deps
)
