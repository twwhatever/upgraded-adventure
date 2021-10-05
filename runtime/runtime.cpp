#include <array>
#include <fstream>
#include <iostream>
#include <wasm_export.h>

int main(int argc, char* argv[]) {
  char *buffer;
  std::array<char, 128> error_buf;
  uint32_t size, stack_size = 8092, heap_size = 8092;

  auto wasm_initialized = wasm_runtime_init();

  if (!wasm_initialized) {
    std::cerr << "Failed to initialize WASM!" << std::endl;
    return -10;
  }

  std::ifstream bytecode_file("wasm/go/sample/go_sample.wasm");

  std::string bytecode(
                       (std::istreambuf_iterator<char>(bytecode_file)),
                       std::istreambuf_iterator<char>());

  auto module = wasm_runtime_load(reinterpret_cast<const uint8_t*>(bytecode.data()), bytecode.size(), error_buf.data(), error_buf.size());

  if (!module) {
    std::cerr << "Failed to load bytecode!" << std::endl;
    return -20;
  }

  auto module_inst = wasm_runtime_instantiate(module, stack_size, heap_size, error_buf.data(), error_buf.size());

  if (!module_inst) {
    std::cerr << "Failed to instantiate module!" << std::endl;
    return -30;
  }

  auto exec_env = wasm_runtime_create_exec_env(module_inst, stack_size);

  if (!exec_env) {
    std::cerr << "Failed to instantiate module!" << std::endl;
    return -40;
  }

  auto func = wasm_runtime_lookup_function(module_inst, "hello", nullptr);

  if (!func) {
    std::cerr << "Failed to find \"hello\" function!" << std::endl;
    return -50;
  }

  // Setting up the function argument.  We need to copy data in to the WASM sandbox
  // (on account of it being, you know, a /sandbox/).
  std::string arg("C++");
  auto wasm_buf = wasm_runtime_module_dup_data(module_inst, arg.data(), arg.size());
  std::array<uint32_t, 2> func_args = {wasm_buf, arg.size()};

  auto call_succeeded = wasm_runtime_call_wasm(exec_env, func, func_args.size(), func_args.data());

  if (!call_succeeded) {
    std::cerr << "Failed to call \"hello\"!" << std::endl;
    return -60;
  }

  // The return value is in the first element of the function argument array.
  // It's a WASM pointer to a null-terminated string.  Fortunately, WAMR allows
  // us to map WASM pointers to native pointers.
  auto func_rv = wasm_runtime_addr_app_to_native(module_inst, func_args[0]);

  std::cout << "WASM function \"hello\" returned: " << std::string(static_cast<char*>(func_rv));

  return 0;
}
