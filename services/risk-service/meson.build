project('risk-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20']
)

add_project_arguments('-w', language: 'cpp')  # Disable warnings

cc = meson.get_compiler('cpp')

# ───── Core Dependencies ─────
spdlog_dep     = dependency('spdlog')
json_dep       = dependency('nlohmann_json')
grpc_dep       = dependency('grpc++', required: true)
protobuf_dep   = dependency('protobuf', required: true)
doctest_dep    = dependency('doctest', required: true, fallback: ['doctest', 'doctest_dep'])

# ───── Boost & Hiredis (Manual) ─────
boost_inc = include_directories('/opt/homebrew/opt/boost/include')
boost_dep = declare_dependency(
  include_directories: boost_inc,
  link_args: ['-L/opt/homebrew/opt/boost/lib', '-lboost_system', '-lboost_thread']
)

redis_dep = declare_dependency(
  include_directories: include_directories('/opt/homebrew/include'),
  link_args: ['-L/opt/homebrew/lib', '-lhiredis']
)

# ───── gRPC Link Fix (Manual) ─────
grpc_fix_dep = declare_dependency(
  link_args: ['-L/opt/homebrew/lib', '-lgrpc++', '-lgrpc', '-lgpr', '-lprotobuf']
)

# ───── Prometheus (Optional) ─────
prometheus_core_dep = dependency('prometheus-cpp', modules: ['prometheus-cpp::core'], required: false)
prometheus_pull_dep = dependency('prometheus-cpp', modules: ['prometheus-cpp::pull'], required: false)

# ───── Final All Dependencies ─────
all_deps = [
  boost_dep,
  redis_dep,
  spdlog_dep,
  json_dep,
  grpc_dep,
  grpc_fix_dep,
  protobuf_dep,
  doctest_dep,
  prometheus_core_dep,
  prometheus_pull_dep
]

# ───── Includes ─────
inc = include_directories('include', 'include/rms')

# ───── Source Files ─────
sources = files(
  'src/main.cpp',
  'src/health_checker.cpp',
  'src/margin_listener.cpp',
  'src/redis_connection.cpp',
  'src/position_tracker.cpp',
  'src/rms_engine.cpp',
  'src/config_loader.cpp',
  'src/symbol_client.cpp',
  'src/symbol.pb.cc',
  'src/symbol.grpc.pb.cc'
)

# ───── Build Executable ─────
executable('risk-service',
  sources,
  include_directories: inc,
  dependencies: all_deps
)
