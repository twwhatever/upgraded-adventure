#include <array>
#include <fstream>
#include <iostream>
#include <wasm_export.h>

static bool module_read_callback(const char* module_name, uint8_t** buffer, uint32_t* size) {
  std::cout << "WAMR requested " << module_name << std::endl;

  std::ifstream bytecode_file("wasm/go/sample/go_sample.wasm");
  
  std::string* bytecode = new std::string(
                       (std::istreambuf_iterator<char>(bytecode_file)),
                       std::istreambuf_iterator<char>());
  *buffer = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(bytecode->data()));
  *size = bytecode->size();
  return true;
}

static void module_destroy_callback(uint8_t* buffer, uint32_t size) {
  
}

// If we want to share memory between modules, we need to make sure they have
// access to it.
uint32_t wasm_malloc(wasm_exec_env_t env, uint32_t size) {
  // Need to allocate memory within the module instance.
  auto module = wasm_runtime_get_module_inst(env);

  if (!module) {
    std::cerr << "Failed to get module instance from execution environment";
    return 0;
  }

  void* native_rv;
  auto rv = wasm_runtime_module_malloc(module, size, &native_rv);

  std::cout << "Allocated " << rv << " at " << native_rv;

  return rv;
}

bool wasm_free(wasm_exec_env_t env, uint32_t ptr) {
  auto module = wasm_runtime_get_module_inst(env);

  if (!module) {
    std::cerr << "Failed to get module instance from execution environment";
    return false;
  }

  wasm_runtime_module_free(module, ptr);

  return true;
}

int main(int argc, char* argv[]) {
  char *buffer;
  std::array<char, 128> error_buf;
  uint32_t size, stack_size = 8092, heap_size = 8092;
  error_buf[0] = 0;
  static char global_heap_buf[1024 * 1024 * 1024];

  RuntimeInitArgs wasm_init_args;
  wasm_init_args.mem_alloc_type = Alloc_With_Pool;
  wasm_init_args.mem_alloc_option.pool.heap_buf = global_heap_buf;
  wasm_init_args.mem_alloc_option.pool.heap_size = sizeof(global_heap_buf);

  static std::array<NativeSymbol, 2> native_symbols = {
    {{"wasm_malloc", (void*)wasm_malloc, "(i)i", nullptr},
     {"wasm_free", (void*)wasm_free, "(*)", nullptr}}
  };

  wasm_init_args.n_native_symbols = native_symbols.size();
  wasm_init_args.native_module_name = "env";
  wasm_init_args.native_symbols = native_symbols.data();

  auto wasm_initialized = wasm_runtime_full_init(&wasm_init_args);

  if (!wasm_initialized) {
    std::cerr << "Failed to initialize WASM!" << std::endl;
    return -10;
  }

  wasm_runtime_set_module_reader(module_read_callback, module_destroy_callback);

  // std::ifstream bytecode_file("wasm/go/sample/go_sample.wasm");
  //  
  // std::string bytecode(
  // (std::istreambuf_iterator<char>(bytecode_file)),
  // std::istreambuf_iterator<char>());
  //  
  // auto module = wasm_runtime_load(reinterpret_cast<const uint8_t*>(bytecode.data()), bytecode.size(), error_buf.data(), error_buf.size());
  //  
  // std::cout << std::string(error_buf.data()) << std::endl;
  //  
  // if (!module) {
  // std::cerr << "Failed to load bytecode!" << std::endl;
  // return -20;
  // }
  //  
  // // We're going to use this first module to define helper functions that
  // // we'll call from the next module.
  // auto registered = wasm_runtime_register_module("go_sample", module, error_buf.data(), error_buf.size());
  // if (!registered) {
  // std::cerr << "Failed to register\"go_sample\" module" << std::endl;
  // return -25;
  // }
  std::ifstream main_bytecode_file("wasm/rust/target/wasm32-wasi/debug/rust.wasm");
  //std::ifstream main_bytecode_file("wasm/rust/pkg/rust_bg.wasm");

  std::string main_bytecode((std::istreambuf_iterator<char>(main_bytecode_file)), std::istreambuf_iterator<char>());
  auto main_module = wasm_runtime_load(reinterpret_cast<const uint8_t*>(main_bytecode.data()), main_bytecode.size(), error_buf.data(), error_buf.size());

  if (!main_module) {
    std::cerr << "Failed to load main bytecode! " << std::string(error_buf.data()) << std::endl;
    return -27;
  }

  auto module_inst = wasm_runtime_instantiate(main_module, stack_size, heap_size, error_buf.data(), error_buf.size());

  if (!module_inst) {
    std::cerr << "Failed to instantiate module!" << std::endl;
    return -30;
  }

  auto exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);

  if (!exec_env) {
    std::cerr << "Failed to instantiate module!" << std::endl;
    return -40;
  }

  auto func = wasm_runtime_lookup_function(module_inst, "hi", nullptr);

  if (!func) {
    std::cerr << "Failed to find \"hi\" function!" << std::endl;
    return -50;
  }

  // Setting up the function argument.  We need to copy data in to the WASM sandbox
  // (on account of it being, you know, a /sandbox/).
  std::string arg("C++");
  auto wasm_buf = wasm_runtime_module_dup_data(module_inst, arg.data(), arg.size());
  std::array<uint32_t, 2> func_args = {wasm_buf, arg.size()};

  auto call_succeeded = wasm_runtime_call_wasm(exec_env, func, func_args.size(), func_args.data());

  if (!call_succeeded) {
    std::cerr << "Failed to call \"hi\"!" << std::endl;
    std::cerr << std::string(error_buf.data()) << std::endl;
    return -60;
  }

  // The return value is in the first element of the function argument array.
  // It's a WASM pointer to a null-terminated string.  Fortunately, WAMR allows
  // us to map WASM pointers to native pointers.
  auto func_rv = wasm_runtime_addr_app_to_native(module_inst, func_args[0]);

  std::cout << "WASM function \"hi\" returned: " << std::string(static_cast<char*>(func_rv)) << std::endl;

  return 0;
}
