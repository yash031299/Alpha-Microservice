project('report-export-service', 'cpp',
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

redis_inc = include_directories('/opt/homebrew/include', '/opt/homebrew/include/sw')
redis_dep = declare_dependency(
  include_directories: redis_inc,
  link_args: ['-L/opt/homebrew/lib', '-lredis++', '-lhiredis']
)

openssl_inc = include_directories('/opt/homebrew/opt/openssl@3/include')
openssl_dep = declare_dependency(
  include_directories: openssl_inc,
  link_args: ['-L/opt/homebrew/opt/openssl@3/lib', '-lssl', '-lcrypto']
)

spdlog_dep        = dependency('spdlog')
grpc_dep          = dependency('grpc++', required: true)
protobuf_dep      = dependency('protobuf', required: true)
nlohmann_json_dep = dependency('nlohmann_json')

all_deps = [
  boost_dep,
  redis_dep,
  spdlog_dep,
  grpc_dep,
  protobuf_dep,
  nlohmann_json_dep,
  openssl_dep
]

inc = include_directories('include')

# ───── Source Files ─────
sources = files(
  'src/main.cpp',
  'src/config_loader.cpp',
  'src/csv_writer.cpp',
  'src/pdf_writer.cpp',
  'src/report_generator.cpp',
  'src/report_export_server.cpp',
  'src/report_export.pb.cc',
  'src/report_export.grpc.pb.cc'
)

# ───── Main Executable ─────
executable('report-export-service',
  sources,
  include_directories: inc,
  dependencies: all_deps
)
