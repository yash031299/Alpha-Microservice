project('symbol-registry-service', 'cpp',
  version: '1.0',
  default_options: ['cpp_std=c++20'])

add_project_arguments('-w', language: 'cpp')  # Optional: suppress warnings

cc = meson.get_compiler('cpp')

# ───── Dependencies ─────
spdlog_dep = dependency('spdlog')
json_dep = dependency('nlohmann_json')
grpc_dep = dependency('grpc++', required: true)
protobuf_dep = dependency('protobuf', required: true)

grpc_link_args = [
  '-L/opt/homebrew/lib',
  '-lgrpc++',
  '-lgrpc',
  '-lgpr',
  '-lprotobuf'
]

grpc_fix_dep = declare_dependency(
  link_args: grpc_link_args
)

all_deps = [
  spdlog_dep,
  json_dep,
  grpc_dep,
  protobuf_dep,
  grpc_fix_dep
]

# ───── Source Files ─────
srcs = files(
  'src/main.cpp',
  'src/grpc_server.cpp',
  'src/symbol_service.cpp',
  'src/symbol.pb.cc',
  'src/symbol.grpc.pb.cc'
)

# ───── Include Path ─────
inc = include_directories('include')

# ───── Executable ─────
executable('symbol-registry-service',
  sources: srcs,
  include_directories: inc,
  dependencies: all_deps
)
